#include "stdafx.h"
#include "./KTexture.h"
#include "./kfpu.h"


#ifdef  NOT64K
//for External Texture
#include <windows.h>
#include <olectl.h>
//#pragma comment( lib, "OlePro32.lib" )
#endif


#ifndef NOT64K
#undef CopyMemory
#undef FillMemory
#undef ZeroMemory
#define CopyMemory(dst, src, bf) kmemcpy(dst, src, bf)
#define FillMemory(dst, sz, bf) kFillMemory(dst, sz, bf)
#define ZeroMemory(ptr, sz)		kZeroMemory(ptr, sz)
#endif


int			KTexture::m_nRef		= 0;
int*		KTexture::istack		= NULL;
char**		KTexture::sstack		= NULL;
char*		KTexture::m_szFontName	= NULL;

GLfloat*	KTexture::pMainBuffer	= NULL;
GLfloat*	KTexture::pSrcBuffer	= NULL;
GLfloat*	KTexture::tex_pixel		= NULL;
GLfloat**	KTexture::tex_pixelStack= NULL;
GLfloat*	KTexture::m_pTmp1		= NULL;
GLfloat*	KTexture::m_pTmp2		= NULL;
GLubyte*	KTexture::m_transfer	= NULL;

char*		KTexture::m_szParseBuffer = NULL;
char*		KTexture::m_swap_buf	= NULL;
KT_FASTLOAD* KTexture::ktf			= NULL;
int			KTexture::m_nHistoryPtr = 0;
//char**		KTexture::m_szSeparatedBuf = NULL;

void (KTexture::*KTexture::gen_func[])() = {
	&DrawCircle,
	&DrawEllipse,
	&DrawRectangle,
	&Text,
	&Color,
	&Fill,
	NULL,
	&Gradient,
	&Perlin,
	&SinEnv,
	&SinPlasma,
	&Operator,
	&Update,
	&Transparent,
	&Push,
	&Pop,
	&Invert,
	&ChangeChannel,
	&ChangeChannel,
	&Operator,
	&Emboss,
	&DrawRoundRect,
	&Edge,
	&Sharp,
	&Smooth,
	&Check,
	&Srand,
	&DrawRectangle,
	&Font,
	&DrawPolygon,
	&RgbToA,
	&Mono,
	&Move,
	&Blur,
	&Normal,
	&AntiAliase,
	&DisableAntiAliase,
	&DrawPolyline
};

KTexture::KTexture(){
	tex_width	= 256;
	tex_height	= 256;
}

int KTexture::StrToInt(const char* szInt){
	char* pRead = (char*)szInt;
	if(!pRead) return 0;
	int cnt = 0;
	int dig = 1;
	int ans = 0;
	int sign = 1;

	if(pRead[0] == '-'){
		pRead++;
		sign=-1;
	}
	while(('0' <= *pRead && *pRead <= '9')){//StrToIntの仕様で、数字以外の文字を見つけるまでをINTに変換するとされている。
		cnt++;
		pRead++;
		//if(*pRead=='\0' || *pRead==0) break;
	}
	if(cnt==0) return 0;
	
	if(cnt>1){//i==1 : dig=1 , i==2 : dig=10;
		for(int i=0; i<cnt-1; i++){
			dig*=10;
		}
	}

	pRead = (char*)szInt;
	if(sign<0) pRead++;
	while(dig>0){
		ans += dig * (*(pRead++) - '0');
		dig/=10;
	}
	return ans * sign;
}

float KTexture::StrToFloat(const char* szFloat){
	char* pRead = (char*)szFloat;
	if(!pRead || pRead[0]=='\0') return 0.0f;

	//整数部分の切り出し
	int iSection = StrToInt(szFloat);
	float ans = (float)iSection;
	unsigned short sign = 1;

	if(pRead[0] == '-'){
		pRead++;
		sign=-1;
	}
	while(*pRead != '.'){
		if('0' > *pRead || *pRead > '9')	return ans;
		//if('0' >= *pRead || *pRead >= '9')	return ans;
		pRead++;
	}
	float dig = 10.0f;
	while(dig<=100000000.0f){
		pRead++;
		if('0' <= *pRead && *pRead <= '9'){
			if(sign==1)
				ans += (*pRead - '0')/dig;
			else
				ans -= (*pRead - '0')/dig;
			dig*=10.0f;
		}else{
			break;
		}
	}
	return ans;
}

void KTexture::ResetRef(){
	m_nRef = 0;
}

void KTexture::ResetHistory(){
	m_nHistoryPtr = 0;
}

#define UPDATE_IMAGE_BUFFER(type_info__,pointer_to_buffer__,sizetoalloc__) {\
	MEMORY_BASIC_INFORMATION mbi;\
	VirtualQuery(pointer_to_buffer__, &mbi, sizeof(MEMORY_BASIC_INFORMATION));\
	if( mbi.RegionSize < sizetoalloc__ ){\
		VirtualFree(pointer_to_buffer__, 0, MEM_RELEASE);\
		pointer_to_buffer__  = (type_info__*)VirtualAlloc(NULL, sizetoalloc__, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);\
	}\
}

__inline void KTexture::Initialize()
{
	ksrand(1);
	gen_pnoize(); //パーリン準備

	//float* pBufferTbl[] = { pSrcBuffer, pMainBuffer, tex_pixel, m_pTmp1, m_pTmp2 };
	//for(int i=0; i<sizeof(pBufferTbl

	UPDATE_IMAGE_BUFFER(float, pSrcBuffer, sizeIMAGEBuffer);
	UPDATE_IMAGE_BUFFER(float, pMainBuffer, sizeIMAGEBuffer);
	UPDATE_IMAGE_BUFFER(float, tex_pixel, sizeIMAGEBuffer);
	UPDATE_IMAGE_BUFFER(float, m_pTmp1, sizeIMAGEBuffer);
	UPDATE_IMAGE_BUFFER(float, m_pTmp2, sizeIMAGEBuffer);
	UPDATE_IMAGE_BUFFER(unsigned char, m_transfer, sizeTRANSFERBuffer);
	for(int i=0; i<IMAGE_STACK; i++){
		UPDATE_IMAGE_BUFFER(float, tex_pixelStack[i], sizeIMAGEBuffer);
	}

	m_operator = 'r';
	max_r = max_g = max_b = max_a = 255;

	m_pixel_stack_level = 0;
	m_render_channel	= KT_B|KT_G|KT_R;

	int width = tex_width, height = tex_height;
	for(int j=0; j<height; j++){
		for(int i=0; i<width; i++){
			int t = posGLInternal(i,j);
			pSrcBuffer[t+0] = pMainBuffer[t+0] = tex_pixel[t+0] = //0.0f;
			pSrcBuffer[t+1] = pMainBuffer[t+1] = tex_pixel[t+1] = //0.0f;
			pSrcBuffer[t+2] = pMainBuffer[t+2] = tex_pixel[t+2] = 0.0f;
			pSrcBuffer[t+3] = pMainBuffer[t+3] = tex_pixel[t+3] = 255.0f;
		}
	}
	lstrcpy(m_szFontName, "Arial");
}

void KTexture::SetTextureSize(int width, int height)
{
	tex_width = width;
	tex_height = height;
	//OutputDebugString("KTexture::SetTextureSize() : NOT-IMPL");
	//tex_width	= width;
	//tex_height	= height;
}

unsigned int KTexture::GetSizeDef(const char* texture, int* width, int* height){
	if(texture[0]!='?'){
		*width = *height = 256;
		return 0;
	}

	//int nDefLength = 0;
	char* szValue = (char*)GlobalAlloc(GPTR, sizeof(char) * 32);
	int* write = NULL;

	int i;
	for(i=0; texture[i]!=';'; i++){
		//nDefLength++;
		if(texture[i]==','){
			i++;
			//nDefLength++;			
			if(width==NULL || height==NULL){
				continue;
			}

			ZeroMemory(szValue, (unsigned long)GlobalSize(szValue));
			for(int j=i; texture[j]!=',' && texture[j]!=';'; j++){
				szValue[j-i] = texture[j];
			}
			write = (write==NULL) ? width : height;
			*write = StrToInt(szValue);
		}
	}

	GlobalFree(szValue);
	return i+sizeof(';');//;の次をさすため
}

