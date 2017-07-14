#include "iodump.h"
#include "dumpmath.h"

using namespace irr;
using namespace content_io::util;

#define failon(x) if(!(x))return false
#define success(...) return true

static inline u32 cross_vt(video::E_VERTEX_TYPE vt){
	switch(vt){
	case video::EVT_STANDARD: return 1;
	case video::EVT_2TCOORDS: return 2;
	case video::EVT_TANGENTS: return 3;
	}
	return 0;
}

#define LFIELD 4
/*
Vertex {
	Field   Len  Offset
	-----   ---  ------
	Color   [1]  0
	Normal  [3]  1
	Pos     [3]  4
	TCoords [2]  7
} = 9;
Vertex2TCoords: Vertex {
	Field    Len  Offset
	-----    ---  ------
	T2Coords [2]  9
} = 11;
VertexTangents: Vertex {
	Field    Len  Offset
	-----    ---  ------
	Binormal [3]  9
	Tangent  [3]  12
} = 15;
*/
enum{
	Vertex_Color   = 0,
	Vertex_Normal  = 1*LFIELD,
	Vertex_Pos     = 4*LFIELD,
	Vertex_TCoords = 7*LFIELD,
	Vertex__Len    = 9*LFIELD,
	
	Vertex2TCoords_T2Coords =  9*LFIELD,
	Vertex2TCoords__Len     = 11*LFIELD,
	
	VertexTangents_Binormal =  9*LFIELD,
	VertexTangents_Tangent  = 12*LFIELD,
	VertexTangents__Len     = 15*LFIELD,
	
	VertexLongest__Len = 15*LFIELD,
};


static inline void storeVertex(u8 * buf, irr::video::S3DVertex &vtx){
	stu(buf+Vertex_Color    ,vtx.Color.color);
	//-----------------------------------------
	stf(buf+Vertex_Normal +0,vtx.Normal .X);
	stf(buf+Vertex_Normal +1,vtx.Normal .Y);
	stf(buf+Vertex_Normal +2,vtx.Normal .Z);
	//-----------------------------------------
	stf(buf+Vertex_Pos    +0,vtx.Pos    .X);
	stf(buf+Vertex_Pos    +1,vtx.Pos    .Y);
	stf(buf+Vertex_Pos    +2,vtx.Pos    .Z);
	//-----------------------------------------
	stf(buf+Vertex_TCoords+0,vtx.TCoords.X);
	stf(buf+Vertex_TCoords+1,vtx.TCoords.Y);
}
static inline void storeVertex2TCoords(u8 * buf, irr::video::S3DVertex2TCoords &vtx){
	storeVertex(buf,(irr::video::S3DVertex &)vtx);
	//--------------------------------------------------
	stf(buf+Vertex2TCoords_T2Coords+0,vtx.TCoords2.X);
	stf(buf+Vertex2TCoords_T2Coords+1,vtx.TCoords2.Y);
}
static inline void storeVertexTangents(u8 * buf, irr::video::S3DVertexTangents &vtx){
	storeVertex(buf,(irr::video::S3DVertex &)vtx);
	//--------------------------------------------------
	stf(buf+VertexTangents_Binormal+0,vtx.Binormal.X);
	stf(buf+VertexTangents_Binormal+1,vtx.Binormal.Y);
	stf(buf+VertexTangents_Binormal+2,vtx.Binormal.Z);
	//--------------------------------------------------
	stf(buf+VertexTangents_Tangent +0,vtx.Tangent .X);
	stf(buf+VertexTangents_Tangent +1,vtx.Tangent .Y);
	stf(buf+VertexTangents_Tangent +2,vtx.Tangent .Z);
}

bool content_io::Dumper::write(const void* buffer,irr::u32 sizeToWrite){
	if(!dest)return false;
	s32 r = dest->write(buffer,sizeToWrite);
	if(r<0)return false;
	if((u32)(r)<sizeToWrite)return false;
	return true;
}

bool content_io::Dumper::writeStaticMesh(irr::scene::IMesh* mesh){
	u8 buffer[4];
	u32 i,n = mesh->getMeshBufferCount();
	stu(buffer,n);
	failon(write(buffer,4));
	for(i=0;i<n;++i){
		irr::scene::IMeshBuffer *imb = mesh->getMeshBuffer(i);
		failon(writeStaticMeshBuffer(imb));
	}
	success();
}

