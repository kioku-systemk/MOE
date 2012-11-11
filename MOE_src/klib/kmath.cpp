#include "stdafx.h"
#include "kmath.h"
#include "kfpu.h"
/*
		original math library (coded by KIOKU)
		2003-2004
*/
/*
sin(x) ¨ x - (1/6) * x^3 + (1/120) * x^5 - (1/5040) * x^7 + (1/362880) * x^9
cos(x) ¨ 1 - (1/2) * x^2 + (1/24) * x^4 - (1/720) * x^6 + (1/40320) * x^8
tan(x) ¨ x + (1/3) * x^3 + (2/15) * x^5 - (17/315) * x^7 + (62/2835) * x^9
asin(x) ¨ x + (1/6) * x^3 + (3/40) * x^5 + (5/112) * x^7 + (35/1152) * x^9
acos(x) ¨ (1/2) * PI - x - (1/6) * x^3 - (3/40) * x^5 - (5/112) * x^7 - (35/1152) * x^9
atan(x) ¨ x - (1/3) * x^3 + (1/5) * x^5 - (1/7) * x^7 + (1/9) * x^9
sinh(x) ¨ x + (1/6) * x^3 + (1/120) * x^5 + (1/5040) * x^7 + (1/362880) * x^9
cosh(x) ¨ 1 + (1/2) * x^2 + (1/24) * x^4 + (1/720) * x^6 + (1/40320) * x^8
tanh(x) ¨ x - (1/3) * x^3 + (2/15) * x^5 - (17/315) * x^7 + (62/2835) * x^9
*/
#ifdef _MSC_VER
extern "C" {
	int __cdecl _ftol2_sse() {
		int integral;
		short oldfcw, newfcw;
		__asm {
			fstcw [oldfcw]
			mov ax,[oldfcw]
			or ax,0c00h ; chop
			mov [newfcw],ax
			fldcw [newfcw]
			fistp dword ptr[integral]
			fldcw [oldfcw]
		}
		return integral;
	}
	
	int __cdecl _ftol2() {return _ftol2_sse();}// for VS.NET 2003
}
#endif

float katan(float x)
{
	//return kfAtan(x);
	return x - (1/3.0f) * x*x*x + (1/5.0f) * x*x*x*x*x - (1/7.0f) * x*x*x*x*x*x*x + (1/9.0f) * x*x*x*x*x*x*x*x*x;
}
float my_dsin(float x)
{
	return kfSin(x);
	//return x - (1.0f/(6.0f)) * x*x*x
	//		 + (1.0f/(6.0f*20.0f)) * x*x*x*x*x
	//		 - (1.0f/(6.0f*20.0f*43.0f)) * x*x*x*x*x*x*x
	//		 + (1.0f/(6.0f*20.0f*43.0f*72.0f)) * x*x*x*x*x*x*x*x*x
	//		 - (1.0f/(6.0f*20.0f*43.0f*72.0f*121.0f)) * x*x*x*x*x*x*x*x*x*x*x;
}


float my_dcos(float x)
{
	return kfCos(x);
	//return 1.0f - (0.5f) * x*x
	//		 + (1.0f/(24.0f)) * x*x*x*x
	//		 - (1.0f/(24.0f*30.0f)) * x*x*x*x*x*x
	//		 + (1.0f/(24.0f*30.0f*56.0f)) * x*x*x*x*x*x*x*x
	//		 - (1.0f/(24.0f*30.0f*56.0f*90.0f)) * x*x*x*x*x*x*x*x*x*x
	//		 + (1.0f/(24.0f*30.0f*56.0f*90.0f*132.0f)) * x*x*x*x*x*x*x*x*x*x*x*x;
}