void KTexture::GenerateTextureIndirect(GLuint* tex_name, const char* texture)
{
	if(texture[0]=='\0') return;//this is not a texture!

	//optimal performance
	for(int i=0; i<m_nHistoryPtr; i++){
		if(lstrcmp(ktf[i].szTexture, texture)==0){
			*tex_name = ktf[i].glTexNum;
			return;
		}
	}

	if(++m_nRef==1){//インスタンス毎に確保・解放する必要性のないバッファ
		ktf = (KT_FASTLOAD*)GlobalAlloc(GPTR, sizeof(KT_FASTLOAD) * KT_HISTORY_BUFFER);
		for(int i=0; i<KT_HISTORY_BUFFER; i++){
			ktf[i].szTexture = (char*)GlobalAlloc(GPTR, HISTORY_BUFFER_TEXT_LENGTH);
		}
		m_szFontName	= (char*)GlobalAlloc(GPTR, sizeof(char) * 256);
		m_szParseBuffer = (char*)GlobalAlloc(GPTR, sizeof(char) * PARSER_BUFFER_LENGTH);
		m_swap_buf		= (char*)GlobalAlloc(GPTR, sizeof(char) * PARSER_BUFFER_LENGTH);

		istack	  = (int*)  GlobalAlloc(GPTR, sizeof(int)   * DATA_INTEGER_STACK); //文字解析用スタック構築
		sstack    = (char**)GlobalAlloc(GPTR, sizeof(char*) * DATA_STRING_STACK);
		for(int i=0; i<DATA_STRING_STACK; i++) sstack[i] = (char*)GlobalAlloc(GPTR, sizeof(char) * DATA_STRING_STACK_LENGTH);	
	
		////use VirtualAlloc!
		//{
		//	pSrcBuffer  = (GLfloat*)VirtualAlloc(NULL, sizeIMAGEBuffer, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);	//OpenGL転送用バッファ作成
		//	pMainBuffer = (GLfloat*)VirtualAlloc(NULL, sizeIMAGEBuffer, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);//OpenGL転送用バッファ作成
		//	tex_pixel   = (GLfloat*)VirtualAlloc(NULL, sizeIMAGEBuffer, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);	//OpenGL転送用バッファ作成
		//	m_pTmp1		= (GLfloat*)VirtualAlloc(NULL, sizeIMAGEBuffer, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);; //32ビットカラーで作成
		//	m_pTmp2		= (GLfloat*)VirtualAlloc(NULL, sizeIMAGEBuffer, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE); //32ビットカラーで作成
		//	m_transfer  = (GLubyte*)VirtualAlloc(NULL, sizeTRANSFERBuffer, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);;
		//	tex_pixelStack   = (GLfloat**)VirtualAlloc(NULL, sizeof(GLfloat*)*IMAGE_STACK, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);	//OpenGLへの転送用スタック構築
		//	for(int i=0; i<IMAGE_STACK; i++){
		//		tex_pixelStack[i]   = (GLfloat*)VirtualAlloc(NULL, sizeIMAGEBuffer, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
		//	}
		//}
		tex_pixelStack   = (GLfloat**)GlobalAlloc(GPTR, sizeof(GLfloat*) * IMAGE_STACK);	//OpenGLへの転送用スタック構築
	}

#ifdef NOT64K
	if(texture[0]=='@'){//This is External File Texture!
		LoadTextureFromFile(&texture[1]);
	}else{
#endif

	int nSizeDefLength = GetSizeDef(texture, &tex_width, &tex_height);
	Initialize();//オペレータや画素をクリアし，textureがさすテクスチャに適切な解像度分のメモリを確保する
	
	//TERA - coding your life -
	isAlreadyHasAA = FALSE;
	int tend = lstrlen(texture);
	if(texture[tend-1] == ';' && 
	   texture[tend-2] == 'd' &&
	   texture[tend-3] == ';' ){
		isAlreadyHasAA = TRUE;
	}

	//テクスチャの生成
  	ProcessTextureGeneration(texture+nSizeDefLength);
	if(!isAlreadyHasAA){
		this->AntiAliase();
	}

#ifdef NOT64K
	}
#endif

	//generate
	glGenTextures(1, tex_name);
	ProcessTextureTransfer(*tex_name);  //OpenGLへ転送

	//optimize
	ktf[m_nHistoryPtr].glTexNum = *tex_name;
	lstrcpy(ktf[m_nHistoryPtr].szTexture, texture);
	if(++m_nHistoryPtr>=KT_HISTORY_BUFFER) m_nHistoryPtr = 0;
}

#ifdef NOT64K

// モジュールのパス取得
void GetModulePath(char* cPath)
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
	int nCount;

	// モジュールのフルパスを探す
	GetModuleFileName( hInstance, cPath, MAX_PATH );

	// モジュール名を切り離す
	for(nCount = (strlen(cPath) - 1); nCount >= 0; nCount--){
		if( cPath[ nCount ] == '\\' ){
			if( cPath[ nCount + 1 ] != '\0' ){
				cPath[ nCount + 1 ] = '\0'; // 切り離す
				return;
			}
		}
	}
	return;
}

