/*
	2003/8/6   coded by KIOKU
	dkText�N���X
	�w�肵��Font��F�Ńe�L�X�g���`��ł���N���X
	(�唼��Sets���̃N���X����̗��p thanx -> Sets�j
	Ver 0.8
				���̂ق����낢������Bprintf���ł�������������ȏア�����ȁH
*/
#ifndef __DKTEXT_h__
#define __DKTEXT_h__

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

class dkText{
	private:
		LOGFONT lfFont;
		COLORREF textColor;
	public:
		dkText();
		~dkText();
		void dkSetFont(const char* fontName);
		void dkSetFontSize(int fontSize);
		void dkSetBold(bool isBold);
		void dkSetItalic(bool isItatic);
		void dkSetUnderline(bool isUnderline);
		void dkSetStrikeOut(bool isStrikeOut);
		void dkSlopeText(float angle);
		void dkSetTextColor(unsigned char red,unsigned char green,unsigned char blue);
		void dkPrint(HDC hdc, long x, long y,const char* str);
		void dkPrintf(HDC hdc, long x, long y,const char* format, ...);
		#ifdef __DKIMAGE_h__//dkImage�N���X��include����Ă���Ȃ�
		void dkPrintf(dkImage* dest_image, long x, long y, const char* format, ...);
		#endif//__DKIMAGE_h__
};

dkText::dkText()
{
	// �t�H���g�̏�����
	lfFont.lfHeight			= 17;
	lfFont.lfWidth			= 0;
	lfFont.lfEscapement		= 0;
	lfFont.lfOrientation	= 0;
	lfFont.lfWeight			= FW_NORMAL;
	lfFont.lfItalic			= FALSE;
	lfFont.lfUnderline		= FALSE;
	lfFont.lfStrikeOut		= FALSE;
	lfFont.lfCharSet		= SHIFTJIS_CHARSET;
	lfFont.lfOutPrecision	= OUT_DEFAULT_PRECIS;
	lfFont.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
	lfFont.lfQuality		= DEFAULT_QUALITY;
	lfFont.lfPitchAndFamily = FIXED_PITCH;   
	lfFont.lfFaceName[0]	= '\0';
	
	textColor = RGB(0, 0, 0);		// �����F
}

dkText::~dkText()
{
}

// �t�H���g���w�肷��
void dkText::dkSetFont(const char* fontName)			{ strcpy(lfFont.lfFaceName, fontName);	}
// �t�H���g�T�C�Y���w�肷��	
void dkText::dkSetFontSize(int fontSize)		{ lfFont.lfHeight	 = fontSize;		}
// �����ɂ��邩�ǂ����w�肷��
void dkText::dkSetBold(bool isBold){
	if(isBold==true) lfFont.lfWeight = FW_BOLD;
	else			 lfFont.lfWeight = FW_NORMAL;
}
// �C�^���b�N�ɂ��邩�ǂ����w�肷��
void dkText::dkSetItalic(bool isItalic)			{ lfFont.lfItalic	 = isItalic;		}
// �A���_�[���C�������邩�ǂ������w�肷��
void dkText::dkSetUnderline(bool isUnderline)	{ lfFont.lfUnderline = isUnderline;		}
// �ł������������邩�ǂ����w�肷��
void dkText::dkSetStrikeOut(bool isStrikeOut)	{ lfFont.lfStrikeOut = isStrikeOut;		}
// ��������X����
void dkText::dkSlopeText(float angle){ 
	lfFont.lfEscapement = lfFont.lfOrientation = (int)(angle*10);
}
// �����F��ݒ肷��
void dkText::dkSetTextColor(unsigned char red,unsigned char green,unsigned char blue){	
	textColor = RGB(red,green,blue);
}

// �͈͎w�肵�ĉ摜��ɕ������`��
void dkText::dkPrint(HDC hdc, long x, long y,const char* str)
{
	RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + 30000;//�K���Ƀf�J�C�����i�e�L�X�g��RECT�g�O�ɏo��̂��ӂ���
	rect.bottom = y + lfFont.lfHeight;//�������͂Ƃ肠���������ł������ȁE�E�H���Ċ���
	::SetBkMode(hdc, TRANSPARENT);
	HFONT hFont = ::CreateFontIndirect(&lfFont);
	HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
	COLORREF oldColor = SetTextColor(hdc,textColor);
		::DrawText(hdc, str, (int)strlen(str), &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
	SetTextColor(hdc,oldColor);
	SelectObject(hdc,oldFont);
	DeleteObject(hFont);
	return ;
}

void dkText::dkPrintf(HDC hdc, long x, long y,const char* format, ...)
{
	char str[1024];
	va_list ap;
	va_start (ap,format);
	vsprintf(str,format,ap);
	va_end(ap);
	dkPrint(hdc,x,y,str);
}

#ifdef __DKIMAGE_h__//dkImage�N���X��include����Ă���Ȃ�
void dkText::dkPrintf(dkImage* dest_image, long x, long y,const char* format, ...)
{
	char str[1024];
	va_list ap;
	va_start (ap,format);
	vsprintf(str,format,ap);
	va_end(ap);
	dkPrint(dest_image->dkGetDC(),x,y,str);
}
#endif//__DKIMAGE_h__

#endif