/*
	2003/8/6   coded by KIOKU
	dkTextクラス
	指定したFontや色でテキストが描画できるクラス
	(大半がSets氏のクラスからの流用 thanx -> Sets）
	Ver 0.8
				そのほかいろいろ実装。printfもできたしもぉこれ以上いいかな？
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
		#ifdef __DKIMAGE_h__//dkImageクラスがincludeされているなら
		void dkPrintf(dkImage* dest_image, long x, long y, const char* format, ...);
		#endif//__DKIMAGE_h__
};

dkText::dkText()
{
	// フォントの初期化
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
	
	textColor = RGB(0, 0, 0);		// 文字色
}

dkText::~dkText()
{
}

// フォントを指定する
void dkText::dkSetFont(const char* fontName)			{ strcpy(lfFont.lfFaceName, fontName);	}
// フォントサイズを指定する	
void dkText::dkSetFontSize(int fontSize)		{ lfFont.lfHeight	 = fontSize;		}
// 太字にするかどうか指定する
void dkText::dkSetBold(bool isBold){
	if(isBold==true) lfFont.lfWeight = FW_BOLD;
	else			 lfFont.lfWeight = FW_NORMAL;
}
// イタリックにするかどうか指定する
void dkText::dkSetItalic(bool isItalic)			{ lfFont.lfItalic	 = isItalic;		}
// アンダーラインをつけるかどうかを指定する
void dkText::dkSetUnderline(bool isUnderline)	{ lfFont.lfUnderline = isUnderline;		}
// 打ち消し線をつけるかどうか指定する
void dkText::dkSetStrikeOut(bool isStrikeOut)	{ lfFont.lfStrikeOut = isStrikeOut;		}
// 文字列を傾ける
void dkText::dkSlopeText(float angle){ 
	lfFont.lfEscapement = lfFont.lfOrientation = (int)(angle*10);
}
// 文字色を設定する
void dkText::dkSetTextColor(unsigned char red,unsigned char green,unsigned char blue){	
	textColor = RGB(red,green,blue);
}

// 範囲指定して画像上に文字列を描画
void dkText::dkPrint(HDC hdc, long x, long y,const char* str)
{
	RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + 30000;//適当にデカイ数字（テキストがRECT枠外に出るのをふせぐ
	rect.bottom = y + lfFont.lfHeight;//こっちはとりあえず高さでいいかな・・？って感じ
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

#ifdef __DKIMAGE_h__//dkImageクラスがincludeされているなら
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