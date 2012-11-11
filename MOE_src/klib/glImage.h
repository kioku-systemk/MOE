#pragma once
#include <windows.h>
#include <GL/gl.h>

class glImage{
	private:
		BITMAPINFOHEADER bih;
		unsigned int gltexturenum;
		HDC bitmapDC;
		unsigned char* image;
		long fixedwidth_byte;
	public:
		HDC __fastcall GetDC();
		void __fastcall Create(long ImageWidth, long ImageHeight,unsigned char red, unsigned char green, unsigned char blue);
		unsigned char* __fastcall GetImagePtr();
		void __fastcall Fill (unsigned char red, unsigned char green, unsigned char blue);
		void __fastcall Convert();
		unsigned short __fastcall GetTextureNum();
		void __fastcall Invert();
};