float my_sin(float wt)
{
	if(wt>=0.0f){
		if(wt<=PI*0.5f) return my_dsin(wt);
		else if(wt<=PI) return my_dcos(wt-PI*0.5f);
		else if(wt<=1.5f*PI) return -my_dsin(wt-PI);
		else if(wt<=2.0f*PI) return -my_dcos(wt-PI*1.5f);
		else {
            long iwt = (long)(wt/PI*0.5f);
            return my_sin(wt-2.0f*PI*iwt);
        }
	}else{
		return -my_sin(-wt);
	}
}

//#define my_cos(angle)		my_sin(angle+PI*0.5f)

float square(float wt)
{
    if(wt<0){				return square(-wt);
    }else if(wt<=PI){		return 1.0f;
    }else if(wt<=2*PI){		return -1.0f;
    }else{
        long iwt = (long)(wt/PI*0.5f);
        return square(wt-2.0f*PI*iwt);
    }
}

float saw(float wt)
{
    if(wt<0.0f){
        return saw(-wt);
    }else if(wt<=2.0f*PI){
        return 1.0f/PI * wt - 1.0f;
    }else{
        long iwt = long(wt/PI*0.5f);
        return saw(wt-2.0f*PI*iwt);
    }
}

float tri(float wt)
{
    float t;
    if(wt<0.0f){ t=tri(-wt);
    }else if(wt<=PI/2.0f){
        t = 2.0f/PI * wt;
    }else if(wt<=3.0f*PI*0.5f){
        t = -2.0f/PI * wt + 2.0f;
    }else if(wt<=2.0f*PI){
        t = 2.0f/PI * wt - 4.0f;
    }else{
        long iwt = (long)(wt/PI*0.5f);
        t=tri(wt-2.0f*PI*iwt);
    }
    return t;
}

static long rand_seed=1;
void __fastcall ksrand(unsigned long seed)
{
	rand_seed = seed;
}

int krand(void)
{
	rand_seed=214013*rand_seed+2531011;
	return (rand_seed>>16)&0x7FFF;
}

float noise(float wt)
{
    return ((krand()%32767)*2-32767)/32767.0f;
}

float sqr(float a)
{
	return kfSqrt(a);
	//if(a<=0) return 0;

	//double Epsilon = a*0.000001f;
	//double x = 0;
	//double z = a;
	//while (!((z-x > -Epsilon)&&(z-x < Epsilon))) {
	//	x = z;
	//	z = x-(x*x-a)/(2*x);
	//}
	//return z;
	///*double x = a;
	//double s,last;
	//if(x<=0.0f) return 0.0;

	//if (x > 1) s = x;  else s = 1.0;
	//do {
	//	last = s;
	//	s = (x / s + s) * 0.5;
	//} while (s < last);

	//return last;*/
}

float nullfunc(float wt)
{
    return 0;
}
#define NOIZE_RANGE		256
float nz[NOIZE_RANGE][NOIZE_RANGE];
float __fastcall pnoize_base(float x,float y, float range)
{
	//int dr = (long)(range/2);
	//x*=range;
	//y*=range;
	//int xd = (long)x;
	//int yd = (long)y;
	//float xf = x - xd;
	//float yf = y - yd;
	//xd += dr;
	//yd += dr;
	//float mx1 = nz[yd][xd]*(1-xf) + nz[yd][xd+1]*xf;
	//float mx2 = nz[yd+1][xd]*(1-xf) + nz[yd+1][xd+1]*xf;
	//float mx = mx1*(1-yf) + mx2*yf;
	//return mx;
	int dr = (long)kfDiv(range,2.0f);
	x = kfMul(x, range);
	y = kfMul(y, range);
	int xd = (long)x;
	int yd = (long)y;
	float xf = x - xd;
	float yf = y - yd;
	xd += dr;
	yd += dr;
	float mx1 = kfMul(nz[yd][xd],  (1.0f-xf)) + kfMul(nz[yd][xd+1]  ,xf);
	float mx2 = kfMul(nz[yd+1][xd],(1.0f-xf)) + kfMul(nz[yd+1][xd+1],xf);
	float mx =  kfMul(mx1,(1.0f-yf)) + kfMul(mx2,yf);
	return mx;
}

