#pragma once

#include <irrlicht.h>

namespace Toolbox3D
{
	class CCompressedLevelReader : public irr::scene::IMeshLoader
	{
	public:
		CCompressedLevelReader(irr::video::IVideoDriver* _driver);
		virtual ~CCompressedLevelReader ();
		
		virtual irr::scene::IAnimatedMesh * createMesh (irr::io::IReadFile *file);
		
		virtual bool isALoadableFileExtension (const irr::io::path &filename) const;
		
		irr::video::SMaterial PreMat;
		
		/**
		 * This string is prepended to the Texture name before it is passed to driver->getTexture().
		 */
		irr::io::path TexPrefix;
	private:
		irr::video::IVideoDriver* driver;
		
	};
}


