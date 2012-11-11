//FPU routine library
//coded by c.r.v.^System K 2006

#ifndef KFPU_H_
#define KFPU_H_

__forceinline void kfInit(){
	__asm finit;
}

__forceinline float kfAdd(float fDst, float fSrc){//fDst + fSrc
	volatile float fAns;
	__asm {
		//finit
		fld  fDst
		fadd fSrc
		fstp fAns
	}
	return fAns;
}

__forceinline float kfSub(float fDst, float fSrc){//fDst - fSrc
	volatile float fAns;
	__asm {
		//finit
		fld fSrc
		fsubr fDst
		fstp fAns
	}
	return fAns;
}

__forceinline float kfMul(float fDst, float fSrc){//fDst * fSrc
	volatile float fAns;
	__asm {
		//finit
		fld fDst
		fmul fSrc
		fstp fAns
	}
	return fAns;
}

__forceinline float kfDiv(float fDst, float fSrc){//fDst / fSrc
	volatile float fAns;
	__asm {
		//finit
		fld fDst
		fdiv fSrc
		fstp fAns
	}
	return fAns;
}

__forceinline float kfAbs(float fSrc){//return (fSrc>=0.0f) ? fSrc : -fSrc;
	volatile float fAns;
	_asm{
		//finit
		fld fSrc
		fabs
		fstp fAns
	}
	return fAns;
}

__forceinline float kfSwitchSign(float fSrc){//fSrc*=-1;
	volatile float fAns;
	_asm{
		//finit
		fld fSrc
		fchs
		fstp fAns
	}
	return fAns;
}

__forceinline float kfSin(float fSrc){//sin(fSrc)
	volatile float fAns;
	_asm{
		//finit
		fld fSrc
		fsin
		fstp fAns
	}
	return fAns;
}

__forceinline float kfCos(float fSrc){//cos(fSrc)
	volatile float fAns;
	_asm{
		//finit
		fld fSrc
		fcos 
		fstp fAns
	}
	return fAns;
}

__forceinline float kfTan(float fSrc){//tan(fSrc)
	volatile float fAns;
	_asm{
		//finit
		fld fSrc
		fptan
		fstp  fAns
	}
	return fAns;
}

__forceinline float kfAtan(float fSrc){//atan(fSrc)
	volatile float fAns;
	_asm{
		//finit
		fld fSrc
		fld1
		fpatan
		fstp fAns
		fld fAns
	}
	return fAns;
}

__forceinline float kfSqrt(float fSrc){//sqrt(fSrc)
	volatile float fAns;
	_asm{
		//finit
		fld fSrc
		fsqrt 
		fstp fAns
	}
	return fAns;
}

__forceinline float kfFtol(float fSrc){
	_asm{
		//finit
		fistp fSrc
	}
	return fSrc;
}

#endif