void __fastcall gen_pnoize()
{
	int x,y;
	for(y=0;y<NOIZE_RANGE;y++){
		for(x=0;x<NOIZE_RANGE;x++){
			nz[y][x] = (krand()%1000)*0.001f;
		}
	}
}

float __fastcall sinplasma(float x, float y, float range,float offset_x,float offset_y)
{
	//x+=offset_x;
	//y+=offset_y;
	//return 0.5f + 0.25f*my_sin(2.0f*PI*x*range) + 0.25f*my_sin(2.0f*PI*y*range);
	x+=offset_x;
	y+=offset_y;
	return 0.5f + kfMul(0.25f, my_sin(kfMul(kfMul(kfMul(2.0f,PI),x),range))) +
		          kfMul(0.25f, my_sin(kfMul(kfMul(kfMul(2.0f,PI),y),range)));
}

float __fastcall sinenv(float x,float y)
{
	//x-=0.5f;
	//y-=0.5f;
	//float r = (1.0f-sqr(x*x+y*y)*2.0f);
	//if(r<0) r=0;
	//return r;
	x-=0.5f;
	y-=0.5f;
	float r = (1.0f-kfMul(sqr(kfMul(x,x)+kfMul(y,y)),2.0f));
	if(r<0) r=0;
	return r;
}

float __fastcall pnoize(long start_r,float end_r, float px,float py)
{
	//float clf=0.0f;
	//long r;
	//for(r=start_r;r<=end_r;r*=2){
	//	clf += 2.0f*pnoize_base(px,py,(float)r)/(float)r;
	//}
	//if(clf>1.0f) clf=1.0f;
	//return clf;
	float clf=0.0f;
	long r;
	for(r=start_r;r<=end_r;r*=2){
		clf += kfDiv(kfMul(2.0f,pnoize_base(px,py,(float)r)),(float)r);
	}
	if(clf>1.0f) clf=1.0f;
	return clf;
}


//void kmemcpy(void* dest, const void* src, unsigned long cpysize)
//{
//	__asm{//fast memcpy
//		mov  ecx, DWORD PTR cpysize;
//		mov  esi, src;
//		mov  eax, ecx;
//		mov  edi, dest;
//		shr  ecx, 2;
//		rep movsd;
//		mov  ecx, eax;
//		and  ecx, 3;
//		rep movsb;
//	}
//	//__asm {//boyC's memcpy. slow.
//	//	mov edi,dest
//	//	mov esi,src
//	//	mov ecx,cpysize
//	//	rep movsb
//	//}
//
//	////original one.
//	//unsigned char* cdest = (unsigned char*)dest;
//	//unsigned char* csrc = (unsigned char*)src;
//	//unsigned char* cenddest=cdest+cpysize;
//	//while(cdest!=cenddest) *(cdest++)=*(csrc++);
//}
//
//
//void kFillMemory(void* dest, unsigned long cpysize, unsigned char fill)
//{
//	//if( cpysize==0 ) return;
//
//	__asm{
//		//jz	ERROR_EXIT;
//		mov al, BYTE PTR fill;
//		//mov al, BYTE PTR fill;		//unsigned char copy_val = fill;
//		xor ebx, ebx;				//int i=0;
//		mov ecx, DWORD PTR cpysize;	//for(;i<cpysize/*ƒzƒ“ƒg‚Íˆá‚¤‚¯‚Ç‚Ë*/;)
//		mov edi, dest;
//MEMFILL:							//{
//		mov BYTE PTR[edi+ebx], al;		//	dest[i] = copy_val;
//		inc ebx;					//	i++;
//		loop MEMFILL;				//}
////ERROR_EXIT:
////		ret;
//	}
//	//unsigned char* cdest = (unsigned char*)dest;
//	//unsigned char* cenddest=cdest+cpysize;
//	//while(cdest!=cenddest) *(cdest++)=fill;
//}