void KTexture::LoadTextureFromFile(const char* filename)
{
	//path -> ..実行ファイルパス\\tex\\filename
	char texFilepath[MAX_PATH] = {0};
	GetModulePath( texFilepath );
	strcat(texFilepath, "tex\\");
	strcat(texFilepath, filename);

	HANDLE hFile;
	unsigned long dw;
	if((hFile = CreateFile(texFilepath,GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		char msgbuf[1024];
		wsprintf(msgbuf, "%s%s", "Can't open file:", texFilepath);
		MessageBox(NULL, msgbuf, "error", MB_OK );
		return;
	}

	long fsize = GetFileSize(hFile,NULL);
	unsigned char* lpBuf = (unsigned char*)GlobalAlloc(GPTR,fsize);
	ReadFile(hFile, lpBuf, fsize, &dw, NULL);
	CloseHandle(hFile);

	//IPictureによる画像読み込み
	IPicture*   lpiPict = NULL ;
	IStream*    iStream = NULL ;
	if( SUCCEEDED(CreateStreamOnHGlobal( (HGLOBAL)lpBuf, FALSE, &iStream ))){
		if( FAILED(OleLoadPicture( iStream, fsize, TRUE, IID_IPicture, (void**)&lpiPict))){
			return;
		}
		
		GlobalFree(lpBuf);
		if(iStream != NULL){
			iStream->Release();
			iStream = NULL ;
		}
		
		// 画像の大きさを取得.
		long lx,ly;
		lpiPict->get_Width( &lx ) ;
		lpiPict->get_Height( &ly ) ;
		HDC dhdc = ::GetDC(0);
		tex_width = MulDiv( lx, GetDeviceCaps( dhdc, LOGPIXELSX), 2540 ) ;
		tex_height = MulDiv( ly, GetDeviceCaps( dhdc, LOGPIXELSY), 2540 ) ;
		ReleaseDC(0,dhdc);

		//オペレータや画素をクリアし，textureがさすテクスチャに適切な解像度分のメモリを確保する
		Initialize();

		//画像サイズのDIBを作成
		HDC hDC;
		HBITMAP hBitmap, hOldBitmap;
		UCHAR* pTempPixels;
		ProcessDIBCreation(&hDC, &hBitmap, &hOldBitmap, &pTempPixels);

		DrawGDIBegin(hDC, pTempPixels);
			lpiPict->Render(hDC ,0,0,tex_width,tex_height, 0,ly,lx,-ly,NULL);//IPictureによりDIBに描画
		DrawGDIEnd(pTempPixels);

		ProcessDIBDestruction(hDC, hBitmap, hOldBitmap);

		if(lpiPict != NULL) lpiPict->Release();//解放
	}
}
#endif

void KTexture::ProcessDIBCreation(HDC* hDestDC, HBITMAP* hBitmap, HBITMAP* hOldBitmap, UCHAR** ppPixels)
{
	int width = tex_width, height = tex_height;
	BITMAPINFO	BmpInfo;
	ZeroMemory(&BmpInfo, sizeof(BITMAPINFO));
	BmpInfo.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	BmpInfo.bmiHeader.biBitCount	= bppDIB;	//32bitで作成しても, GDI描画APIが正常に動作しない.
	BmpInfo.bmiHeader.biPlanes		= 1;
	BmpInfo.bmiHeader.biWidth		= width;
	BmpInfo.bmiHeader.biHeight		= height;
	BmpInfo.bmiHeader.biSizeImage	= sizeDIBBuffer;
	BmpInfo.bmiHeader.biCompression = BI_RGB;
	(*hBitmap) = CreateDIBSection(NULL, &BmpInfo, DIB_RGB_COLORS, (void**)(ppPixels), NULL, 0);
	
	HDC hDC = GetDC(HWND_DESKTOP);
		(*hDestDC) = CreateCompatibleDC(hDC); //メモリデバイスコンテキストを作成.
	ReleaseDC(HWND_DESKTOP, hDC);

	*hOldBitmap = (HBITMAP)SelectObject(*hDestDC, (HGDIOBJ)*hBitmap); //DIBSectionモードに移行
}

void KTexture::ProcessDIBDestruction(HDC hDestDC, HBITMAP hBitmap, HBITMAP hOldBitmap)
{
	SelectObject(hDestDC, hOldBitmap); //DIBSectionモードを抜ける
	DeleteObject(hBitmap);				//CreateDIBSectionで確保されたメモリを解放させる
	DeleteDC(hDestDC);					//メモリデバイスコンテキストの破棄
}

void KTexture::ProcessPenAndBrushCreation(HDC hDestDC, HBRUSH* hBrush, HBRUSH* hOldBrush, HPEN* hPen, HPEN* hOldPen)
{
	//ブラシとペンの色を統一すれば図形の縁は描画されない
	*hBrush = CreateSolidBrush(RGB(max_r, max_g, max_b));
	*hOldBrush = (HBRUSH)SelectObject(hDestDC, *hBrush);

	*hPen   = CreatePen(PS_SOLID, 0, RGB(max_r, max_g, max_b));
	*hOldPen   = (HPEN)SelectObject(hDestDC, *hPen);
}

void KTexture::ProcessPenAndBrushDestruction(HDC hDestDC, HBRUSH* hBrush, HBRUSH* hOldBrush, HPEN* hPen, HPEN* hOldPen)
{
	SelectObject(hDestDC, *hOldBrush);
	DeleteObject(*hBrush);
	*hBrush = *hOldBrush = NULL;

	SelectObject(hDestDC, *hOldPen);
	DeleteObject(*hPen);
	*hPen = *hOldPen = NULL;
}

__inline void KTexture::MakeEscapeSequenceFromString(char* szSrc)
{
	int len = lstrlen(szSrc)+1;
	int diff=0;
	char* tmp = (char*)GlobalAlloc(GPTR, sizeof(char)*len);

	for(int i=0; i<len; i++)
	{
		if(szSrc[i+diff+0]=='\\'){
			if(szSrc[i+diff+1]=='\\'){
				tmp[i] = '\\';
			}else if(szSrc[i+diff+1]=='t'){
				tmp[i] = '\t';
			}else if(szSrc[i+diff+1]=='n'){
				tmp[i] = '\n';
			}else if(szSrc[i+diff+1]=='"'){
				tmp[i] = '"';
			}
			diff+=1;
		}else{
			tmp[i] = szSrc[i+diff];
		}
	}
	lstrcpy(szSrc, tmp);
	GlobalFree(tmp);
}

const char* KTexture::FindNextComma(const char* szSearch){
	const char* p = szSearch;
	while(*p!=',' && *p!='\0'){	p++; }
	return ++p;
}

void KTexture::ProcessTextureGeneration(const char* texture)
{
	lstrcpy(m_szParseBuffer, texture);

	int j, k;
	while(m_szParseBuffer[0]!='\0')
	//for(;m_szParseBuffer[0]!='\0';)
	//for(;m_szParseBuffer[0]!='\n';)//multi texture
	{
		int found_index = m_szParseBuffer[0]-'A'; //関数の番号を格納
		if(found_index<0 || found_index>=parser_num) break;

		char* p = m_szParseBuffer;
		int parse_point = 0;
		int isText = 0;
		//bool isText = false;
		while(1) //次のパースポイントを探す(TOKEN) //Convert.cppとはちがう.
		{
			//if(*p=='"' && (*(p-1)!='\\' && *(p-2)!='\\'))				 isText = !isText;
			if(*p=='"'){
				isText = !isText;
				if(*(p-1)=='\\' && *(p-2)!='\\' && !isText)	isText = !isText;
			}
			else if((!isText) && *p==TOKEN) break; //テキスト中の;には反応しないため
			p++;
			parse_point++;
		}
		m_szParseBuffer[parse_point] = '\0';	//つめておく

		char* c = m_szParseBuffer;
		if(parser_val_type[found_index][0]=='v'){
			c = const_cast<char*>(FindNextComma(c));
			istack[0]=StrToInt(c);
			for(j=0; j<istack[0]*(parser_val_type[found_index][1]-'0'); j++) //引数リストの2番目ずつ、istack[0]回読み込む
			{
				c = const_cast<char*>(FindNextComma(c));
				istack[j+1]=StrToInt(c);
			}
		}else{
			for(j=0; j<lstrlen(parser_val_type[found_index]); j++) //lstrlen()だけでいい
			{
				c = const_cast<char*>(FindNextComma(c));
				switch(parser_val_type[found_index][j])
				{
				case 'c':
					istack[j] = *c;
					break;
				case 'i':
				case 'u':
					istack[j] = StrToInt(c);
					break;
				case 's':
					for(k=0;k<lstrlen(c)+1-2-1;k++)	//""の分を削除
					{
						sstack[j][k] = c[k+1];
					}
					sstack[j][k] = '\0';
					MakeEscapeSequenceFromString(sstack[j]);
					break;
				}
			}
		}
		//(*gen_func[found_index])(this);
		/*if(this->*gen_func[found_index]!=NULL)*/
		(this->*gen_func[found_index])();

		m_szParseBuffer[parse_point] = TOKEN;
		int limit = lstrlen(m_szParseBuffer)+1; //保存しておく必要がある
		CopyMemory(m_swap_buf, m_szParseBuffer, PARSER_BUFFER_LENGTH);
		//ZeroMemory(m_szParseBuffer, PARSER_BUFFER_LENGTH);
		for(j=0; j<(limit - parse_point); j++) //次のループのために,使用した分を消去する
		{
			m_szParseBuffer[j] = m_swap_buf[j+(parse_point+1)];
		}
	}
}

void KTexture::ProcessTextureTransfer(GLuint tex_name)
{
	int width = tex_width, height = tex_height;
	for(int i=0; i<width*height*4; i++){
		m_transfer[i] = (unsigned char)tex_pixel[i];
	}

	//テクスチャとして登録する
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_name);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// Linear Filtering
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_transfer);
}

void KTexture::PutPixel(GLfloat* pSrc, int i, int j,GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	int width = tex_width, height = tex_height;
	int tex = posGLInternal(i,j);
	pSrc[tex+3] = a;
	pSrc[tex+2] = b;
	pSrc[tex+1] = g;
	pSrc[tex+0] = r;
}

