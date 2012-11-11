/*
	KTexture
	coded by c.r.v.

	singletogn class

	last modified 05/Sep/17
*/
#pragma once
#ifndef __KTEXTURE_H__
#define __KTEXTURE_H__

//#define NOT64K

#include <windows.h>
#include <GL/gl.h>
#include "../GL/glext.h"

#include "kmath.h"
#include "vector.h"

#define TOKEN		';'

#define HISTORY_BUFFER_TEXT_LENGTH 1024*64
#define IMAGE_STACK 16						//Push,Pop用スタック数
#define DATA_STRING_STACK 16				//Stringスタック数
#define DATA_STRING_STACK_LENGTH (1024*64)  //Stringスタックバッファ
#define DATA_INTEGER_STACK 1024				//Integerスタック数
#define PARSER_BUFFER_LENGTH (1024*64)		//文字列解析用バッファサイズ
#define parser_num	38						//ワード数
#define DEFAULT_TEX_SIZE 256.0f	
//#define MULTI_TEXTURE_MAX 8					

//Channel
#define KT_B		0x0001
#define KT_G		0x0002
#define KT_R		0x0004
#define KT_A		0x0008


//Constants && Macro-Functions
#define bppDIB			24
#define posDIB(x,y)		((x)*(bppDIB/8) + ((height-1-y)*width*(bppDIB/8)))
#define posDIBInternal(x,y) posDIB(x,y)
#define sizeDIBBuffer	(sizeof(GLubyte) * width * height * (bppDIB/8)) //CreateDIBSection()で確保されたサイズはGlobalSize()では取得不可能
#define bppGL			32
#define posGL(x,y)		((x)*(bppGL/8) + (y)*(width)*(bppGL/8))
#define posGLInternal(x,y)		posGL(x,y)
#define sizeGLBuffer	(sizeof(float) * width * height * (bppGL/8))
////Image buffer
//#define KT_IMAGE_WIDTH	2048
//#define KT_IMAGE_HEIGHT 2048
//#define sizeIMAGEBuffer (sizeof(float) * KT_IMAGE_WIDTH * KT_IMAGE_HEIGHT * (32/8))
//#define sizeTRANSFERBuffer (sizeof(unsigned char) * KT_IMAGE_WIDTH * KT_IMAGE_HEIGHT * (32/8))
#define sizeIMAGEBuffer (sizeof(float) * tex_width * tex_height * (32/8))
#define sizeTRANSFERBuffer (sizeof(unsigned char) * tex_width * tex_height * (32/8))

#define KT_HISTORY_BUFFER 512

const char parser_val_type[parser_num][64]   = {"iii",    "iiii",    "iiii",      "iiiiiis",  "uuuu",      "uuuu",   "u",        "iiiiii", "iii",    "uii",      "iii",       "c",        "",       "uuuu",        "",     "",    "",        "i",		  "i",     "c",    "i",    "iiiiii", "i",    "ii",     	   "ii",	"iiiiiiiiii", "i",	 "iiii", "s",        "v2",       "",      "",     "iii",  "ii",   "", "", "", "v2"};

typedef struct _tagKTFASTLOAD{
	UINT glTexNum;
	char* szTexture;
}KT_FASTLOAD;

class KTexture
{
public:
	static KT_FASTLOAD* ktf;
	static int m_nHistoryPtr;
	static int m_nRef;

	//データ格納
	static int* istack;
	static char** sstack;
	static char* m_szFontName;

	//メインバッファ
	static float* pMainBuffer;
	static float* pSrcBuffer;
	static float* tex_pixel;
	//テクスチャサイズ
	int tex_width;
	int tex_height;

	int m_operator;	//描画オペレータ

	//Push, Pop
	static float** tex_pixelStack;
	int m_pixel_stack_level;

	int m_render_channel; //RGBAチャンネル
	unsigned char max_r, max_g, max_b, max_a; //Color()

	static float* m_pTmp1;
	static float* m_pTmp2;
	static unsigned char* m_transfer;
	static char* m_szParseBuffer;
	static char* m_swap_buf;

	//static char** m_szSeparatedBuf;

	BOOL isAlreadyHasAA;
protected:
	//	virtual
	void Initialize();
	//void DeInitialize();