bool content_io::Dumper::writeStaticMeshBuffer(irr::scene::IMeshBuffer* meshBuffer){
	u8 buffer[VertexLongest__Len];
	u32 v_type,v_count,i_count,i;
	v_type = cross_vt(meshBuffer->getVertexType());
	v_count = meshBuffer->getVertexCount();
	i_count = meshBuffer->getIndexCount();
	
	failon(v_type); // unknown type.
	
	stu(buffer  ,v_type);
	stu(buffer+4,v_count);
	stu(buffer+8,i_count);
	failon(write(buffer,12));
	
	// TODO: Material.
	failon(writeMaterial(meshBuffer->getMaterial()));
	
	switch(v_type){
	case 1: //Vertex
		{
		video::S3DVertex* s3dv = reinterpret_cast<video::S3DVertex*>(meshBuffer->getVertices());
		for(i=0;i<v_count;++i){
			storeVertex(buffer,s3dv[i]);
			failon(write(buffer,Vertex__Len));
		}
		}break;
	case 2: //Vertex2TCoords
		{
		video::S3DVertex2TCoords* s3dv = reinterpret_cast<video::S3DVertex2TCoords*>(meshBuffer->getVertices());
		for(i=0;i<v_count;++i){
			storeVertex2TCoords(buffer,s3dv[i]);
			failon(write(buffer,Vertex2TCoords__Len));
		}
		}break;
	case 3: //VertexTangent
		{
		video::S3DVertexTangents* s3dv = reinterpret_cast<video::S3DVertexTangents*>(meshBuffer->getVertices());
		for(i=0;i<v_count;++i){
			storeVertexTangents(buffer,s3dv[i]);
			failon(write(buffer,VertexTangents__Len));
		}
		}break;
	}
	switch(meshBuffer->getIndexType()){
	case video::EIT_16BIT:{
		u16* idx = meshBuffer->getIndices();
		for(i=0;i<v_count;++i){
			stu(buffer,(u32)(idx[i]));
			failon(write(buffer,4));
		}
	}break;
	case video::EIT_32BIT:{
		u32* idx = reinterpret_cast<u32*>(meshBuffer->getIndices());
		for(i=0;i<v_count;++i){
			stu(buffer,idx[i]);
			failon(write(buffer,4));
		}
	}break;
	}
	
	success();
}

// Unstable yet.
bool content_io::Dumper::writeMaterial(const irr::video::SMaterial &mat){
	u8 buffer[4];
	u32 i;
	buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0;
	
	// OK, this is definitely a Data-Reduction.
	// We only want to store informations, that might be VERY interesting for us later.
	switch(mat.MaterialType){ // Topological Informations.
	case video::EMT_SOLID_2_LAYER:
		// Use dual layer.
		buffer[0]='2';
		break;
	case video::EMT_LIGHTMAP:
	case video::EMT_LIGHTMAP_ADD:
	case video::EMT_LIGHTMAP_M2:
	case video::EMT_LIGHTMAP_M4:
	case video::EMT_LIGHTMAP_LIGHTING:
	case video::EMT_LIGHTMAP_LIGHTING_M2:
	case video::EMT_LIGHTMAP_LIGHTING_M4:
		// Use Lightmap
		buffer[0]='L';
		break;
	case video::EMT_DETAIL_MAP:
		// Use Detail-Mapped
		buffer[0]='D';
		break;
	case video::EMT_TRANSPARENT_ADD_COLOR:
	case video::EMT_TRANSPARENT_ALPHA_CHANNEL:
	case video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF:
	case video::EMT_TRANSPARENT_REFLECTION_2_LAYER:
	case video::EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR:
	case video::EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR:
		// Use Transparency based on the Texture.
		buffer[0]='T';
		break;
	case video::EMT_TRANSPARENT_VERTEX_ALPHA:
	case video::EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA:
	case video::EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA:
		// Use Transparency based on Vertex-Alpha.
		buffer[0]='A';
		break;
	}
	switch(mat.MaterialType){ // Per pixel lightning Options.
	case video::EMT_NORMAL_MAP_SOLID:
	case video::EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR:
	case video::EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA:
		// Per Pixel lightning : Normal Map;
		buffer[1]='N';
		break;
	case video::EMT_PARALLAX_MAP_SOLID:
	case video::EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR:
	case video::EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA:
		// Per Pixel lightning : Parallax Map;
		buffer[1]='P';
		break;
	}
	// Two bytes are used, two bytes for later use.
	failon(write(buffer,4));
	
	for(i=0;i<2;++i){
		video::ITexture* tex = mat.getTexture(i);
		if(tex){
			textBuffer = core::stringc(tex->getName().getInternalName());
		}else{
			textBuffer = core::stringc("");
		}
		stu(buffer,(u32)textBuffer.size());
		failon(write(buffer,4));
		failon(write(textBuffer.c_str(),textBuffer.size()));
	}
	success();
}

