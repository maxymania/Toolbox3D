#pragma once
#include <irrlicht.h>

namespace content_io {
	namespace util {
		irr::u32 f2i(irr::f32 f);
		irr::f32 i2f(irr::u32 i);
		void stu(irr::u8 *p,irr::u32 u);
		irr::u32 ldu(const irr::u8 *p);
		
		void sts(irr::u8 *p,irr::s32 s);
		irr::s32 lds(const irr::u8 *p);
		
		void stf(irr::u8 *p,irr::f32 f);
		irr::f32 ldf(const irr::u8 *p);
	}
}