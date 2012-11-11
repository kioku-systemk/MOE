#pragma once
#ifndef __KMATH__
#define __KMATH__
/*
		original math library (coded by KIOKU)
		2003-2004
*/
extern "C" {
	int __cdecl _ftol2_sse();
	int __cdecl _ftol2();// for VS.NET 2003
}

#define e                2.718281828f
#define PI               3.141592654f
#define PI2              6.283185308f

//----math triagle functions---------------------------------
float katan(float x);
float my_sin(float wt);
#define my_cos(angle)		my_sin(angle+PI*0.5f)
//------------------------------------------------------------

//---some wave functions--------------------------------------
float square(float wt);
float saw(float wt);
float tri(float wt);
void __fastcall ksrand(unsigned long seed);
int krand(void);
float noise(float wt);
float sqr(float a);
float nullfunc(float wt);
//------------------------------------------------------------

//------------------------------------------------------------
//These function use by enable (inline and __fastcall)
//float To Half-float Funtion
inline short __fastcall ftohf(float fl)
{
	return ((short)((*(int*)(&fl))>>16));
}

//Half-float To float Funtion
inline float __fastcall hftof(short sh)
{
	int sfl = (sh<<16);
	return *((float*)(&sfl));
}
//------------------------------------------------------------

float __fastcall pnoize_base(float x,float y, float range);
void __fastcall gen_pnoize();
float __fastcall sinplasma(float x, float y, float range,float offset_x,float offset_y);
float __fastcall sinenv(float x,float y);
float __fastcall pnoize(long start_r,float end_r, float px,float py);

//------------------------------------------------------------
//These function use by enable (inline and __fastcall)
//Degree To char
inline char __fastcall dtoc(float degree)
{
	return (char)((degree/180.0f)*128);
}

//char To Degree
inline float __fastcall ctod(char ch)
{
	return ((ch/128.0f)*180.0f);
}
//------------------------------------------------------------
//#include <windows.h>
//inline unsigned long kstrlen(const char* string){
//	__asm{//fast strlen
//		mov edi, string; 
//		xor al,  al; 
//		mov ecx, -1; 
//		cld;
//		repne scasb;
//		mov eax, -1; 
//		sub eax,ecx; 
//		dec eax; 
//	} 
//}
//
//#define kstrcpy(dest, src) kmemcpy(dest, src, kstrlen(src));

inline void __fastcall kmemcpy(void* dest, const void* src, unsigned long cpysize)
{
	__asm{//fast memcpy
		mov  ecx, DWORD PTR cpysize;
		mov  esi, src;
		mov  eax, ecx;
		mov  edi, dest;
		shr  ecx, 2;
		rep movsd;
		mov  ecx, eax;
		and  ecx, 3;
		rep movsb;
	}
	//__asm {//boyC's memcpy. slow.
	//	mov edi,dest
	//	mov esi,src
	//	mov ecx,cpysize
	//	rep movsb
	//}

	////original one.
	//unsigned char* cdest = (unsigned char*)dest;
	//unsigned char* csrc = (unsigned char*)src;
	//unsigned char* cenddest=cdest+cpysize;
	//while(cdest!=cenddest) *(cdest++)=*(csrc++);
}

#define kZeroMemory(dest, size) kFillMemory(dest, size, (unsigned char)0);

inline void __fastcall kFillMemory(void* dest, unsigned long cpysize, unsigned char fill)
{
	__asm{
		mov al, BYTE PTR fill;
		xor ebx, ebx;
		mov ecx, DWORD PTR cpysize;
		mov edi, dest;
MEMFILL:
		mov BYTE PTR[edi+ebx], al;
		inc ebx;
		loop MEMFILL;
	}

	//unsigned char* cdest = (unsigned char*)dest;
	//unsigned char* cenddest=cdest+cpysize;
	//while(cdest!=cenddest) *(cdest++)=fill;
}

#define DENORMALIZE(fv) (((*(unsigned int*)&(fv))&0x7f800000)<0x08000000)?0.0f:(fv)
#endif
