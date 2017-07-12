#include <3D_Toolbox/CCompressedLevelReader.hpp>
#include <math.h>

#ifdef METHOD
#undef METHOD
#endif

#define METHOD Toolbox3D::CCompressedLevelReader::

static inline irr::u32 DesInt(irr::u8* ptr){
	irr::u32 u(0);
	u = ptr[0];
	u = (u<<=8)|ptr[1];
	u = (u<<=8)|ptr[2];
	u = (u<<=8)|ptr[3];
	return u;
}
static inline irr::u16 DesSht(irr::u8* ptr){
	irr::u16 u(0);
	u = ptr[0];
	u = (u<<=8)|ptr[1];
	return u;
}
static inline irr::f32 DesFlt(irr::u8* ptr){
	irr::s16 x = DesSht(ptr);
	irr::s32 s = DesInt(ptr+2);
	double f = s;
	f/=0x7fffffff;
	f = ldexp(f,x);
	return (irr::f32)f;
}

#define flto(x) ((x)*6)

static inline irr::core::vector2d<irr::f32> DesV2f(irr::u8* ptr){
	return irr::core::vector2d<irr::f32>(
		DesFlt(ptr+ flto(0)),/*.X*/
		DesFlt(ptr+ flto(1))/*.Y*/
	);
	
}
static inline irr::core::vector3df DesV3f(irr::u8* ptr){
	return irr::core::vector3df(
		DesFlt(ptr+ flto(0)),/*.X*/
		DesFlt(ptr+ flto(1)),/*.Y*/
		DesFlt(ptr+ flto(2))/*.Z*/
	);
}

METHOD CCompressedLevelReader(irr::video::IVideoDriver* _driver): driver(_driver) {}
METHOD ~CCompressedLevelReader (){}
bool METHOD isALoadableFileExtension (const irr::io::path &filename) const{
	return irr::core::hasFileExtension ( filename, "clevel");
}

using namespace irr;
using namespace scene;
using namespace core;

#define V3F (6*3)
#define V2F (6*2)

#define VTX_Normal   0
#define VTX_Pos      VTX_Normal+V3F
#define VTX_TCoords  VTX_Pos+V3F
#define VTX_Binormal VTX_TCoords+V2F
#define VTX_Tangent  VTX_Binormal+V3F
#define VTX_END      VTX_Tangent+V3F

#define faildef(from,to,code) from: code; goto to
#define failon(a,b) if(!a) goto b

static inline bool readx(irr::io::IReadFile *file,void *buffer, u32 sizeToRead){
	s32 readmust = sizeToRead;
	s32 readt = file->read(buffer,sizeToRead);
	if(readt<readmust) return false;
	return true;
}
template<class T>
static inline bool checkEq(const u8* a,const T* b,u32 sz){
	do{
		u8 c = (u8)(*b);
		if((*a)!=c)return false;
		sz--;
		a++;
		b++;
	}while(sz);
	return true;
}
template<class T>
static inline T moveptr(T &p){
	T temp = p;
	p = 0;
	return temp;
}

irr::scene::IAnimatedMesh* METHOD createMesh (irr::io::IReadFile *file){
	u8 buffer[VTX_END];
	u32 i,j,nBufs,nVtx;
	array<u8> strb;
	video::S3DVertexTangents vtx = video::S3DVertexTangents(0.f,0.f,0.f);
	SMesh* mesh = new SMesh;
	
	// Reuse variable slot.
#define nIdx nVtx
#define nTfn nVtx
	
	failon(readx(file,buffer,8+4), failMesh);
	failon(checkEq(buffer,"CmpLevel",8), failMesh);
	nBufs = DesInt(buffer+8);
	
	for(i = 0;i<nBufs;++i){
		video::SMaterial mat = PreMat;
		for(j = 0;j<2;++j){
			// Read the filename (Length;Content)
			failon(readx(file,buffer,4), failMesh);
			nTfn = DesInt(buffer);
			strb.reallocate(nTfn+1);
			failon(readx(file,strb.pointer(),nTfn), failMesh);
			strb.pointer()[nTfn]=0;
			mat.setTexture(j,driver->getTexture(TexPrefix+strb.const_pointer()));
		}
		
		
		// Read nVtx
		failon(readx(file,buffer,4), failMesh);
		nVtx = DesInt(buffer);
		const bool mustBe32 = nVtx>0xFFFF;
		
		CDynamicMeshBuffer* cmb = new CDynamicMeshBuffer(video::EVT_TANGENTS,mustBe32?video::EIT_32BIT:video::EIT_16BIT);
		
		for(j = 0;j<nVtx;++j){
			failon(readx(file,buffer,VTX_END), failCMB);
			
			vtx.Normal   = DesV3f(buffer+VTX_Normal   );
			vtx.Pos      = DesV3f(buffer+VTX_Pos      );
			vtx.TCoords  = DesV2f(buffer+VTX_TCoords  );
			vtx.Binormal = DesV3f(buffer+VTX_Binormal );
			vtx.Tangent  = DesV3f(buffer+VTX_Tangent  );
			cmb->getVertexBuffer().push_back(vtx);
		}
		
		// Read nIdx
		failon(readx(file,buffer,4), failCMB);
		nIdx = DesInt(buffer);
		
		for(j = 0;j<nIdx;++j){
			failon(readx(file,buffer,4), failCMB);
			u32 vtxidx = DesInt(buffer);
			cmb->getIndexBuffer().push_back(vtxidx);
		}
		
		
		// Let's add the newly created Meshbuffer to the mesh object.
#if 1
		// This is an optional optimization, that turns the CDynamicMeshBuffer into an
		// SMeshBufferTangents (aka CMeshBuffer<S3DVertexTangents>).
		// Unfortunately, this only works with 16-bit indeces.
		if(!mustBe32){
			SMeshBufferTangents* smt = new SMeshBufferTangents;
			// append (const void *const vertices, u32 numVertices, const u16 *const indices, u32 numIndices) 
			smt->append(cmb->getVertices(),cmb->getVertexCount(),cmb->getIndices(),cmb->getIndexCount());
			smt->Material = mat;// Set the material.
			smt->recalculateBoundingBox();//Calculate the bounding box
			smt->setHardwareMappingHint(EHM_STATIC,EBT_VERTEX_AND_INDEX);
			moveptr(cmb)->drop();
			mesh->addMeshBuffer(moveptr(smt));
		}else
#endif
		{
			cmb->Material = mat;// Set the material.
			cmb->recalculateBoundingBox();//Calculate the bounding box
			cmb->setHardwareMappingHint(EHM_STATIC,EBT_VERTEX_AND_INDEX);
			mesh->addMeshBuffer(moveptr(cmb));
		}
		
		continue;
		faildef(failCMB,failMesh,cmb->drop());
	}
	
	return new SAnimatedMesh(mesh);
failMesh:
	mesh->drop();
	return 0;
}

