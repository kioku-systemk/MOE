#include "stdafx.h"
#include "glImage.h"

unsigned char* __fastcall glImage::GetImagePtr()
{
	return image;
}



void __fastcall glImage::Fill (unsigned char red, unsigned char green, unsigned char blue)
{
	//画像をすべて透明色で埋める
	short w,h;
	for(h=0;h<bih.biHeight;h++){
		for(w=0;w<bih.biWidth*3;w+=3){
			image[h*fixedwidth_byte + w    ] = blue ;//B
			image[h*fixedwidth_byte + w + 1] = green;//G
			image[h*fixedwidth_byte + w + 2] = red  ;//R
		}
	}
}

HDC __fastcall glImage::GetDC()
{
	return bitmapDC;
} 
void __fastcall glImage::Create(long ImageWidth, long ImageHeight,unsigned char red, unsigned char green, unsigned char blue)
{		
	fixedwidth_byte = ((ImageWidth*3 + 3)>>2)<<2;//横幅を4倍数に修正 24ビットカラー専用
	
	//BITMAPINFOヘッダの作成(24bitカラー)
	bih.biBitCount = 24;
	bih.biClrImportant = 0;
	bih.biClrUsed = 0;
	bih.biCompression = 0;
	bih.biHeight = ImageHeight;
	bih.biPlanes = 1;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biSizeImage = (((ImageWidth*3 + 3)>>2)<<2) * ImageHeight;
	bih.biWidth = ImageWidth;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	
	//DIBの作成
	HDC gdc = ::GetDC(NULL);
	HBITMAP hBitmap = CreateDIBSection(gdc,(BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&image, NULL, 0);
	//デバイスコンテキストの作成
	bitmapDC = CreateCompatibleDC(gdc);
	HBITMAP oldBMP = (HBITMAP)SelectObject(bitmapDC, hBitmap);
	ReleaseDC(NULL,gdc);
	//画像をすべて透明色で埋める
	Fill(red, green, blue);
		
}

void __fastcall glImage::Convert()
{
	unsigned char* data = GetImagePtr();
	short wid = (short)bih.biWidth;
	short hei = (short)bih.biHeight;
	unsigned char* setdata = (unsigned char*)GlobalAlloc(GPTR,wid*hei*4);
	unsigned short w,h;
	for(h=0; h<hei; h++){
		for(w=0; w<wid; w++){
			unsigned char r,g,b;
			r = data[(w+h*wid)*3+2];
			g = data[(w+h*wid)*3+1];
			b = data[(w+h*wid)*3+0];
			setdata[(w+h*wid)*4+0] = r;
			setdata[(w+h*wid)*4+1] = g;
			setdata[(w+h*wid)*4+2] = b;
			if((r==0)&&(g==0)&&(b==0)) setdata[(w+h*wid)*4+3] = 0;
			else setdata[(w+h*wid)*4+3] = 255;
		}
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1,&gltexturenum);//テクスチャ番号の生成
	
	glBindTexture(GL_TEXTURE_2D, gltexturenum);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, wid, hei, 0, GL_RGBA, GL_UNSIGNED_BYTE, setdata);
	GlobalFree(setdata);
}

void __fastcall glImage::Invert()
{
	unsigned char* data = GetImagePtr();
	short wid = (short)bih.biHeight;
	short hei = (short)bih.biWidth;
	
	short w,h;
	for(h=0; h<hei; h++){
		for(w=0; w<wid; w++){
			data[(w+h*wid)*4+0]=255-data[(w+h*wid)*4+0];
			data[(w+h*wid)*4+1]=255-data[(w+h*wid)*4+1];
			data[(w+h*wid)*4+2]=255-data[(w+h*wid)*4+2];
		}
	}
	glBindTexture(GL_TEXTURE_2D, gltexturenum);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, wid, hei,0,GL_RGBA,GL_UNSIGNED_BYTE, data);
}

unsigned short __fastcall glImage::GetTextureNum()
{
	return gltexturenum;
}