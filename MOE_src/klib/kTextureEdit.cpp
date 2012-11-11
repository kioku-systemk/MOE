#include "stdafx.h"
#include "kTextureEdit.h"
#pragma comment(lib, "shlwapi.lib")

KTextureEdit::KTextureEdit(){
	m_pLatest = NULL;
}

KTextureEdit::~KTextureEdit(){
	//for(int i=0; i<KT_HISTORY_BUFFER; i++)
	//	if(ktf[i].szTexture)
	//		GlobalFree(ktf[i].szTexture);
	//GlobalFree(ktf);

	//DestructStack();
	//DeInitialize();
}

bool KTextureEdit::IsEof(FILE* fp)
{
	long fpos = ftell(fp);
	fgetc(fp);
	if(feof(fp)==0)
	{
		fseek(fp, fpos, SEEK_SET);
		return false;
	}
	return true;
}

int KTextureEdit::Load(const char* szFile, char* szRecv, int nSize)
{
	FILE* fp = fopen(szFile, "rt");
	char szHeader[4];
	int i;
	for(i=0;i<3; i++) szHeader[i] = fgetc(fp);
	szHeader[i] = '\0';
	if(!StrStrI(szHeader, "KTF"))
	{
		fclose(fp);
		return FALSE;
	}
	//fseek(fp, 0, SEEK_SET);
	ZeroMemory(szRecv, nSize);
	for(int i=0;;i++)
	{
		szRecv[i] = fgetc(fp);
		if(IsEof(fp)) break;
	}
	fclose(fp);
	return TRUE;
}

void KTextureEdit::DeInitialize()
{
	//if(m_pLatest){
	//	GlobalFree(m_pLatest);
	//	m_pLatest = NULL;
	//}
	GlobalFree(m_szParseBuffer);
	GlobalFree(m_swap_buf);
	GlobalFree(m_szFontName);
	GlobalFree(m_pLatest);
	GlobalFree(m_pTmp1);
	GlobalFree(m_pTmp2);
	GlobalFree(pSrcBuffer);
	GlobalFree(pMainBuffer);
	GlobalFree(tex_pixel);
}

void KTextureEdit::DestructStack()	//DIBSection用スタックの破壊
{
	int i;
	for(i=0; i<IMAGE_STACK; i++){
		GlobalFree(tex_pixelStack[i]);//OpenGLへの転送用スタックの破壊
	}
	GlobalFree(tex_pixelStack);

	GlobalFree(istack);	//文字解析用スタックの破壊

	for(i=0; i<DATA_STRING_STACK; i++) GlobalFree(sstack[i]);
	GlobalFree(sstack);
}

void KTextureEdit::GenerateTextureIndirect(GLuint* tex_name, const char* texture)
{
	//KTexture::Initialize();

	//ktf[m_nHistoryPtr].glTexNum = tex_name;
	//ktf[m_nHistoryPtr].szTexture = (char*)GlobalAlloc(GPTR, sizeof(char)*(lstrlen(texture)+1));
	//lstrcpy(ktf[m_nHistoryPtr].szTexture, texture);
	//if(++m_nHistoryPtr>=KT_HISTORY_BUFFER){
	//	for(int i=0; i<KT_HISTORY_BUFFER; i++)
	//		if(ktf[i].szTexture)
	//			GlobalFree(ktf[i].szTexture);
	//	m_nHistoryPtr = 0;
	//}
	//Initialize();

	//char* szBuf;
	//unsigned int nTexture = this->GetSeparetedTextures(texture, &szBuf);
	//glGenTextures(nTexture, tex_name);//テクスチャを生成
 //   for(unsigned int i=0; i<nTexture; i++, szBuf++){
 //       ProcessTextureGeneration(szBuf); //テクスチャの生成
	//	ProcessTextureTransfer(tex_name[i]);  //OpenGLへ転送
	//}
	//GlobalFree(szBuf);

	KTexture::GenerateTextureIndirect(tex_name, texture);

	int width = tex_width, height = tex_height;
	//if(m_pLatest){
	//	GlobalFree(m_pLatest);
	//	m_pLatest = NULL;
	//}
	//m_pLatest = (UCHAR*)GlobalAlloc(GPTR, sizeGLBuffer);
	//CopyMemory(m_pLatest, tex_pixel, sizeGLBuffer);
}
/*
GLuint KTextureEdit::GenerateTexture(const char* texture)
{
	if(texture[0]=='\0') return 0;
	
	//for(int i=0; i<m_nHistoryPtr; i++){
	//	if(lstrcmp(ktf[i].szTexture, texture)==0){
	//		return ktf[i].glTexNum;
	//	}
	//}

	UINT name = 0;
	glGenTextures(1, &name);//テクスチャを生成
	GenerateTextureIndirect(name, texture);
	return name;
}
*/

UCHAR* KTextureEdit::GetImagePixel(){ return m_pLatest; }

int KTextureEdit::GetWidth(){
	return tex_width;
}

int KTextureEdit::GetHeight(){
	return tex_height;
}
