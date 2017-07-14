#include "dumpmath.h"
#include <math.h>

using namespace irr;

//-----------------------------------------------------------------------------

irr::u32 content_io::util::f2i(irr::f32 f){
	int exp;
	double d = frexp((double)f,&exp);
	s32 r = (s32)(d*0x7fffff)*0x100;
	u8 uexp = (u8)((s8)(exp));
	return ((u32)(r)) | (u32)(uexp);
}
irr::f32 content_io::util::i2f(irr::u32 i){
	s8 exp = (s8)(i&0xff);
	s32 s = (s32)i;
	s/=0x100;
	double d = (double)(s)/(double)(0x7fffff);
	return (f32)ldexp(d,(int)exp);
}

//-----------------------------------------------------------------------------

void content_io::util::stu(irr::u8 *p,irr::u32 u){
	p[0] = (u8)(u>>24);
	p[1] = (u8)(u>>16);
	p[2] = (u8)(u>>8);
	p[3] = (u8)(u);
}
irr::u32 content_io::util::ldu(const irr::u8 *p){
	return
	((u32)(p[0])<<24)|
	((u32)(p[1])<<16)|
	((u32)(p[2])<< 8)|
	(u32)(p[3]);
}

//-----------------------------------------------------------------------------

void content_io::util::sts(irr::u8 *p,irr::s32 s){
	stu(p,(u32)(s));
}
irr::s32 content_io::util::lds(const irr::u8 *p){
	return (s32)(ldu(p));
}

//-----------------------------------------------------------------------------

void content_io::util::stf(irr::u8 *p,irr::f32 f){
	stu(p,f2i(f));
}
irr::f32 content_io::util::ldf(const irr::u8 *p){
	return i2f(ldu(p));
}