const short Channels[4] = {KT_R, KT_G, KT_B, KT_A};
void KTexture::ChangePixelData(GLfloat* pPrevious, GLfloat* pSrc, GLfloat* pOut, int width, int height)
{
	int i,j,k;

	for(j=0; j<height; j++){
		for(i=0; i<width; i++){
			int tpos = posGLInternal(i,j);
			//Channelによる制限を受けないオペレータ
			switch(m_operator){
				case 'n':{//normal
					float px=(width*0.5f/DEFAULT_TEX_SIZE*width),py=(height*0.5f/DEFAULT_TEX_SIZE*height);//中心点を求める
					if(pPrevious[posGLInternal(i,j)+0]!=0){
						px = i + ((pPrevious[posGLInternal(i,j)+2]/255.0f-0.5f)*13.0f/DEFAULT_TEX_SIZE*width);
						py = j + ((pPrevious[posGLInternal(i,j)+1]/255.0f-0.5f)*13.0f/DEFAULT_TEX_SIZE*height);
					}
					if(px<=0) px=1.0f; if(px>=width-1) px=(float)(width-1-1);
					if(py<=0) py=1.0f; if(py>=height-1) py=(float)(height-1-1);
					long ix = (long)px;
					long iy = (long)py;
					float fx = (float)(px - ix);
					float fy = (float)(py - iy);

					pOut[tpos+0] = (UCHAR)(pSrc[posGLInternal(ix,iy)+0]*(1-fx)*(1-fy));
					pOut[tpos+1] = (UCHAR)(pSrc[posGLInternal(ix,iy)+1]*(1-fx)*(1-fy));
					pOut[tpos+2] = (UCHAR)(pSrc[posGLInternal(ix,iy)+2]*(1-fx)*(1-fy));
					pOut[tpos+3] = (UCHAR)(pSrc[posGLInternal(ix,iy)+3]*(1-fx)*(1-fy));

					pOut[tpos+0] += (UCHAR)(pSrc[posGLInternal(ix+1,iy)+0]*(fx)*(1-fy));
					pOut[tpos+1] += (UCHAR)(pSrc[posGLInternal(ix+1,iy)+1]*(fx)*(1-fy));
					pOut[tpos+2] += (UCHAR)(pSrc[posGLInternal(ix+1,iy)+2]*(fx)*(1-fy));
					pOut[tpos+3] += (UCHAR)(pSrc[posGLInternal(ix+1,iy)+3]*(fx)*(1-fy));

					pOut[tpos+0] += (UCHAR)(pSrc[posGLInternal(ix+1,iy+1)+0]*(fx)*(fy));
					pOut[tpos+1] += (UCHAR)(pSrc[posGLInternal(ix+1,iy+1)+1]*(fx)*(fy));
					pOut[tpos+2] += (UCHAR)(pSrc[posGLInternal(ix+1,iy+1)+2]*(fx)*(fy));
					pOut[tpos+3] += (UCHAR)(pSrc[posGLInternal(ix+1,iy+1)+3]*(fx)*(fy));

					pOut[tpos+0] += (UCHAR)(pSrc[posGLInternal(ix,iy+1)+0]*(1-fx)*(fy));
					pOut[tpos+1] += (UCHAR)(pSrc[posGLInternal(ix,iy+1)+1]*(1-fx)*(fy));
					pOut[tpos+2] += (UCHAR)(pSrc[posGLInternal(ix,iy+1)+2]*(1-fx)*(fy));
					pOut[tpos+3] += (UCHAR)(pSrc[posGLInternal(ix,iy+1)+3]*(1-fx)*(fy));
				}
				continue;
				
				case 'e':{//env
					float px=width*0.5f/DEFAULT_TEX_SIZE*width,py=height*0.5f/DEFAULT_TEX_SIZE*height;
					//int px=width*0.5f,py=height*0.5f;
					if(pPrevious[tpos]!=0){
						px += (pPrevious[tpos+2]/255.0f-0.5f)*128.0f/DEFAULT_TEX_SIZE*width;
						//py += (pPrevious[tpos+1]/255.0f-0.5f)*128.0f;
						py = 128.0f/DEFAULT_TEX_SIZE*height;
					}
					if(px<=0) px=1.0f; if(px>=width-1) px=(float)(width-1-1);
					if(py<=0) py=1.0f; if(py>=height-1) py=(float)(height-1-1);
					long ix = (long)px;
					long iy = (long)py;
					float fx = (float)(px - ix);
					float fy = (float)(py - iy);

					pOut[tpos+0] = (UCHAR)(pSrc[posGLInternal(ix,iy)+0]*(1-fx)*(1-fy));
					pOut[tpos+1] = (UCHAR)(pSrc[posGLInternal(ix,iy)+1]*(1-fx)*(1-fy));
					pOut[tpos+2] = (UCHAR)(pSrc[posGLInternal(ix,iy)+2]*(1-fx)*(1-fy));
					pOut[tpos+3] = (UCHAR)(pSrc[posGLInternal(ix,iy)+3]*(1-fx)*(1-fy));

					pOut[tpos+0] += (UCHAR)(pSrc[posGLInternal(ix+1,iy)+0]*(fx)*(1-fy));
					pOut[tpos+1] += (UCHAR)(pSrc[posGLInternal(ix+1,iy)+1]*(fx)*(1-fy));
					pOut[tpos+2] += (UCHAR)(pSrc[posGLInternal(ix+1,iy)+2]*(fx)*(1-fy));
					pOut[tpos+3] += (UCHAR)(pSrc[posGLInternal(ix+1,iy)+3]*(fx)*(1-fy));

					pOut[tpos+0] += (UCHAR)(pSrc[posGLInternal(ix+1,iy+1)+0]*(fx)*(fy));
					pOut[tpos+1] += (UCHAR)(pSrc[posGLInternal(ix+1,iy+1)+1]*(fx)*(fy));
					pOut[tpos+2] += (UCHAR)(pSrc[posGLInternal(ix+1,iy+1)+2]*(fx)*(fy));
					pOut[tpos+3] += (UCHAR)(pSrc[posGLInternal(ix+1,iy+1)+3]*(fx)*(fy));

					pOut[tpos+0] += (UCHAR)(pSrc[posGLInternal(ix,iy+1)+0]*(1-fx)*(fy));
					pOut[tpos+1] += (UCHAR)(pSrc[posGLInternal(ix,iy+1)+1]*(1-fx)*(fy));
					pOut[tpos+2] += (UCHAR)(pSrc[posGLInternal(ix,iy+1)+2]*(1-fx)*(fy));
					pOut[tpos+3] += (UCHAR)(pSrc[posGLInternal(ix,iy+1)+3]*(1-fx)*(fy));
				}
				continue;
			}
			for(k=0; k<4; k++){
				//if(m_render_channel&Channels[k]){
				//	long offset = tpos + k; //32ビットでのオフセットを決定
				//	GLfloat dest = pPrevious[offset];
				//	GLfloat val = pSrc[offset];
				//	if(val<0.0f){
				//	}else{
				//		float temp_val = val;
				//		switch(m_operator)
				//		{
				//			case '/'://division
				//				if(val!=0) temp_val = (float)(dest/(float)val)*255.0f;
				//				break;
				//			case '*'://mul
				//				temp_val = (float)(dest*val)/255.0f;
				//				break;
				//			case '+'://add
				//				temp_val = (float)(dest+val);//-255.0f;
				//				break;
				//			case '-'://subtract
				//				temp_val = (float)(dest-val);//+255.0f;
				//				break;
				//			case '%'://amari
				//				if(val!=0) temp_val = (float)((int)dest % (int)val);
				//				break;
				//			case '&'://AND
				//				temp_val = (float)((int)dest & (int)val);
				//				break;
				//			case '|'://OR
				//				temp_val = (float)((int)dest | (int)val);
				//				break;
				//			case '^'://XOR
				//				temp_val = (float)((int)dest ^ (int)val);
				//				break;
				//			case '~'://NOT
				//				temp_val =(float)(dest + val);
				//				temp_val = (float)(~(int)temp_val);
				//				break;
				//			case 'a'://alpha
				//				temp_val = (float)(dest*(1.0f-(pSrc[posGLInternal(i,j)+3]/255.0f)) + val*(pSrc[posGLInternal(i,j)+3]/255.0f));
				//				break;
				//		}
				//		if(temp_val<0.0f) temp_val = 0.0f;
				//		else if(temp_val>255.0f) temp_val = 255.0f;
				//		pOut[offset] = /*(UCHAR)*/temp_val;
				//	}
				//}
				if(m_render_channel&Channels[k]){
					long offset = tpos + k; //32ビットでのオフセットを決定
					GLfloat dest = pPrevious[offset];
					GLfloat val = pSrc[offset];
					if(val<0.0f){
					}else{
						float temp_val = val;
						switch(m_operator)
						{
							case '/'://division
								if(val!=0) temp_val = kfDiv(dest,val)*255.0f;
								break;
							case '*'://mul
								temp_val = kfDiv(kfMul(dest,val),255.0f);
								break;
							case '+'://add
								temp_val = kfAdd(dest,val);//-255.0f;
								break;
							case '-'://subtract
								temp_val = kfSub(dest,val);//+255.0f;
								break;
							case '%'://amari
								if(val!=0) temp_val = (float)((int)dest % (int)val);
								break;
							case '&'://AND
								temp_val = (float)((int)dest & (int)val);
								break;
							case '|'://OR
								temp_val = (float)((int)dest | (int)val);
								break;
							case '^'://XOR
								temp_val = (float)((int)dest ^ (int)val);
								break;
							case '~'://NOT
								temp_val = kfAdd(dest,val);
								temp_val = (float)(~(int)temp_val);
								break;
							case 'a'://alpha
								temp_val = (float)(dest*(1.0f-(pSrc[posGLInternal(i,j)+3]/255.0f)) + val*(pSrc[posGLInternal(i,j)+3]/255.0f));
								break;
						}
						if(temp_val<0.0f) temp_val = 0.0f;
						else if(temp_val>255.0f) temp_val = 255.0f;
						pOut[offset] = /*(UCHAR)*/temp_val;
					}
				}
			}
		}
	}
	CopyMemory(pPrevious, pOut, sizeGLBuffer);
}

void KTexture::DrawCircle()
{
	int x, y, w;
	w = istack[2];
	x = istack[0];
	y = istack[1];
	
	istack[0] = x-w;
	istack[1] = y-w;
	istack[2] = x+w;
	istack[3] = y+w;

	DrawEllipse();
}

