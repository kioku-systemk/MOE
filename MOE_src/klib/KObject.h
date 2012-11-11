/*
	KObject Lib(for 64k intro)
	coded by kioku 2005
	last modified 2005/6/5
	using original format kmo,kpr,kof
*/
#ifndef __KOBJECT__
#define __KOBJECT__

#include "vector.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include "../GL/glext.h"

class KObject
{
	private:
		unsigned char objecttype;

		//KMO
		unsigned long kobject_size;
		char* object_name;
		unsigned short vertex_num;		//4 byte
		unsigned short face_num[2];		//8 byte
 
		unsigned short* face;
		CVector* fvertex;
		CVector* fnormal;

		//KOF
		//unsigned short clone_num;
		//unsigned short* objectsize;
		//KObject* kmo;
		//KCloneData* kcln;

		CVector __fastcall Spline2D(CVector vec0,CVector vec1,CVector vec2, float rate);

		const unsigned char*__fastcall LoadKMB(const unsigned char* dptr);//KMB format
		const unsigned char* __fastcall LoadKOF(const unsigned char* dptr);//KOF format
		const unsigned char* __fastcall LoadKPR(const unsigned char* dptr);//KPR format
		void __fastcall FreeKMB();
		void __fastcall FreeKOF();
		void __fastcall FreeKPR();

		void __fastcall DrawSDTriangle(CVector vt[3],CVector n[3], long divide, long mapping, unsigned int mt_texcnt);
		void __fastcall DrawSDSquare(CVector vt[4],CVector n[4], long divide, long mapping, unsigned int mt_texcnt);
		void __fastcall DrawKMB(int wireframe,int ninv, int subdiv, long mapping, unsigned int mt_texcnt);
		void __fastcall DrawKOF(int wireframe,int ninv);
		void __fastcall DrawKPR(int wireframe,int ninv);
		
	public:
		char* __fastcall GetName(){ return object_name; }
		const unsigned char* __fastcall Load(const unsigned char* dptr);//Load KMB,KOF,KPR Primitive
		void __fastcall Free();
		void __fastcall Draw(int wireframe=0,int ninv=0, int subdiv=0, long mapping=0, unsigned int mt_texcnt=0);//Draw
		unsigned long __fastcall GetSize();
};
#endif