	//描画関数
	void DrawCircle();
	void DrawEllipse();
	void DrawRectangle();
    void Text();
	void Color();
    void Fill();
	//void SetBlendAlpha();
	void Gradient();
	void Perlin();
	void SinEnv();
	void SinPlasma();
	void Operator();
	void Update();
	void Transparent();
	void Push();
	void Pop();
	void Invert();
	void ChangeChannel();
	void Emboss();
	void Edge();
	void Sharp();
	void Smooth();
	void Check();
	void Srand();
	void DrawRoundRect();
	void Font();
	void DrawPolygon();
	void RgbToA();
	void Mono();
	void Move();
	void Blur();
	void Normal();
	void AntiAliase();
	void DisableAntiAliase();
	void DrawPolyline();

	int	StrToInt(const char* szInt);
	float StrToFloat(const char* szFloat);
	const char* FindNextComma(const char*szSearch);
	long GetGLBufferSize();

	//static void ((*gen_func[parser_num]))(KTexture*); //描画関数呼び出し用
	static void ((KTexture::*gen_func[]))(); //描画関数呼び出し用

#ifdef NOT64K
	void LoadTextureFromFile(const char* filename);
#endif

	void ProcessDIBCreation(HDC* hDestDC, HBITMAP* hBitmap, HBITMAP* hOldBitmap, UCHAR** ppPixels);	   //DIBSectionのバッファを構築
	void ProcessDIBDestruction(HDC hDestDC, HBITMAP hBitmap, HBITMAP hOldBitmap); //DIBSectionのバッファを破壊
	void ProcessPenAndBrushCreation(HDC hDestDC, HBRUSH* hBrush, HBRUSH* hOldBrush, HPEN* hPen, HPEN* hOldPen); //ペンとブラシを作成(max_r,g,bに基づく)
	void ProcessPenAndBrushDestruction(HDC hDestDC, HBRUSH* hBrush, HBRUSH* hOldBrush, HPEN* hPen, HPEN* hOldPen);//ペンとブラシを破棄
	//void DestructStack();	//Push,Pop用のスタックを破壊
	void ProcessTextureGeneration(const char* texture); //文字列解析とテクスチャの生成作業
	void ProcessTextureTransfer(GLuint tex_name);		//テクスチャの転送
	void ChangePixelData(GLfloat* pPrevious, GLfloat* pSrc, GLfloat* pOut, int width, int height); //任意のピクセルデータを書き換える
	//void ChangePixelData(GLubyte* pDest, long offset, UCHAR val, GLubyte* pOut = NULL, int x = -1, int y = -1, GLubyte* pSrc = NULL); //任意のピクセルデータを書き換える
	void MaskImage(int i, int j, GLfloat* original, GLfloat* dest, UCHAR color_num, GLfloat* mask, int mask_width, int mask_height, GLfloat mul, GLfloat add, GLfloat ifmin, GLfloat ifmax);
	//void MaskImage(int i, int j, GLfloat* original, GLfloat* dest, UCHAR color_num, int* mask, int mask_width, int mask_height, float mul, int add, int ifmin, int ifmax);
	void MakeEscapeSequenceFromString(char* szSrc);
	void DrawGDIBegin(HDC hDC, UCHAR* pTempPixels);
	void DrawGDIEnd  (UCHAR* pTempPixels);
	void PutPixel(GLfloat* pSrc, int i, int j, GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	void MakeItMono(GLfloat* pDestGL, GLfloat* pPixelGL);
	void PutPixelInUsual(GLfloat* pDest, GLfloat* pSrc);
	unsigned int GetSizeDef(const char* texture, int* width = NULL, int* height = NULL);
	void AntiAliaseRGBA(float* rgba_tex, float* blrgba_tex, unsigned long texsize_w,unsigned long texsize_h);
public:
	KTexture();
	static void ResetRef();
	static void ResetHistory();
	//unsigned int GetSeparetedTextures(const char* texture, char** psrc);
	void SetTextureSize(int width, int height); //テクスチャのサイズを変更する(デフォルトは256)
	//virtual
	void GenerateTextureIndirect(GLuint* gltex_name, const char* texture); //テクスチャを生成する(w/o glGenTexture())
	//virtual GLuint GenerateTexture(const char* texture);				  //テクスチャを生成する(w/  glGenTexture())
	//unsigned int GetTextureCount(const char* texture);
	//void GetMultiTextureEnviroment(KMultiTextureEnv* pMultiTextureEnv, const char* szTexture);
};
#endif