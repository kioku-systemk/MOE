#pragma once
#include <shlwapi.h>
#include <stdio.h>
#include "kTexture.h"


//Word List
const char parser[parser_num][64]		     = {"circle", "ellipse", "rectangle", "text",    "color",    "fill",  "reserved", "reserved",   "perlin", "sinenv", "sinplasma", "operator", "update", "transparent", "push", "pop", "invert", "changechannel", "channel", "op", "emboss", "roundrect", "edge", "sharp",  "smooth", "check",		"srand", "rect", "font", "polygon", "rgbtoa", "mono", "move", "blur", "normal","anti","noaa", "polyline"};	//�g�p�\���[�h���X�g

class KTextureEdit : public KTexture
{
private:
	bool IsEof(FILE* fp);
	GLubyte* m_pLatest;
public:
	KTextureEdit();
	~KTextureEdit();
	int GetWidth();
	int GetHeight();

	int Load(const char* szFile, char* szRecv, int nSize);
	UCHAR* GetImagePixel();
	void GenerateTextureIndirect(GLuint* gltex_name, const char* texture); //�e�N�X�`���𐶐�����(w/o glGenTexture())
	//GLuint GenerateTexture(const char* texture);				  //�e�N�X�`���𐶐�����(w/  glGenTexture())
	void DestructStack();
	void DeInitialize();
};