void KTexture::DrawEllipse()
{
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;
	UCHAR* pTempPixels;
	HBRUSH hOldBrush, hBrush;
	HPEN hOldPen, hPen;
	ProcessDIBCreation(&hDC, &hBitmap, &hOldBitmap, &pTempPixels);
	ProcessPenAndBrushCreation(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	
	DrawGDIBegin(hDC, pTempPixels);
		Ellipse(hDC, (int)(istack[0]/DEFAULT_TEX_SIZE*tex_width+1), (int)(istack[1]/DEFAULT_TEX_SIZE*tex_height+1), (int)(istack[2]/DEFAULT_TEX_SIZE*tex_width), (int)(istack[3]/DEFAULT_TEX_SIZE*tex_height));
	DrawGDIEnd(pTempPixels);

	ProcessPenAndBrushDestruction(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	ProcessDIBDestruction(hDC, hBitmap, hOldBitmap);
}

void KTexture::DrawRectangle()
{
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;
	UCHAR* pTempPixels;
	HBRUSH hOldBrush, hBrush;
	HPEN hOldPen, hPen;
	ProcessDIBCreation(&hDC, &hBitmap, &hOldBitmap, &pTempPixels);
	ProcessPenAndBrushCreation(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	
	DrawGDIBegin(hDC, pTempPixels);
		Rectangle(hDC, (int)(istack[0]/DEFAULT_TEX_SIZE*tex_width+1), (int)(istack[1]/DEFAULT_TEX_SIZE*tex_height+1), (int)(istack[2]/DEFAULT_TEX_SIZE*tex_width+istack[0]/DEFAULT_TEX_SIZE*tex_width), (int)(istack[3]/DEFAULT_TEX_SIZE*tex_height+istack[1]/DEFAULT_TEX_SIZE*tex_height));
	DrawGDIEnd(pTempPixels);

	ProcessPenAndBrushDestruction(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	ProcessDIBDestruction(hDC, hBitmap, hOldBitmap);
}

void KTexture::Text()
{
	int width = tex_width, height = tex_height;
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;
	UCHAR* pTempPixels;
	HBRUSH hOldBrush, hBrush;
	HPEN hOldPen, hPen;
	ProcessDIBCreation(&hDC, &hBitmap, &hOldBitmap, &pTempPixels);
	ProcessPenAndBrushCreation(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	
	DrawGDIBegin(hDC, pTempPixels);
		RECT rect = {(int)(istack[0]/DEFAULT_TEX_SIZE*width+1), (int)(istack[1]/DEFAULT_TEX_SIZE*height+1), (int)(istack[0]/DEFAULT_TEX_SIZE*width+300000), (int)(istack[1]/DEFAULT_TEX_SIZE*height+istack[2]/DEFAULT_TEX_SIZE*height)};

		SetBkMode(hDC, TRANSPARENT);
		HFONT hFont = CreateFont((int)(istack[2]/DEFAULT_TEX_SIZE*height), 0, 0, 0, (istack[3] > 0) ? FW_BOLD : FW_NORMAL,
								(istack[4] > 0) ? TRUE : 0, 0, 0,
								ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, /*ANTIALIASED_QUALITY*/DEFAULT_QUALITY, 0, m_szFontName);
		HFONT holdFont = (HFONT)SelectObject(hDC, hFont);
		COLORREF oldColor = SetTextColor(hDC, RGB(max_r, max_g, max_b));
		UINT oldAlign = 0;
		if(istack[5] > 0){
			oldAlign = SetTextAlign(hDC , TA_CENTER | TA_BASELINE);
			rect.left += width/2;
			rect.top  += istack[2];
		}
		DrawText(hDC, sstack[6], -1, &rect,  DT_LEFT | DT_WORDBREAK | DT_NOCLIP);

		SetTextAlign(hDC, oldAlign);
		SetTextColor(hDC, oldColor);
		SelectObject(hDC, holdFont);
		DeleteObject(hFont);
	DrawGDIEnd(pTempPixels);

	ProcessPenAndBrushDestruction(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	ProcessDIBDestruction(hDC, hBitmap, hOldBitmap);
}

void KTexture::Color()
{
	max_r = istack[0];
	max_g = istack[1];
	max_b = istack[2];
	max_a = istack[3];
}

void KTexture::Fill()
{
	int width = tex_width, height = tex_height;
	int i,j;
	for(i=0; i<width; i++)
	{
		for(j=0; j<height; j++)
		{
			PutPixel(pSrcBuffer, i, j, (GLfloat)istack[0], (GLfloat)istack[1], (GLfloat)istack[2], (GLfloat)istack[3]);
		}
	}
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);
}

//void KTexture::SetBlendAlpha(KTexture* t)	//すべてのピクセルのAにaをセット
//{
//	t->m_BlendAlpha = istack[0];
//}

//void KTexture::Grad(KTexture* t)
//{
//}
#define NOIZE_RANGE		256
void KTexture::Perlin()
{
	int i,j;

	for(i=0; i<3;i++)
	{
		if(istack[i]<=0) istack[i] = 0;
	}
	if(istack[0]>istack[1]) istack[1] = istack[0];
	//if(istack[1]>=istack[2]) istack[2] = istack[1]+2;

	//pnoize()で死なないようにするための事前計算
	float ox,oy;//original
	float tx,ty;//for test
	ox = kfDiv((float)(tex_width-1), (kfMul(kfDiv((float)istack[2],DEFAULT_TEX_SIZE),(float)tex_width)));
	oy = kfDiv((float)(tex_height-1),(kfMul(kfDiv((float)istack[2],DEFAULT_TEX_SIZE),(float)tex_height)));
	BOOL isPassedTest = FALSE;
	while(!isPassedTest){
		tx = kfMul((float)istack[1],ox);
		tx+= kfDiv((float)istack[1],2);

		ty = kfMul((float)istack[1],oy);
		ty+= kfDiv((float)istack[1],2);
		if(tx+1>=NOIZE_RANGE || ty+1>=NOIZE_RANGE){
			istack[1] -= 2;
			//x=NOIZE_RANGE-2;
		}else{
			isPassedTest = TRUE;
		}
	}

	for(j=0; j<tex_height; j++)
	{
		for(i=0; i<tex_width; i++)
		{
			float perlin = pnoize(
				(int)istack[0],
				(float)istack[1],
				kfDiv((float)i,kfMul(kfDiv((float)istack[2],DEFAULT_TEX_SIZE),(float)tex_width)),
				kfDiv((float)j,kfMul(kfDiv((float)istack[2],DEFAULT_TEX_SIZE),(float)tex_height))
				);
			PutPixel(pSrcBuffer, i, j, (UCHAR)(max_r*perlin), (UCHAR)(max_g*perlin), (UCHAR)(max_b*perlin), (UCHAR)(max_a*perlin));
		}
	}
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, tex_width, tex_height);
}

void KTexture::SinEnv()
{
	if(istack[1] <= 0) istack[1] = 1;
	if(istack[2] <= 0) istack[2] = 1;
	int i,j;

	for(j=0; j<tex_height; j++)
	{
		for(i=0; i<tex_width; i++)
		{
			float sine = istack[0]/255.0f * sinenv( 
				i/((float)istack[1]/DEFAULT_TEX_SIZE*tex_width),
				j/((float)istack[2]/DEFAULT_TEX_SIZE*tex_height)
				);
			//float sine = (istack[0]/255.0f) * sinenv(i/(float)istack[1], j/(float)istack[2]);
			PutPixel(pSrcBuffer, i, j,(UCHAR)(max_r*sine), (UCHAR)(max_g*sine), (UCHAR)(max_b*sine), (UCHAR)(max_a*sine));
		}
	}
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, tex_width, tex_height);
}

void KTexture::SinPlasma()
{
	int i,j;
	for(j=0; j<tex_height; j++)
	{
		for(i=0; i<tex_width; i++)
		{
			float sinp = sinplasma(
				(float)i/((float)tex_width/DEFAULT_TEX_SIZE*tex_width),
				(float)j/((float)tex_height/DEFAULT_TEX_SIZE*tex_height),
				(float)(istack[0]/DEFAULT_TEX_SIZE*tex_width),
				(float)(istack[1])/255.0f/tex_width*DEFAULT_TEX_SIZE,
				(float)(istack[2])/255.0f/tex_height*DEFAULT_TEX_SIZE
				);
			PutPixel(pSrcBuffer, i, j,(UCHAR)(max_r*sinp), (UCHAR)(max_g*sinp), (UCHAR)(max_b*sinp), (UCHAR)(max_a*sinp));
		}
	}
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, tex_width, tex_height);

}

void KTexture::Operator()
{
	m_operator = istack[0];	//グローバルに参照できようにオペレータを格納
}

void KTexture::Update()	//状態を更新する
{
	PutPixelInUsual(pSrcBuffer, tex_pixel);
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, tex_width, tex_height);
}

void KTexture::Transparent()	//指定されたRGBに合致するピクセルのAに値を代入する
{
	int width = tex_width, height = tex_height;
	int i,j;
	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			if(tex_pixel[posGL(i,j)+0] == istack[0] 
			&& tex_pixel[posGL(i,j)+1] == istack[1]
			&& tex_pixel[posGL(i,j)+2] == istack[2]){
				tex_pixel[posGL(i,j)+3] = (float)istack[3];
				//PutPixel(pSrcBuffer, i, j, tex_pixel[posGL(i,j)+0], tex_pixel[posGL(i,j)+1],tex_pixel[posGL(i,j)+2],(UCHAR)istack[3]);
			}
		}
	}
	CopyMemory(pMainBuffer, tex_pixel, sizeGLBuffer);
	//ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, tex_width, tex_height);
}

