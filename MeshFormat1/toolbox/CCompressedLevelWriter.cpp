#include <3D_Toolbox/CCompressedLevelWriter.hpp>
#include <math.h>

#ifdef METHOD
#undef METHOD
#endif

#define METHOD Toolbox3D::CCompressedLevelWriter::

using namespace irr::scene;
using namespace irr::video;
using namespace irr::io;
using namespace irr::core;
using namespace irr;

static inline void SerInt(u8* ptr,irr::u32 u){
	ptr[0] = (u8)(0xFF&(u>>24));
	ptr[1] = (u8)(0xFF&(u>>16));
	ptr[2] = (u8)(0xFF&(u>>8));
	ptr[3] = (u8)(0xFF&(u));
}
static inline void SerSht(u8* ptr,irr::u16 u){
	ptr[0] = (u8)(0xFF&(u>>8));
	ptr[1] = (u8)(0xFF&(u));
}

static inline void SerFlt(u8* ptr,f32 f){
	double d = (double)f;
	int exp;
	irr::s32 s = (s32)(0x7fffffff*frexp(d,&exp));
	irr::s16 x = (s16)exp;
	SerSht(ptr  ,(u16)x);
	SerInt(ptr+2,(u32)s);
}

#define flto(x) ((x)*6)

static inline void SerV2f(u8* ptr,const vector2d<f32> &df){
	SerFlt(ptr+ flto(0) ,df.X);
	SerFlt(ptr+ flto(1) ,df.Y);
}
static inline void SerV3f(u8* ptr,const vector3df &df){
	SerFlt(ptr+ flto(0) ,df.X);
	SerFlt(ptr+ flto(1) ,df.Y);
	SerFlt(ptr+ flto(2) ,df.Z);
}
/* */
//static inline path filePath(const path &p) { return p; }

irr::scene::EMESH_WRITER_TYPE METHOD getType() const{
	return (irr::scene::EMESH_WRITER_TYPE)0;
}

#define V3F (6*3)
#define V2F (6*2)

#define VTX_Normal   0
#define VTX_Pos      VTX_Normal+V3F
#define VTX_TCoords  VTX_Pos+V3F
#define VTX_Binormal VTX_TCoords+V2F
#define VTX_Tangent  VTX_Binormal+V3F
#define VTX_END      VTX_Tangent+V3F

#define failsz(a,b)  if(a<b) return false
bool METHOD writeMesh(irr::io::IWriteFile* file, irr::scene::IMesh* mesh, irr::s32 flags){

	u32 nBufs = mesh->getMeshBufferCount();
	u32 i,j,nVtx;
	u8 buffer[VTX_END];
	u32 *idx32;
	u16 *idx16;
	
	// Reuse variable slot.
#define nIdx nVtx
#define nTfn nVtx
	
	for(i = 0;i<nBufs;++i){
		IMeshBuffer* imb = mesh->getMeshBuffer(i);
		if(!imb)return false;
		if(imb->getVertexType()!=EVT_TANGENTS) return false;
	}
	
	failsz(file->write("CmpLevel",8),8);
	SerInt(buffer,nBufs);
	failsz(file->write(buffer,4),4);
	for(i = 0;i<nBufs;++i){
		IMeshBuffer* imb = mesh->getMeshBuffer(i);
		
		const video::SMaterial &mat = imb->getMaterial();
		
		for(j = 0;j<2;++j){
			ITexture* tex = mat.getTexture(j);
			stringc strc = "";
			if(tex){
				path p = filePath(tex->getName().getInternalName());
				strc = stringc(p);
			}
			// Write the filename (Length;Content)
			nTfn = strc.size();
			SerInt(buffer,nTfn);
			failsz(file->write(buffer,4),4);
			failsz(file->write(strc.c_str(),nTfn),nTfn);
		}
		
		S3DVertexTangents* vtx = (S3DVertexTangents*)imb->getVertices();
		nVtx = imb->getVertexCount();
		
		// Write nVtx
		SerInt(buffer,nVtx);
		failsz(file->write(buffer,4),4);
		for(j = 0;j<nVtx;++j){
			//Write vtx[j];
			SerV3f(buffer+VTX_Normal   ,vtx[j].Normal);
			SerV3f(buffer+VTX_Pos      ,vtx[j].Pos);
			SerV2f(buffer+VTX_TCoords  ,vtx[j].TCoords);
			SerV3f(buffer+VTX_Binormal ,vtx[j].Binormal);
			SerV3f(buffer+VTX_Tangent  ,vtx[j].Tangent);
			failsz(file->write(buffer,VTX_END),VTX_END);
		}
		
		nIdx = imb->getIndexCount();
		
		// Write nIdx
		SerInt(buffer,nIdx);
		failsz(file->write(buffer,4),4);
		switch(imb->getIndexType()){
		case irr::video::EIT_16BIT :
			idx16 = imb->getIndices();
			for(j = 0;j<nIdx;++j){
				SerInt(buffer,(u32)(idx16[i]));
				failsz(file->write(buffer,4),4);
			}
			break;
		case irr::video::EIT_32BIT :
			;
			idx32 = (u32*)(imb->getIndices());
			for(j = 0;j<nIdx;++j){
				SerInt(buffer,idx32[i]);
				failsz(file->write(buffer,4),4);
			}
		}
	}
	
	return false;
}

irr::io::path METHOD filePath(const irr::io::path &p) {
	return p;
}
