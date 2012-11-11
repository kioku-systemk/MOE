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
#define IMAGE_STACK 16						//Push,Pop�p�X�^�b�N��
#define DATA_STRING_STACK 16				//String�X�^�b�N��
#define DATA_STRING_STACK_LENGTH (1024*64)  //String�X�^�b�N�o�b�t�@
#define DATA_INTEGER_STACK 1024				//Integer�X�^�b�N��
#define PARSER_BUFFER_LENGTH (1024*64)		//�������͗p�o�b�t�@�T�C�Y
#define parser_num	38						//���[�h��
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
#define sizeDIBBuffer	(sizeof(GLubyte) * width * height * (bppDIB/8)) //CreateDIBSection()�Ŋm�ۂ��ꂽ�T�C�Y��GlobalSize()�ł͎擾�s�\
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

	//�f�[�^�i�[
	static int* istack;
	static char** sstack;
	static char* m_szFontName;

	//���C���o�b�t�@
	static float* pMainBuffer;
	static float* pSrcBuffer;
	static float* tex_pixel;
	//�e�N�X�`���T�C�Y
	int tex_width;
	int tex_height;

	int m_operator;	//�`��I�y���[�^

	//Push, Pop
	static float** tex_pixelStack;
	int m_pixel_stack_level;

	int m_render_channel; //RGBA�`�����l��
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

	//�`��֐�
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

	//static void ((*gen_func[parser_num]))(KTexture*); //�`��֐��Ăяo���p
	static void ((KTexture::*gen_func[]))(); //�`��֐��Ăяo���p

#ifdef NOT64K
	void LoadTextureFromFile(const char* filename);
#endif

	void ProcessDIBCreation(HDC* hDestDC, HBITMAP* hBitmap, HBITMAP* hOldBitmap, UCHAR** ppPixels);	   //DIBSection�̃o�b�t�@���\�z
	void ProcessDIBDestruction(HDC hDestDC, HBITMAP hBitmap, HBITMAP hOldBitmap); //DIBSection�̃o�b�t�@��j��
	void ProcessPenAndBrushCreation(HDC hDestDC, HBRUSH* hBrush, HBRUSH* hOldBrush, HPEN* hPen, HPEN* hOldPen); //�y���ƃu���V���쐬(max_r,g,b�Ɋ�Â�)
	void ProcessPenAndBrushDestruction(HDC hDestDC, HBRUSH* hBrush, HBRUSH* hOldBrush, HPEN* hPen, HPEN* hOldPen);//�y���ƃu���V��j��
	//void DestructStack();	//Push,Pop�p�̃X�^�b�N��j��
	void ProcessTextureGeneration(const char* texture); //�������͂ƃe�N�X�`���̐������
	void ProcessTextureTransfer(GLuint tex_name);		//�e�N�X�`���̓]��
	void ChangePixelData(GLfloat* pPrevious, GLfloat* pSrc, GLfloat* pOut, int width, int height); //�C�ӂ̃s�N�Z���f�[�^������������
	//void ChangePixelData(GLubyte* pDest, long offset, UCHAR val, GLubyte* pOut = NULL, int x = -1, int y = -1, GLubyte* pSrc = NULL); //�C�ӂ̃s�N�Z���f�[�^������������
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
	void SetTextureSize(int width, int height); //�e�N�X�`���̃T�C�Y��ύX����(�f�t�H���g��256)
	//virtual
	void GenerateTextureIndirect(GLuint* gltex_name, const char* texture); //�e�N�X�`���𐶐�����(w/o glGenTexture())
	//virtual GLuint GenerateTexture(const char* texture);				  //�e�N�X�`���𐶐�����(w/  glGenTexture())
	//unsigned int GetTextureCount(const char* texture);
	//void GetMultiTextureEnviroment(KMultiTextureEnv* pMultiTextureEnv, const char* szTexture);
};
#endif