void KTexture::Push()	//スタックに積む
{
	int width = tex_width, height = tex_height;
	int i, j;
	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			tex_pixelStack[m_pixel_stack_level][posGL(i,j)+3] = tex_pixel[posGL(i,j)+3];
			tex_pixelStack[m_pixel_stack_level][posGL(i,j)+2] = tex_pixel[posGL(i,j)+2];
			tex_pixelStack[m_pixel_stack_level][posGL(i,j)+1] = tex_pixel[posGL(i,j)+1];
			tex_pixelStack[m_pixel_stack_level][posGL(i,j)+0] = tex_pixel[posGL(i,j)+0];
		}
	}
	if(++m_pixel_stack_level>=IMAGE_STACK) m_pixel_stack_level--;
}

void KTexture::Pop() //元に戻す処理
{
	int width = tex_width, height = tex_height;
	if(--m_pixel_stack_level<0) m_pixel_stack_level++;
	PutPixelInUsual(pSrcBuffer, tex_pixelStack[m_pixel_stack_level]);
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);
}

void KTexture::Invert()
{
	int i,j;
	int width = tex_width, height = tex_height;
	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			PutPixel(pSrcBuffer,i, j,255 - (tex_pixel[posGL(i,j)+0]), 255 - (tex_pixel[posGL(i,j)+1]), 255 - (tex_pixel[posGL(i,j)+2]), 255 - (tex_pixel[posGL(i,j)+3]));
		}
	}
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);
}

void KTexture::ChangeChannel()
{
	m_render_channel = istack[0];
}

void KTexture::MaskImage(int i, int j, GLfloat* original, GLfloat* dest, UCHAR color_num, GLfloat* mask, int mask_width, int mask_height, GLfloat mul, GLfloat add, GLfloat ifmin, GLfloat ifmax)
{
	int x, y, loop;
	int width = tex_width, height = tex_height;
	int xm = mask_width/2, ym = mask_height/2;

	int ff;
	GLfloat val;
	GLfloat* color = (GLfloat*)GlobalAlloc(GPTR, sizeof(GLfloat) * color_num);

	for(loop = 0; loop<color_num; loop++){
		val = 0.0f;
		ff  = 0;
		for(x=-xm; x<=xm; x++)
		{
			for(y=-ym; y<=ym; y++)
			{
				int px, py;
				px = i+x;
				if(px < 0){
					px = 0;
				}else if(px >= width){
					px = width-1;
				}

				py = j+y;
				if(py < 0){
					py = 0;
				}else if(py >= height){
					py = height-1;
				}

				color[loop] = original[posGL(px, py)+loop];
				val += color[loop]*mask[ff];
				ff++;
			}
		}
		if(mul!=0.0f)	val *= mul;
		if(add!=0.0f)	val += add;
		if(val<0.0f){
			if(ifmin<0.0f){
				val=-val;
				if(val<0.0f) val=0.0f;
			}else{
				val=ifmin;
			}
		}else if(val>255.0f){
			val=ifmax;
		}
		dest[posGL(i, j)+loop] = (UCHAR)val;
	}
	GlobalFree(color);
}

void KTexture::MakeItMono(GLfloat* pDestGL, GLfloat* pPixelGL){
	int height = tex_height;
	int width  = tex_width;
	for(int j=0; j<height; j++)
		for(int i=0; i<width; i++)
			pDestGL[posGLInternal(i, j)+0] = (UCHAR)(pPixelGL[posGLInternal(i,j)+2]*0.3 + pPixelGL[posGLInternal(i,j)+1]*0.59 + pPixelGL[posGLInternal(i,j)+0]*0.11);
}

void KTexture::Emboss()
{
	int width = tex_width, height = tex_height;
	//UCHAR* tmp = (UCHAR*)GlobalAlloc(GPTR, sizeof(UCHAR)*width*height*32); //32ビットカラーで作成
	//UCHAR* pNew = (UCHAR*)GlobalAlloc(GPTR, sizeof(UCHAR)*width*height*32); //32ビットカラーで作成

	int i,j;
	MakeItMono(m_pTmp1, tex_pixel);

	int strength = (int)(istack[0]*tex_width/DEFAULT_TEX_SIZE);

	float mask[9] = {
		0.0f, (float)(-strength), 0.0f,
		(float)(-strength),0.0f, (float)strength,
		0.0f, (float)strength, 0.0f
	};

	for(j=0; j<height; j++)
		for(i=0; i<width; i++)
			MaskImage(i, j, m_pTmp1, m_pTmp2, 1, mask, 3, 3, 0.25, 128, 0, 255);

	for(j=0; j<height; j++)    //処理後の画素を戻す
		for(i=0; i<width; i++)
			PutPixel(pSrcBuffer, i, j,m_pTmp2[posGL(i,j)+0], m_pTmp2[posGL(i,j)+0], m_pTmp2[posGL(i,j)+0], m_pTmp2[posGL(i,j)+0]);

	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);

	//GlobalFree(tmp);
	//GlobalFree(pNew);
}

void KTexture::Edge(){
	int width = tex_width, height = tex_height;
	//UCHAR* tmp = (UCHAR*)GlobalAlloc(GPTR, sizeof(UCHAR)*width*height*32); //32ビットカラーで作成
	//UCHAR* pNew = (UCHAR*)GlobalAlloc(GPTR, sizeof(UCHAR)*width*height*32); //32ビットカラーで作成

	int i,j;
	MakeItMono(m_pTmp1, tex_pixel);
	
	int strength = (int)(istack[0]*tex_width/DEFAULT_TEX_SIZE);

	float mask[9] = {
		0.0f, (float)(-strength), 0.0f,
		(float)(-strength),0.0f, (float)strength,
		0.0f, (float)strength, 0.0f
	};

	for(j=0; j<height; j++)
		for(i=0; i<width; i++)
			MaskImage(i, j, m_pTmp1, m_pTmp2, 1, mask, 3, 3, 0, 0, -1, 255);

    //処理後の画素を戻す
	for(j=0; j<height; j++)
		for(i=0; i<width; i++)
			PutPixel(pSrcBuffer, i, j,m_pTmp2[posGL(i,j)+0], m_pTmp2[posGL(i,j)+0], m_pTmp2[posGL(i,j)+0],  m_pTmp2[posGL(i,j)+0]);

	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel,width, height);

	//GlobalFree(tmp);
	//GlobalFree(pNew);
}

long KTexture::GetGLBufferSize(){
	return (sizeof(GLfloat) * tex_width * tex_height * (bppGL/8));
}

void KTexture::Sharp(){
	int width = tex_width, height = tex_height;

	int i,j;
	DWORD dwSizeGL  = GetGLBufferSize();
	CopyMemory(m_pTmp1, tex_pixel, dwSizeGL);
	
	int strength = istack[0];

	float mask[9] = {
		0.0f, (float)(-strength), 0.0f,
		(float)(-strength), (float)(strength*(istack[1])), (float)(-strength),
		0.0f, (float)(-strength), 0.0f
	};

	for(j=0; j<height; j++)
		for(i=0; i<width; i++)
			MaskImage(i, j, m_pTmp1, m_pTmp2, 3, mask, 3, 3, 0, 0, 0, 255);


	PutPixelInUsual(pSrcBuffer, m_pTmp2);
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);
}

void KTexture::Smooth(){
	int width = tex_width, height = tex_height;
	int i,j;
	DWORD dwSizeGL  = GetGLBufferSize();
	CopyMemory(m_pTmp1, tex_pixel, dwSizeGL);
	
	float mask[9] = {
		(float)istack[0], (float)istack[0], (float)istack[0],
		(float)istack[0], (float)istack[1], (float)istack[0],
		(float)istack[0], (float)istack[0], (float)istack[0]
	};

	for(j=0; j<height; j++)
		for(i=0; i<width; i++)
			MaskImage(i, j, m_pTmp1, m_pTmp2, 3, mask, 3, 3, 1.0f/((istack[0]*8.0f)+(istack[1])), 0, 0, 255);

	//MaskImage(t, m_pTmp1, m_pTmp2, 3, mask, 3, 3, 1.0f/(8.0f+4.0f), 0, 0, 255);
	//MaskImage(t, m_pTmp1, m_pTmp2, 3, mask, 3, 3, 1.0f/(float)((strength*8)+(strength*4)), 0, 0, 255);

	PutPixelInUsual(pSrcBuffer, m_pTmp2);
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel,width, height);
}

void KTexture::Check(){
	int i, j;
	int width = tex_width, height = tex_height;
	int dx, dy; //タイル分割数
	int kx, ky; //該当ピクセルの色を決定するための一時変数
	
	if(istack[0] == 0 || istack[1] == 0) return;
	dx = width / istack[0]; 
	dy = height / istack[1];
	for(i=0; i<width; i++){
		kx = (int)(i/(float)dx);
		if(kx%2!=0) kx = -1;
		else		kx =  1;

		for(j=0; j<height; j++){
			ky = (int)(j/(float)dy);
			if(ky%2!=0) ky = -1;
			else		ky =  1;

			if(kx*ky == 1){ //両方とも同じなら
				PutPixel(pSrcBuffer, i, j, (GLfloat)istack[2], (GLfloat)istack[3], (GLfloat)istack[4],  (GLfloat)istack[5]);
			}else{
				PutPixel(pSrcBuffer, i, j, (GLfloat)istack[6], (GLfloat)istack[7], (GLfloat)istack[8],  (GLfloat)istack[9]);
			}
		}
	}
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);
}

