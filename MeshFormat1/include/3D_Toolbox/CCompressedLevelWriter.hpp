#pragma once

#include <irrlicht.h>

namespace Toolbox3D
{
	class CCompressedLevelWriter : public irr::scene::IMeshWriter
	{
	public:
		//! Returns the type of the mesh writer
		virtual irr::scene::EMESH_WRITER_TYPE getType() const;
		
		//! writes a mesh
		virtual bool writeMesh(irr::io::IWriteFile* file, irr::scene::IMesh* mesh, irr::s32 flags=0);
		
		virtual irr::io::path filePath(const irr::io::path &p);
	};
}


