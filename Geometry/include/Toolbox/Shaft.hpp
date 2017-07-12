/*
 * Copyright (c) 2017 Simon Schmidt
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
#pragma once
#include <irrlicht.h>

namespace Toolbox{
namespace Rotation{
	namespace AngleShaft{
		enum Type{ X,Y,Z,};
		inline irr::f32 &getAxle(irr::core::vector3df &v,Type ast){
			switch(ast){
			case X:return v.X;
			case Y:return v.Y;
			case Z:return v.Z;
			}
			return v.X; // Undefined Behavoir? Fallback!
		}
	}
	class Angle {
	public:
		irr::f32 Shaft;
		irr::f32 Transverse;
		irr::f32 Longitudinal;
		irr::f32 Vertical;
		inline Angle():
			Shaft(.0f),Transverse(.0f),Longitudinal(.0f),Vertical(.0f){}
		inline Angle(irr::f32 s,irr::f32 t,irr::f32 l,irr::f32 v):
			Shaft(s),Transverse(t),Longitudinal(l),Vertical(v){}
		inline void toRotation(AngleShaft::Type as,irr::core::vector3df &rotation){
			irr::core::vector3df SRT(.0f,.0f,.0f), euler;
			getAxle(SRT,as) = Shaft*irr::core::DEGTORAD;
			(
				irr::core::quaternion(SRT)*
				irr::core::quaternion(Transverse*irr::core::DEGTORAD,.0f,Longitudinal*irr::core::DEGTORAD)*
				irr::core::quaternion(.0f,Vertical*irr::core::DEGTORAD,.0f)
			).toEuler(euler);
			rotation = euler*irr::core::RADTODEG;
		}
	};
	class AngleWithType : public Angle {
	public:
		AngleShaft::Type ShaftType;
		inline AngleWithType(): Angle(), ShaftType(AngleShaft::Z) {}
		inline AngleWithType(irr::f32 s,irr::f32 t,irr::f32 l,irr::f32 v): Angle(s,t,l,v), ShaftType(AngleShaft::Z) {}
		inline AngleWithType(irr::f32 s,irr::f32 t,irr::f32 l,irr::f32 v, AngleShaft::Type ast): Angle(s,t,l,v), ShaftType(ast) {}
		inline void toRotation(irr::core::vector3df &rotation){
			Angle::toRotation(ShaftType,rotation);
		}
	};
}
}

