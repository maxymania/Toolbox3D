#pragma once
#include <irrlicht.h>
#include "util.h"

namespace content_io {
	class Dumper {
	private:
		irr::io::IWriteFile *dest;
		irr::core::stringc textBuffer;
	public:
		Dumper(irr::io::IWriteFile *_dest) : dest(_dest){
			rc::retain(dest);
		}
		~Dumper() {
			rc::destroy(dest);
		}
		
		bool write(const void* buffer,irr::u32 sizeToWrite);
		bool writeStaticMesh(irr::scene::IMesh* mesh);
		bool writeStaticMeshBuffer(irr::scene::IMeshBuffer* meshBuffer);
		bool writeMaterial(const irr::video::SMaterial &mat);
	};
}