void KTexture::Srand(){
	ksrand(istack[0]);
	gen_pnoize();
}

void KTexture::DrawRoundRect(){
	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;
	UCHAR* pTempPixels;
	HBRUSH hOldBrush, hBrush;
	HPEN hOldPen, hPen;
	ProcessDIBCreation(&hDC, &hBitmap, &hOldBitmap, &pTempPixels);
	ProcessPenAndBrushCreation(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	
	DrawGDIBegin(hDC, pTempPixels);
		RoundRect(hDC, (int)(istack[0]/DEFAULT_TEX_SIZE*tex_width+1), (int)(istack[1]/DEFAULT_TEX_SIZE*tex_height+1), (int)(istack[2]/DEFAULT_TEX_SIZE*tex_width+istack[0]/DEFAULT_TEX_SIZE*tex_width), (int)(istack[3]/DEFAULT_TEX_SIZE*tex_height+istack[1]/DEFAULT_TEX_SIZE*tex_height), (int)(istack[4]/DEFAULT_TEX_SIZE*tex_width), (int)(istack[5]/DEFAULT_TEX_SIZE*tex_height));
	DrawGDIEnd(pTempPixels);

	ProcessPenAndBrushDestruction(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	ProcessDIBDestruction(hDC, hBitmap, hOldBitmap);
}

void KTexture::Font(){
	lstrcpy(m_szFontName, sstack[0]);
}

void KTexture::DrawPolygon(){
	HRGN hrgn;
	POINT* pt = (POINT*)GlobalAlloc(GPTR, sizeof(POINT)*istack[0]);
	for(int i=0, j=0; i<istack[0]; i++, j+=2){
		pt[i].x = (int)(istack[j+1]/DEFAULT_TEX_SIZE*tex_width+1);
		pt[i].y = (int)(istack[j+2]/DEFAULT_TEX_SIZE*tex_height+1);
	}

	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;
	UCHAR* pTempPixels;
	HBRUSH hOldBrush, hBrush;
	HPEN hOldPen, hPen;
	ProcessDIBCreation(&hDC, &hBitmap, &hOldBitmap, &pTempPixels);
	ProcessPenAndBrushCreation(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	
	DrawGDIBegin(hDC, pTempPixels);
		hrgn = CreatePolygonRgn(pt , istack[0] , WINDING);
		PaintRgn(hDC , hrgn);
	DrawGDIEnd(pTempPixels);

	ProcessPenAndBrushDestruction(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	ProcessDIBDestruction(hDC, hBitmap, hOldBitmap);
	DeleteObject(hrgn);
	GlobalFree(pt);
}

void KTexture::DrawPolyline(){
	POINT* pt = (POINT*)GlobalAlloc(GPTR, sizeof(POINT)*istack[0]);
	for(int i=0, j=0; i<istack[0]; i++, j+=2){
		pt[i].x = (int)(istack[j+1]/DEFAULT_TEX_SIZE*tex_width+1);
		pt[i].y = (int)(istack[j+2]/DEFAULT_TEX_SIZE*tex_height+1);
	}

	HDC hDC;
	HBITMAP hBitmap, hOldBitmap;
	UCHAR* pTempPixels;
	HBRUSH hOldBrush, hBrush;
	HPEN hOldPen, hPen;
	ProcessDIBCreation(&hDC, &hBitmap, &hOldBitmap, &pTempPixels);
	ProcessPenAndBrushCreation(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	
	DrawGDIBegin(hDC, pTempPixels);
		Polyline(hDC, pt, istack[0]);
	DrawGDIEnd(pTempPixels);

	ProcessPenAndBrushDestruction(hDC, &hBrush, &hOldBrush, &hPen, &hOldPen);
	ProcessDIBDestruction(hDC, hBitmap, hOldBitmap);
	GlobalFree(pt);
}

void KTexture::RgbToA(){
	int i,j;
	int width = tex_width;
	int height = tex_height;
	GLfloat dat;

	DWORD dwSizeGL = GetGLBufferSize();
	CopyMemory(pSrcBuffer, tex_pixel, dwSizeGL);
	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			if(m_render_channel&(KT_R|KT_G|KT_B)){
				dat = (UCHAR)(((tex_pixel)[posGL(i,j)+0] + (tex_pixel)[posGL(i,j)+1] + (tex_pixel)[posGL(i,j)+2])/3);
			}else if(m_render_channel&KT_B){
				dat = tex_pixel[posGL(i,j)+2];
			}else if(m_render_channel&KT_G){
				dat = tex_pixel[posGL(i,j)+1];
			}else if(m_render_channel&KT_R){
				dat = tex_pixel[posGL(i,j)+0];
			}
			pSrcBuffer[posGL(i,j)+3] = dat;
			//PutPixel(pSrcBuffer, i, j, 0, 0, 0, dat);
		}
	}
	CopyMemory(tex_pixel, pSrcBuffer, dwSizeGL);
	CopyMemory(pMainBuffer, tex_pixel, dwSizeGL);
		//ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, tex_width, tex_height);
}

void KTexture::Gradient(){

}

void KTexture::Mono(){
	MakeItMono(pSrcBuffer, tex_pixel);
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, tex_width, tex_height);
}

void KTexture::DrawGDIBegin(HDC hDC, UCHAR* pTempPixels)
{
	int i,j,d;
	int width = tex_width, height = tex_height;

	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			d = i*(bppDIB/8) + ((height-1-j)*width*(bppDIB/8));
			//pTempPixels[d+2] = (max_r != 0) ? max_r-1 : 1;
			//pTempPixels[d+1] = (max_g != 0) ? max_g-1 : 1;
			//pTempPixels[d+0] = (max_b != 0) ? max_b-1 : 1;
			pTempPixels[d+2] = 255 - max_r;
			pTempPixels[d+1] = 255 - max_g;
			pTempPixels[d+0] = 255 - max_b;
			//pTempPixels[d+2] = (pMainBuffer[posGLInternal(i,j)+0] != 0) ? 255 - pMainBuffer[posGLInternal(i,j)+0] : 1;
			//pTempPixels[d+1] = (pMainBuffer[posGLInternal(i,j)+1] != 0) ? 255 - pMainBuffer[posGLInternal(i,j)+1] : 1;
			//pTempPixels[d+0] = (pMainBuffer[posGLInternal(i,j)+2] != 0) ? 255 - pMainBuffer[posGLInternal(i,j)+2] : 1;
		}
	}
}

void KTexture::DrawGDIEnd(UCHAR* pTempPixels)
{
	int i,j,d;
	int width = tex_width, height = tex_height;

	GLfloat r,g,b,a;
	for(j=0; j<height; j++)
	{
		for(i=0; i<width; i++)
		{
			d = i*(bppDIB/8) + ((height-1-j)*width*(bppDIB/8));
			r = (255-max_r != pTempPixels[d+2]) ? pTempPixels[d+2] : -1.0f;
			g = (255-max_g != pTempPixels[d+1]) ? pTempPixels[d+1] : -1.0f;
			b = (255-max_b != pTempPixels[d+0]) ? pTempPixels[d+0] : -1.0f;
			a = max_a;
			if( r == -1.0f
				||
				g == -1.0f
				||
				b == -1.0f
				){
				a = -1.0f;
			}

			PutPixel(pSrcBuffer, i, j, r, g, b, a);
		}
	}
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);

}

void KTexture::Move(){
	int i,j,x,y;
	int width = tex_width, height = tex_height;
	
	DWORD dwSizeGLBuffer  = GetGLBufferSize();
	CopyMemory(m_pTmp1, tex_pixel, dwSizeGLBuffer);
	FillMemory(pSrcBuffer, dwSizeGLBuffer, 0);
		for(j=0; j<height; j++)
		{
			for(i=0; i<width; i++)
			{
				x = i+(int)(istack[0]/DEFAULT_TEX_SIZE*width);
				y = j+(int)(istack[1]/DEFAULT_TEX_SIZE*height);

				if((x<width && x>=0) && (y<height && y>=0)){ //正常に描画できる範囲内
					PutPixel(pSrcBuffer, x, y, 
						m_pTmp1[posGL(i,j)+0],
						m_pTmp1[posGL(i,j)+1], 
						m_pTmp1[posGL(i,j)+2], 
						m_pTmp1[posGL(i,j)+3]);
				}else{	//普通に描画することができない範囲. 必要に応じて, ラップアラウンドする.
					if(istack[2]>0){
						//マイナスの値が入力されている
						if(x<0) x = width +x;
						if(y<0) y = height+y;

						//範囲外に出た場合
						if(x>=width)  x = x-width;
						if(y>=height) y = y-height;
						PutPixel(pSrcBuffer, x, y,
							m_pTmp1[posGL(i,j)+0],
							m_pTmp1[posGL(i,j)+1], 
							m_pTmp1[posGL(i,j)+2], 
							m_pTmp1[posGL(i,j)+3]);
					}//else continue;
				}
			}
		}
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);
}

void KTexture::Blur(){
	int width = tex_width, height = tex_height;
	GLfloat** tmp=&m_pTmp1;
	GLfloat** pNew=&m_pTmp2;
	int i,j;

	DWORD dwSizeGL  = GetGLBufferSize();
	CopyMemory(*pNew, tex_pixel, dwSizeGL);
	
	float xmask[9] = {
		//1, 2, 1,
		//2, 4, 2,
		//1, 2, 1
		0.0f, 3.0f, 0.0f,
		0.0f, 3.0f, 0.0f,
		0.0f, 3.0f, 0.0f
	};

	float ymask[9] = {
		//1, 2, 1,
		//2, 4, 2,
		//1, 2, 1
		0.0f, 0.0f, 0.0f,
		3.0f, 3.0f, 3.0f,
		0.0f, 0.0f, 0.0f
	};

	int x;
	for(x=0; x<istack[0]; x++){
		GLfloat** swp=tmp;
		tmp = pNew;
		pNew = swp;
		
		for(j=0; j<height; j++)
			for(i=0; i<width; i++)	
				MaskImage(i, j, (*tmp), (*pNew), 3, xmask, 3, 3, 1.0f/9.0f, 0, 0, 255);
	}

	int y;
	for(y=0; y<istack[1]; y++){
		GLfloat** swp=tmp;
		tmp = pNew;
		pNew = swp;
		
		for(j=0; j<height; j++)
			for(i=0; i<width; i++)	
				MaskImage(i, j, (*tmp), (*pNew), 3, ymask, 3, 3, 1.0f/9.0f, 0, 0, 255);
	}

	PutPixelInUsual(pSrcBuffer, *pNew);

	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);
}

void KTexture::DisableAntiAliase(){
	isAlreadyHasAA = TRUE;
}

void KTexture::AntiAliase(){
	int width = tex_width, height = tex_height;
	//int i,j;

	//DWORD dwSizeGL  = GetGLBufferSize();
	//CopyMemory(m_pTmp1, tex_pixel, dwSizeGL);
	//
	////const long pj[jit_n][2] = {{0,0},{1,0},{0,1},{1,1}};
	////const float jitter[jit_n][2] = {{0.375f,0.25f},{0.875f,0.25f},{0.125f,0.75f},{0.625f,0.75f}};

	//float xmask[8] = {
	//	0.375f, 0.25f,
	//	0.875f, 0.25f,
	//	0.125f, 0.75f,
	//	0.625f, 0.75f
	//};

	//for(j=0; j<height; j++)
	//	for(i=0; i<width; i++)
	//		MaskImage(i, j, m_pTmp1, m_pTmp2, 3, xmask, 2, 4, , 0, 0, 255);

	//PutPixelInUsual(pSrcBuffer, m_pTmp2);
	//ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);

//	GLfloat* pNew = this->m_pTmp1;
	ZeroMemory(m_pTmp1, GetGLBufferSize());
	ZeroMemory(m_pTmp2, GetGLBufferSize());
	CopyMemory(m_pTmp1, tex_pixel, GetGLBufferSize());
	this->AntiAliaseRGBA(m_pTmp1, m_pTmp2, width, height);
	PutPixelInUsual(pSrcBuffer, m_pTmp2);
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);
}

void KTexture::Normal(){
	int i,j;
	int width = tex_width, height = tex_height;

	DWORD dwSizeGL  = GetGLBufferSize();
	Mono();
	//Mono(t);	//画素をグレースケールにする
	CopyMemory(m_pTmp1, tex_pixel, dwSizeGL);

    for(j=1; j<height-1; j++)
	{
		for(i=1; i<width-1; i++)
		{
			float val[4];
			val[0] = (float)(
				tex_pixel[posGL(i,j+1)+0] +
				tex_pixel[posGL(i,j+1)+1] + 
				tex_pixel[posGL(i,j+1)+2]
				);

			val[1] = (float)(
				tex_pixel[posGL(i-1,j)+0] + 
				tex_pixel[posGL(i-1,j)+1] + 
				tex_pixel[posGL(i-1,j)+2]
				);

			val[2] = (float)(
				tex_pixel[posGL(i+1,j)+0] + 
				tex_pixel[posGL(i+1,j)+1] + 
				tex_pixel[posGL(i+1,j)+2]
				);

			val[3] = (float)(
				tex_pixel[posGL(i,j-1)+0] + 
				tex_pixel[posGL(i,j-1)+1] + 
				tex_pixel[posGL(i,j-1)+2]
				);

			CVector vec(-val[2]+val[1], val[0]-val[3], 2);
			vec.normalize();
			vec*=0.5f;
			vec.x += 0.5f; vec.y += 0.5f; vec.z += 0.5f;
			//if(vec.x<0) vec.x=0; if(vec.x>1.0f) vec.x=1.0f;
			//if(vec.y<0) vec.y=0; if(vec.y>1.0f) vec.y=1.0f;
			//if(vec.z<0) vec.z=0; if(vec.z>1.0f) vec.z=1.0f;
			m_pTmp1[posGL(i,j)+2] = (UCHAR)((vec.z)*255);
			m_pTmp1[posGL(i,j)+1] = (UCHAR)((vec.y)*255);
			m_pTmp1[posGL(i,j)+0] = (UCHAR)((vec.x)*255);
		}
	}

	PutPixelInUsual(pSrcBuffer, m_pTmp1);
	ChangePixelData(pMainBuffer, pSrcBuffer, tex_pixel, width, height);
}

void KTexture::PutPixelInUsual(GLfloat* pDest, GLfloat* pSrc){
	CopyMemory(pDest, pSrc, GetGLBufferSize());
	//int width = tex_width, height = tex_height;
	//for(int j=0; j<height; j++)//処理後の画素を戻す
	//	for(int i=0; i<width; i++)
	//		PutPixel(pDest, i, j, pSrc[posGL(i,j)+0], pSrc[posGL(i,j)+1], pSrc[posGL(i,j)+2], pSrc[posGL(i,j)+3]);
}

void KTexture::AntiAliaseRGBA(float* rgba_tex, float* blrgba_tex, unsigned long texsize_w,unsigned long texsize_h)
{
	//zero clear?
	/*long tp=0;
	while(tp<(texsize_w*texsize_h)){
		blrgba_tex[tp*4  ] = 0;
		blrgba_tex[tp*4+1] = 0;
		blrgba_tex[tp*4+2] = 0;
		blrgba_tex[tp*4+3] = 0;
		tp++;
	}*/
	const unsigned long jit_n = 4;
	const long pj[jit_n][2] = {{0,0},{1,0},{0,1},{1,1}};
	const float jitter[jit_n][2] = {{0.375f,0.25f},{0.875f,0.25f},{0.125f,0.75f},{0.625f,0.75f}};
	unsigned long x,y,j;
	
	//data = px * py * (a - b - c + d) + px * (b - a) + py * (c - a) + a
	for(y=1; y<texsize_h-1; y++){
		for(x=1; x<texsize_w-1; x++){
			unsigned long cl;
			for(cl=0; cl<4; cl++){
				float a=0,b=0,c=0,d=0;
				unsigned long r;
				for(r=0;r<4;r++){
					a += (float)((int)rgba_tex[(x+pj[0][0]-(r&1)+(y+pj[0][1]-((r>>1)&2))*texsize_w)*4+cl]*0.25f);
					b += (float)((int)rgba_tex[(x+pj[1][0]-(r&1)+(y+pj[1][1]-((r>>1)&2))*texsize_w)*4+cl]*0.25f);
					c += (float)((int)rgba_tex[(x+pj[2][0]-(r&1)+(y+pj[2][1]-((r>>1)&2))*texsize_w)*4+cl]*0.25f);
					d += (float)((int)rgba_tex[(x+pj[3][0]-(r&1)+(y+pj[3][1]-((r>>1)&2))*texsize_w)*4+cl]*0.25f);
				}
				for(j=0; j<jit_n; j++){
					//blrgba_tex[(x+y*texsize_w)*4+cl] = (unsigned char)((jitter[j][0]*jitter[j][1]*(a-b-c+d)+jitter[j][0]*(b-a)+jitter[j][1]*(c-a)+a));
					blrgba_tex[(x+y*texsize_w)*4+cl] += (unsigned char)((jitter[j][0]*jitter[j][1]*(a-b-c+d)+jitter[j][0]*(b-a)+jitter[j][1]*(c-a)+a)*0.25f);
				}
			}
		}
	}
}