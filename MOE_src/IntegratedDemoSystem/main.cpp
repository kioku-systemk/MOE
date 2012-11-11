/*
	4KB/64KB OpenGL Intro Template
	coded by kioku@Cyber K
*/
#include "stdafx.h"

PFNGLISRENDERBUFFEREXTPROC		glIsRenderbufferEXT;
PFNGLBINDRENDERBUFFEREXTPROC		glBindRenderbufferEXT;
PFNGLDELETERENDERBUFFERSEXTPROC	glDeleteRenderbuffersEXT;
PFNGLGENRENDERBUFFERSEXTPROC		glGenRenderbuffersEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC	glRenderbufferStorageEXT;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT;
PFNGLISFRAMEBUFFEREXTPROC		glIsFramebufferEXT;
PFNGLBINDFRAMEBUFFEREXTPROC		glBindFramebufferEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC	glDeleteFramebuffersEXT;
PFNGLGENFRAMEBUFFERSEXTPROC		glGenFramebuffersEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
PFNGLGENERATEMIPMAPEXTPROC		glGenerateMipmapEXT;

PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1fARB;
PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
PFNGLMULTITEXCOORD3FARBPROC		glMultiTexCoord3fARB;
PFNGLMULTITEXCOORD4FARBPROC		glMultiTexCoord4fARB;
PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
PFNWGLCHOOSEPIXELFORMATARBPROC	wglChoosePixelFormatARB;


#include "../klib/kbWindow.h"
#include "../klib/kfpu.h"
#include "../klib/kmodel.h"
#include "../klib/kdemo.h"
#include "../klib/glScreen.h"
#include "../kLib/kTexture.h"
#include "resource.h"

//#pragma comment(linker, "/entry:CodeStart")

//--------------------------------------------------------------------------------------------------
/* define complile mode... */
#define MODE_PLAYER			 1//compile as a stand alone player
#define MODE_DEMO			 0//compile as a release version demo
#define MODE_RESOURCE_LOADER 0//compile as a self-exportable demo

#if	!MODE_DEMO && !MODE_PLAYER && !MODE_RESOURCE_LOADER
	#error "YOU MUST #define MODE_DEMO, MODE_PLAYER or MODE_RESOURCE_LOADER!"
#elif MODE_DEMO 
	#if MODE_PLAYER || MODE_RESOURCE_LOADER
		#error "YOU CAN ENABLE ONLY ONE OF MODE_XXXs!"
	#endif
#elif MODE_PLAYER 
	#if MODE_DEMO || MODE_RESOURCE_LOADER
		#error "YOU CAN ENABLE ONLY ONE OF MODE_XXXs!"
	#endif
#elif MODE_RESOURCE_LOADER
	#if MODE_DEMO || MODE_PLAYER
		#error "YOU CAN ENABLE ONLY ONE OF MODE_XXXs!"
	#endif
#endif

//--------------------------------------------------------------------------------------------------
/* MODE_DEMOの場合は、#includeとDEMO_PTR,LOADER_PTRを適宜書き換えること。 */
/* MODE_PLAYERの場合は、#includeとLOADER_PTRを適宜書き換えること。 */
#if		MODE_PLAYER
	#include "../CLibrary/Util.h"
	#define SZ_PROD_TITLE "KDB Stand Alone Player"
	#define SZ_COPYRIGHT_INFO "System K Works 2006"
	char* g_szFileName = "demo.kdb";
	#include "tera_loader.h"
	#define LOADER_PTR tera_loader
#elif	MODE_DEMO
	#define SZ_PROD_TITLE "TERA - coding your life -"
	#define SZ_COPYRIGHT_INFO "System K Works 2006"
	#include "demo.h"
	#include "tera_loader.h"
	#define DEMO_PTR demo
	#define LOADER_PTR tera_loader
#elif	MODE_RESOURCE_LOADER
	#define SZ_DEMO_TITLE_BASE "%s- System K Works %d"
	//#define SZ_DEMO_TITLE Demoname
#endif

//--------------------------------------------------------------------------------------------------
/* define debug_purpose const. */
#define SPEED_TEST			 0//write loading time -> speed_test.log 

//--------------------------------------------------------------------------------------------------
#define nMTRequired			 4//required number of multi-texture layer.
#define nMultiSample		 4//FSAAxnMultiSample
#define MINIMUM_SCREEN_WIDTH		640
#define MINIMUM_SCREEN_HEIGHT		480
#define MINIMUM_SCREEN_BPP			16
#define MINIMUM_SCREEN_FREQUENCY	60
#define PREF_SCREEN_WIDTH			1024
#define PREF_SCREEN_HEIGHT			768
#define PREF_SCREEN_FREQUENCY		60
#define PREF_SCREEN_BPP				32
//const float fIgnoreAspectRatios[] = { 1.7777777777 };

//--------------------------------------------------------------------------------------------------
#if	MODE_RESOURCE_LOADER
#include "../kLib/kResourceDemoDat.h"
	KResourceDemoDat resDemo;

	//typedef struct _tagKResource{
	//	HRSRC* hrSrc;
	//	HGLOBAL* hRes;
	//	DWORD* dwSize;
	//	char* szResourceType;
	//	INT szResourceName;
	//}KRESOURCE;
#endif

int SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SCREEN_FREQUENCY;
KDemo Demo;
KDemo Loader;
//BOOL isShowFPS;
bool highMode = false;

//--------------------------------------------------------------------------------------------------
#if	SPEED_TEST
	HANDLE stfp;
	DWORD dwStartLogTime;
	inline void SpeedTestBegin(){
		if((stfp=CreateFile("speed_test.log", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL))==INVALID_HANDLE_VALUE){
			FatalAppExit(0, "Couldn't open \"demo.kdb\".");
		}

		DWORD dwRead = 0;
		char str[64];
		dwStartLogTime = timeGetTime();
		wsprintf(str, "start logging from %d\r\n", dwStartLogTime);
		WriteFile(stfp, str, lstrlen(str), &dwRead, NULL);
	}

	inline void SpeedTestEnd(){
		DWORD dwRead = 0;
		char str[64];
		wsprintf(str, "loading phase took %dmsec.\r\n", timeGetTime() - dwStartLogTime);
		WriteFile(stfp, str, lstrlen(str), &dwRead, NULL);

		CloseHandle(stfp);
	}
#endif

//--------------------------------------------------------------------------------------------------
#include <shlwapi.h>
int CheckForExtension(const char* szSrchExt){//指定された文字列が示す拡張がサポートされているか調べる
	const char* extensions = (char*)glGetString(GL_EXTENSIONS);
	int isExtensionFound = 0;
	if(StrStrI(extensions, szSrchExt) != NULL){
		isExtensionFound = 1;
	}


	PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");
	if(wglGetExtString){
		extensions = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());
		if(StrStrI(extensions, szSrchExt) != NULL){
			isExtensionFound = 1;
		}
	}
	return isExtensionFound;
}

BOOL IsCmdline(const char* szCheckFor){
	int nArgc;
	WCHAR** wszCmdLine = CommandLineToArgvW(GetCommandLineW(), &nArgc);
	int length;
	char** szArgv = (char**)GlobalAlloc(GPTR, sizeof(char*) * nArgc);
	for(int i=0; i<nArgc; ++i){
		if((length = ::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, NULL, 0, NULL, NULL))==0) continue;
		szArgv[i] = (char*)GlobalAlloc(GPTR, sizeof(char) * (length+1));
		::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, szArgv[i], length, NULL, NULL);
	}

	BOOL isFound = FALSE;
	for(int i=0; i<nArgc; ++i){
		if(StrStrI(szArgv[i], szCheckFor) != NULL){
			isFound = TRUE;
		}
	}
	
	//for(i=0; i<nArgc; i++){
	//	GlobalFree(szArgv[i]);
	//}
	//GlobalFree(szArgv);
	return isFound;
}

void __fastcall InitGLState()
{
	glLightf(GL_LIGHT0,GL_DIFFUSE,0.6f);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glAlphaFunc(GL_GREATER,0.01f);
	glEnable(GL_ALPHA_TEST);

	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_MULTISAMPLE_ARB);

	glEnable(GL_DEPTH_TEST);
}

bool EnableVsync(int isVsync)
{
	typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
	PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT  = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );
	if(!wglSwapIntervalEXT){ return false; }
	wglSwapIntervalEXT(isVsync);
	return true;
}

//
//-----------------------------------------------------------------------------------------
//inline void DrawSquare(UINT tex_num, float scale){
//	glBindTexture(GL_TEXTURE_2D, tex_num);
//	glBegin(GL_POLYGON);
//		glTexCoord2f(0.0f, 1.0f);	glVertex2f(-scale, -scale);
//		glTexCoord2f(1.0f, 1.0f);	glVertex2f(scale, -scale);
//		glTexCoord2f(1.0f, 0.0f);	glVertex2f(scale, scale);
//		glTexCoord2f(0.0f, 0.0f);	glVertex2f(-scale, scale);
//	glEnd();
//}

//UINT font[128];
//char fontstr[128];
//inline void __fastcall InitString()
//{
//	KTexture fonts;
//	for(int i=0; i<128; i++){
//		char strt[2] = {i, '\0'};
//		if(lstrlen(strt)==0) continue;
//		char* first = "?,64,64;],\"Lucida Console\";S,15;F,0,0,0,0;E,50,50,50,255;D,22,6,256,1,0,0,\"";
//		char* middle = "\";E,80,120,130,255;D,14,0,256,1,0,0,\"";
//		if((char)i=='"' || (char)i=='\\')
//			wsprintf(fontstr, "%s\\%c%s\\%c\";", first, i, middle, i);
//		else
//			wsprintf(fontstr, "%s%c%s%c\";", first, i, middle, i);
//		fonts.GenerateTextureIndirect(&font[i], fontstr);
//	}
//	//fonts.ResetHistory();
//}

inline void SetupOrthoView(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

inline void EnableOrthoState(){
	//glDisable(GL_CULL_FACE);
	//glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
}

inline void DisableOrthoState(){
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	//glDisable(GL_ALPHA_TEST);
	//glEnable(GL_CULL_FACE);
}

//inline void __fastcall DrawString(const char* str)
//{
//	SetupOrthoView();
//	EnableOrthoState();
//
//	const float fFontSize = 0.075f; 
//	const float fXStart = -1.0f;
//	const float fYStart = 0.0f;
//	int i;
//	int n = lstrlen(str);
//	glEnable(GL_TEXTURE_2D);
//	for(i=0; i<n; i++){
//		glBindTexture(GL_TEXTURE_2D, font[str[i]]);
//		glBegin(GL_QUADS);
//			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//			glTexCoord2f(0.0f, 0.0f);	glVertex2f(fXStart,				 fYStart);
//			glTexCoord2f(1.0f, 0.0f);	glVertex2f(fXStart + fFontSize,  fYStart);
//			glTexCoord2f(1.0f, 1.0f);	glVertex2f(fXStart + fFontSize , fYStart - fFontSize);
//			glTexCoord2f(0.0f, 1.0f);	glVertex2f(fXStart,				 fYStart - fFontSize);
//		glEnd();
//		glTranslatef(fFontSize-0.025f, 0.0f, 0.0f);
//	}
//	glDisable(GL_TEXTURE_2D);
//
//	DisableOrthoState();
//}

//inline void DrawFPS(){
//	static DWORD oldTime = timeGetTime();
//	DWORD currentTime = timeGetTime();
//	static DWORD fps = 0;
//	static char str[16] = {'\0'};
//
//	DWORD elapsed = currentTime - oldTime;
//	if(elapsed>=1000){//one second has been passed.
//		wsprintf(str, "%dfps", fps);
//		oldTime = currentTime;
//		fps = 0;
//	}
//	fps++;
//	glPushMatrix();
//		glTranslatef(0.0f, -0.85f, 0.0f);
//		DrawString(str);
//	glPopMatrix();
//}

inline void DrawFrame(int width, int height){
	//const float invTargetAspect = 1.0f / (16.0f / 9.0f);
	//float framesize = (1.0f - (float)width / (float)height * invTargetAspect) * 0.5f;
	const float framesize = 0.125f;//for 4:3

	SetupOrthoView();
	EnableOrthoState();
	//glDisable(GL_CULL_FACE);
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

		//const float frame_h = 1.0f - 0.125f*2.0f;
		//glBegin(GL_QUADS);
		//	glVertex2f(-1.0f, 1.0f);
		//	glVertex2f( 1.0f, 1.0f);
		//	glVertex2f( 1.0f, frame_h);
		//	glVertex2f(-1.0f, frame_h);
		////glEnd();

		////glBegin(GL_QUADS);
		//	glVertex2f(-1.0f, -frame_h);
		//	glVertex2f( 1.0f, -frame_h);
		//	glVertex2f( 1.0f, -1.0f);
		//	glVertex2f(-1.0f, -1.0f);
		//glEnd();
		glBegin(GL_QUADS);
			//const float sight_h = 0.75f;
			glVertex2f(0.0f, 0.0f);
			glVertex2f(0.0f, framesize);
			glVertex2f(1.0f, framesize);
			glVertex2f(1.0f, 0.0f);

			glVertex2f(0.0f, 1.0f - framesize);
			glVertex2f(0.0f, 1.0f);
			glVertex2f(1.0f, 1.0f);
			glVertex2f(1.0f, 1.0f - framesize);
		glEnd();
	//glEnable(GL_CULL_FACE);
	DisableOrthoState();
}

inline void OnLoadRender(float fRatioComplete){//fRatioComplete 0.0 to 1.0
//check for esckey
	if(GetAsyncKeyState(VK_ESCAPE)){//if((GetAsyncKeyState(VK_ESCAPE)&0x8000)==0x8000){
		ExitProcess(0);
	}

//reder the loader
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	Loader.RenderScene(0, fRatioComplete);
	DrawFrame(SCREEN_WIDTH, SCREEN_HEIGHT);

	kbSwapBuffers();
}
inline void nullLoader(float fRatioComplete){}

inline void __fastcall LoadDemo()
{
	//InitString();

#if		MODE_PLAYER
	Loader.Load(LOADER_PTR, nullLoader, false);
#elif	MODE_DEMO
	Loader.Load(LOADER_PTR, nullLoader);
#elif	MODE_RESOURCE_LOADER
	Loader.Load(resDemo.kdb[KDB_LOADER], nullLoader, false);
#endif

#if		SPEED_TEST
	SpeedTestBegin();
#endif


#if		MODE_PLAYER
	GET_ARGUMENTS();
		for(int i=0; i<argc; i++){
			if( GetFileAttributes(argv[i])==0xFFFFFFFF ){
				//nop(file not found)
			}else{
				if( StrStrI(argv[i], ".kdb") ){
					g_szFileName = argv[i];
					break;
				}
			}
		}
		HANDLE fp;
		if((fp=CreateFile(g_szFileName,GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))==INVALID_HANDLE_VALUE){
			FatalAppExit(0, "Couldn't open .kdb file");
		}
		unsigned long filesize = GetFileSize(fp, NULL);
		unsigned char* mfile = (unsigned char*)GlobalAlloc(GPTR,filesize);
		DWORD dwRead = 0;
		ReadFile(fp, mfile, filesize, &dwRead, NULL);
		CloseHandle(fp);
		Demo.Load(mfile, OnLoadRender, highMode);
	RELEASE_ARGUMENTS();
#elif	MODE_DEMO
	Demo.Load(DEMO_PTR, OnLoadRender, highMode);
#elif	MODE_RESOURCE_LOADER
	Demo.Load(resDemo.kdb[KDB_MAIN], OnLoadRender, highMode);
#endif

#if	SPEED_TEST
	SpeedTestEnd();
#endif
}

inline BOOL RenderScreen(){//isEnd
	BOOL isEnd = Demo.RenderDemo();
	DrawFrame(SCREEN_WIDTH, SCREEN_HEIGHT);
	//if(isShowFPS){
	//	DrawFPS();
	//}
	return isEnd;
}

inline BOOL __fastcall OnIdle()
{
	static BOOL isInit = FALSE;
	if(!isInit){
		LoadDemo();
		Demo.Play();
		isInit = TRUE;
		return FALSE;
	}

	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	BOOL isEnd = RenderScreen();

	kbSwapBuffers();
	return isEnd;
}

BOOL OnExitFade(){
	static DWORD dwStartTime = timeGetTime();

	//フェードアウトコード
	const float fVol = Demo.GetVolume();
	const DWORD dwFadeTime = 3000;//5000;
	BOOL isReadyToExit = FALSE;
	float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	DWORD dwElapsedTime = timeGetTime() - dwStartTime;
	
	float fRatio = 1.0f - (dwElapsedTime/(float)dwFadeTime);//1 to 0

	if(fRatio<0.0f){//fRatio>1.0f){
		isReadyToExit = TRUE;
	}else{
		glClearColor(0.0f,0.0f,0.0f,0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		if( RenderScreen() ){
			isReadyToExit = TRUE;
		}else{//まだDEMOがまわってるなら
			//sqr
			{
				//if(fRatio!=0.0f){
				//	fRatio = sqr(fRatio);
				//}
				//demo.ks->vol = fVol * fRatio;//1 to 0
				//color[3]	 = 1.0f * (1.0f-fRatio);//0 to 1
				//
				////linear
				//color[3]	 = 1.0f * (1.0f-fRatio);//0 to 1
				//if(fRatio!=0.0f){
				//	fRatio = sqr(fRatio);
				//}
				//if(Demo.ks){demo.ks->vol = fVol * fRatio;}//1 to 0
			}
			//Pow2
			{
				float alphaf = fRatio*fRatio;
				color[3]	 = 1.0f * (1.0f-alphaf);//0 to 1
				if(fRatio!=0.0f){
					fRatio = sqr(fRatio);
				}
				Demo.SetVolume(fVol*fRatio);
			}

			glPushMatrix();
				SetupOrthoView();
				EnableOrthoState();
					glColor4fv(color);
					glBegin(GL_QUADS);
						glVertex2f(0.0f, 0.0f);
						glVertex2f(0.0f, 1.0f);
						glVertex2f(1.0f, 1.0f);
						glVertex2f(1.0f, 0.0f);
					glEnd();
				DisableOrthoState();
			glPopMatrix();
		}
		kbSwapBuffers();
	}

	return isReadyToExit;
}

//----------------------ENTRY POINT, SETUP DIALOG, CHECK FOR EXTENSION-----------------------------------------------------------------------
BOOL isMSSupported;
BOOL isFBOSupported;
void InitializeProcedure(){

#if MODE_RESOURCE_LOADER
	//
	//Load Res
	//
	HINSTANCE hInstance = GetModuleHandle(NULL);
	HRSRC   hrSrc     = FindResource(hInstance,   MAKEINTRESOURCE(IDR_SYSTEM_K_DEMODAT), "SYSTEM_K_DEMODAT");
	HGLOBAL hRes      = LoadResource(hInstance,   hrSrc);
	DWORD   dwSizeRes = SizeofResource(hInstance, hrSrc);
	{
        unsigned char* buf = (unsigned char*)GlobalAlloc(GPTR, dwSizeRes);
		int bufptr = 0;
		kmemcpy(buf, (unsigned char*)LockResource(hRes), dwSizeRes);
		
		kmemcpy(&resDemo.nYear,		buf+bufptr, sizeof(int)  );				bufptr += sizeof(int);
		kmemcpy(&resDemo.dwLength,	buf+bufptr, sizeof(DWORD));				bufptr += sizeof(DWORD);

		resDemo.szRc_DemoTitle = (char*)GlobalAlloc(GPTR, resDemo.dwLength);
		kmemcpy(resDemo.szRc_DemoTitle, buf+bufptr, resDemo.dwLength);		bufptr += resDemo.dwLength;

		kmemcpy(&resDemo.dwSize[KDB_LOADER], buf+bufptr, sizeof(DWORD));	bufptr += sizeof(DWORD);
		kmemcpy(&resDemo.dwSize[KDB_MAIN],   buf+bufptr, sizeof(DWORD));	bufptr += sizeof(DWORD);

		resDemo.kdb[KDB_LOADER] = (unsigned char*)GlobalAlloc(GPTR, resDemo.dwSize[KDB_LOADER]);
		resDemo.kdb[KDB_MAIN]   = (unsigned char*)GlobalAlloc(GPTR, resDemo.dwSize[KDB_MAIN]  );
		kmemcpy(resDemo.kdb[KDB_LOADER], buf+bufptr, resDemo.dwSize[KDB_LOADER]);	bufptr += resDemo.dwSize[KDB_LOADER];
		kmemcpy(resDemo.kdb[KDB_MAIN],   buf+bufptr, resDemo.dwSize[KDB_MAIN]);		bufptr += resDemo.dwSize[KDB_MAIN];

	}
	//	UnlockResource(hRes);
	//	FreeResource(hRes);

	////
	////Load Res
	////
	//HINSTANCE hInstance = GetModuleHandle(NULL);
	//HRSRC hrSrcYear, hrSrcDemotitle, hrSrcLoader, hrSrcMain;
	//HGLOBAL hResYear, hResDemotitle, hResLoader, hResMain;
	//DWORD dwSizeYear, dwSizeDemotitle, dwSizeLoader, dwSizeMain;

	//const KRESOURCE pLoadRes[] = {
	//	{&hrSrcYear,		&hResYear,		&dwSizeYear,		"SYSTEM_K_CONFIG_YEAR",		IDR_SYSTEM_K_CONFIG_YEAR},
	//	{&hrSrcDemotitle,	&hResDemotitle, &dwSizeDemotitle,	"SYSTEM_K_CONFIG_DEMOTITLE",IDR_SYSTEM_K_CONFIG_DEMOTITLE},
	//	{&hrSrcLoader,		&hResLoader,	&dwSizeLoader,		"SYSTEM_K_DEMODAT_LOADER",	IDR_SYSTEM_K_DEMODAT_LOADER},
	//	{&hrSrcMain,		&hResMain,		&dwSizeMain,		"SYSTEM_K_DEMODAT_MAIN",	IDR_SYSTEM_K_DEMODAT_MAIN},
	//};
	//for(int i=0; i<sizeof(pLoadRes)/sizeof(pLoadRes[0]); ++i){
	//	*(pLoadRes[i].hrSrc) = FindResource(hInstance, MAKEINTRESOURCE(pLoadRes[i].szResourceName), pLoadRes[i].szResourceType);
	//	*(pLoadRes[i].hRes) = LoadResource(hInstance, *(pLoadRes[i].hrSrc));
	//	*(pLoadRes[i].dwSize) = SizeofResource(hInstance, *(pLoadRes[i].hrSrc));
	//}
	//{
	//	int resPtr = 0;

	//	//YEAR
	//	nYear = *((INT*)LockResource(*(pLoadRes[resPtr].hRes)));
	//	resPtr++;
	//	//DEMOTITLE
	//	szRc_DemoTitle = (char*)GlobalAlloc(GPTR, *(pLoadRes[resPtr].dwSize));
	//	lstrcpy(szRc_DemoTitle, (char*)LockResource(*(pLoadRes[resPtr].hRes)));
	//	resPtr++;

	//	//LOADER
	//	kdbLoader = (unsigned char*)GlobalAlloc(GPTR, *(pLoadRes[resPtr].dwSize));
	//	kmemcpy(kdbLoader, (unsigned char*)LockResource(*(pLoadRes[resPtr].hRes)), *(pLoadRes[resPtr].dwSize));
	//	resPtr++;
	//	//MAIN
	//	kdbMain = (unsigned char*)GlobalAlloc(GPTR, *(pLoadRes[resPtr].dwSize));
	//	kmemcpy(kdbMain, (unsigned char*)LockResource(*(pLoadRes[resPtr].hRes)), *(pLoadRes[resPtr].dwSize));
	//	//char str[512];
	//	//wsprintf(str, "%d, %d", *(pLoadRes[2].dwSize), *(pLoadRes[3].dwSize));
	//	//MessageBox(NULL, str, NULL, MB_SYSTEMMODAL);
	//}
	////for(i=0; i<sizeof(pLoadRes)/sizeof(pLoadRes[0]); i++){
	////	UnlockResource(*(pLoadRes[i].hRes));
	////	FreeResource(*(pLoadRes[i].hRes));
	////}
#endif

	//
	//
	//
	kfInit(); //init fpu

	kbCreateWindow(0, 0, 1, 1, "", WS_POPUP);//dummy window
	if(!IsCmdline("--force")){
		//Check for multi-texture extension support.
		const char* szMultiTextureExt[] = {
		"GL_ARB_multitexture",		//MultiTexture
		"GL_EXT_texture_env_combine",//texture_env_combining
		};

		char* szErrorReason = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
		const char* szCrue = "Buy new video card(ATi-RADEON preferred) or try \"--force\" switch at your own risk.";

		int isMTSupported = 0;
		for(int i=0; i<sizeof(szMultiTextureExt)/sizeof(szMultiTextureExt[0]); ++i){
			isMTSupported |= CheckForExtension(szMultiTextureExt[i]);
		}
		if(!isMTSupported){
			wsprintf(szErrorReason, "Unfortunately, your video card doesn't support %s and %s extensions. %s", szMultiTextureExt[0], szMultiTextureExt[1], szCrue);
			FatalAppExit(0, szErrorReason);
		}

		GLint nSupportedTextureNum;
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &nSupportedTextureNum);
		if(nSupportedTextureNum < nMTRequired){
			wsprintf(szErrorReason, "Unfortunately, we are unable to run this demo due to your video card problem(%d texture units are required). %s", nMTRequired, szCrue);
			FatalAppExit(0, szErrorReason);
		}

		//Check for multi-sampling extension support
		isMSSupported = CheckForExtension("WGL_ARB_multisample");

		//
		isFBOSupported = CheckForExtension("EXT_framebuffer_object");
	}

	//get entry points
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");	
	//glMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC)wglGetProcAddress("glMultiTexCoord1fARB");
	glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
	glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)wglGetProcAddress("glMultiTexCoord3fARB");
	//glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)wglGetProcAddress("glMultiTexCoord4fARB");
	glActiveTextureARB	 = (PFNGLACTIVETEXTUREARBPROC)	wglGetProcAddress("glActiveTextureARB");

	glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)wglGetProcAddress("glIsRenderbufferEXT");
	glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
	glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
	glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
	glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
	glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)wglGetProcAddress("glGetRenderbufferParameterivEXT");
	glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)wglGetProcAddress("glIsFramebufferEXT");
	glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
	glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
	glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
	glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
	glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)wglGetProcAddress("glFramebufferTexture1DEXT");
	glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
	glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)wglGetProcAddress("glFramebufferTexture3DEXT");
	glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
	glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
	glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)wglGetProcAddress("glGenerateMipmapEXT");

	if( !glIsRenderbufferEXT || !glBindRenderbufferEXT || !glDeleteRenderbuffersEXT || 
		!glGenRenderbuffersEXT || !glRenderbufferStorageEXT || !glGetRenderbufferParameterivEXT || 
		!glIsFramebufferEXT || !glBindFramebufferEXT || !glDeleteFramebuffersEXT || 
		!glGenFramebuffersEXT || !glCheckFramebufferStatusEXT || !glFramebufferTexture1DEXT || 
		!glFramebufferTexture2DEXT || !glFramebufferTexture3DEXT || !glFramebufferRenderbufferEXT||  
		!glGetFramebufferAttachmentParameterivEXT || !glGenerateMipmapEXT )
	{
		MessageBox(NULL,"One or more EXT_framebuffer_object functions were not found",
			"ERROR",MB_OK|MB_ICONEXCLAMATION);
	}


	DestroyWindow(kbhWnd);//destroy dummy window
}

//-----------------------------------------------------------------------------------------
#include <shellapi.h>
unsigned int setting_r = 0;
int CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static long select_mode=0;
	switch (message) {
		//case WM_KEYDOWN:
		//{
		//	if((unsigned char)wParam==VK_RETURN){
		//		SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_GO, 0), 0);
		//	}
		//}
		//case WM_INITDIALOG:{
		//}break;
		case WM_COMMAND:
			//lst = GetDlgItem(hWnd,IDC_VIDEOMODE);
			//if((HIWORD(wParam)==LBN_SELCHANGE)&&(lst==(HWND)lParam) ){
			//	select_mode = (long)SendMessage(lst, LB_GETCURSEL, 0, 0);
			//}
			//SendMessage(GetDlgItem(hWnd,IDC_VIDEOMODE) , LB_SETCURSEL, 2, 0);
			
			switch(LOWORD(wParam)) {
				case IDC_GO:
					select_mode = (long)SendMessage(GetDlgItem(hWnd,IDC_VIDEOMODE), LB_GETCURSEL, 0, 0);
					setting_r = select_mode;
					setting_r|=(((long)SendMessage(GetDlgItem(hWnd,IDC_FULLSCR) , BM_GETCHECK , 0 , 0)==BST_CHECKED)<<8);
					setting_r|=(((long)SendMessage(GetDlgItem(hWnd, IDC_VSYNC) , BM_GETCHECK , 0 , 0)==BST_CHECKED)<<9);
					setting_r|=(((long)SendMessage(GetDlgItem(hWnd, IDC_FSAA4X) , BM_GETCHECK , 0 , 0)==BST_CHECKED)<<10);
					//setting_r|=(((long)SendMessage(GetDlgItem(hWnd, IDC_SHOWFPS) , BM_GETCHECK , 0 , 0)==BST_CHECKED)<<11);
					PostQuitMessage(0);
				break;
				case IDC_GOHP:
					ShellExecute(hWnd,"open","http://www.sys-k.net/",NULL,NULL,SW_NORMAL);
				break;
				case IDC_HALT:
					ExitProcess(0);
				break;
				//case IDC_MPLAY:
				//	EnableWindow(GetDlgItem(hWnd, IDC_MPLAY), FALSE);
				//	EnableWindow(GetDlgItem(hWnd, IDC_MSTOP), TRUE);
				//	sks.Clean();
				//	sks.Init();
				//	uraMusicInit(&sks);
				//	sks.Play();
				//break;
				//case IDC_MSTOP:
				//	EnableWindow(GetDlgItem(hWnd, IDC_MPLAY), TRUE);
				//	EnableWindow(GetDlgItem(hWnd, IDC_MSTOP), FALSE);
				//	sks.Clean();
				//break;
			}
		break;
		case WM_QUIT:
		case WM_CLOSE:
			ExitProcess(0);
		break;
	}
	return 0;
}

int __fastcall Settings()
{
	HWND hdlg = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DIALOG1),NULL,(DLGPROC)DialogProc);

#if MODE_PLAYER
	SetWindowText(hdlg, SZ_PROD_TITLE);
#elif MODE_DEMO
	SetWindowText(hdlg, SZ_PROD_TITLE);
#elif MODE_RESOURCE_LOADER
	SetWindowText(hdlg, resDemo.szRc_DemoTitle);
#endif

	//Add strings to VideoList
	HWND lst = GetDlgItem(hdlg,IDC_VIDEOMODE);

	DEVMODE* devModes = (DEVMODE*)GlobalAlloc(GPTR, sizeof(DEVMODE)*1024);
	DWORD dwValidSelectionCounter=0;
	DWORD dwPrefferedSelection=0;

	DEVMODE devMode;
	devMode.dmSize = sizeof(DEVMODE);
	devMode.dmDriverExtra = 0;
	DWORD dwModeNum = 0;
	char* szDisplaySetting = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
	while( EnumDisplaySettings(NULL, dwModeNum, &devMode) ){
		if( devMode.dmPelsWidth>=MINIMUM_SCREEN_WIDTH && devMode.dmPelsHeight>=MINIMUM_SCREEN_HEIGHT
		&&  devMode.dmBitsPerPel>=MINIMUM_SCREEN_BPP  && devMode.dmDisplayFrequency>=MINIMUM_SCREEN_FREQUENCY)
		{
			wsprintf(szDisplaySetting, "%dx%d:%dBits@%dHz",	devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmBitsPerPel, devMode.dmDisplayFrequency);
			SendMessage(lst, LB_ADDSTRING, 0, (LPARAM)szDisplaySetting);

			if( devMode.dmPelsWidth==PREF_SCREEN_WIDTH && devMode.dmPelsHeight==PREF_SCREEN_HEIGHT && devMode.dmBitsPerPel==PREF_SCREEN_BPP && devMode.dmDisplayFrequency==PREF_SCREEN_FREQUENCY){
				dwPrefferedSelection = dwValidSelectionCounter;
			}

			devModes[dwValidSelectionCounter] = devMode;
			++dwValidSelectionCounter;
		}

		++dwModeNum;
	}
	//GlobalFree(szDisplaySetting);

	//Set default settings
	SendMessage(lst , LB_SETCURSEL, dwPrefferedSelection, 0);
	SendMessage(GetDlgItem(hdlg,IDC_FULLSCR) , BM_SETCHECK , BST_CHECKED , 0);
	SendMessage(GetDlgItem(hdlg,IDC_VSYNC) , BM_SETCHECK , BST_CHECKED, 0);
	//SendMessage(GetDlgItem(hdlg,IDC_SHOWFPS) , BM_SETCHECK , BST_UNCHECKED, 0);
	//SendMessage(GetDlgItem(hdlg,IDC_FSAA4X) , BM_SETCHECK , BST_UNCHECKED, 0);
	EnableWindow(GetDlgItem(hdlg, IDC_FSAA4X), isMSSupported);
	ShowWindow(hdlg, TRUE);
	
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0)){
		DispatchMessage(&msg);
	}
	//ShowWindow(hdlg,0);
	DestroyWindow(hdlg);

	//Save selected display setting
	unsigned char video_mode = (setting_r&255);
	SCREEN_WIDTH	 = devModes[video_mode].dmPelsWidth;
	SCREEN_HEIGHT	 = devModes[video_mode].dmPelsHeight;
	SCREEN_BPP		 = devModes[video_mode].dmBitsPerPel;
	SCREEN_FREQUENCY = devModes[video_mode].dmDisplayFrequency;
	//GlobalFree(devModes);
	if (SCREEN_WIDTH >= 1920)
		highMode = true;

	return setting_r;
}


LRESULT CALLBACK NewWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
//#if MODE_PLAYER
	if(msg==WM_KEYDOWN){
		if((BYTE)wParam==VK_RIGHT){
			Demo.SeekRelative(-1000);
		}
		if((BYTE)wParam==VK_LEFT){
			Demo.SeekRelative(1000);
		}
	}
//#endif
	if(msg == WM_CLOSE){
		ExitProcess(0);
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CodeStart()
{
	InitializeProcedure();
	int st = Settings();
	//if(st<0) return;
	
	if(st&1024){
		kbInitMultiSample(nMultiSample);
	}
	//if(((BYTE)(st>>11))){
	//	isShowFPS = TRUE;
	//}

	char* SZ_DEMO_TITLE = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
#if   MODE_PLAYER
	wsprintf(SZ_DEMO_TITLE, "%s %s", SZ_PROD_TITLE, SZ_COPYRIGHT_INFO);
#elif MODE_DEMO
	wsprintf(SZ_DEMO_TITLE, "%s %s", SZ_PROD_TITLE, SZ_COPYRIGHT_INFO);
#elif MODE_RESOURCE_LOADER
	wsprintf(SZ_DEMO_TITLE, SZ_DEMO_TITLE_BASE, resDemo.szRc_DemoTitle, resDemo.nYear);
#endif
	//SetWindowText(kbhWnd, Demoname);
	//GlobalFree(Demoname);
	
//#ifdef _DEBUG
//	kbCreateWindow 0,0,SCREEN_WIDTH/100,SCREEN_HEIGHT/100,SZ_DEMO_TITLE, WS_OVERLAPPEDWINDOW);//ウインドウモード
//#else
	if(st&256){//フルスクリーンモード
		kbCreateWindow (0,0,SCREEN_WIDTH,SCREEN_HEIGHT, SZ_DEMO_TITLE, WS_POPUP|WS_VISIBLE);
		kbGoFullScreen (SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_BPP,SCREEN_FREQUENCY);
		ShowCursor(FALSE);
	}else{//ウインドウモード
		kbCreateWindow (0,0,SCREEN_WIDTH,SCREEN_HEIGHT, SZ_DEMO_TITLE, (WS_OVERLAPPEDWINDOW&~WS_THICKFRAME&~WS_MAXIMIZEBOX)|WS_VISIBLE);
		//ShowCursor(TRUE);
	}
//#endif

	SetWindowLong(kbhWnd, GWL_WNDPROC, (LONG)NewWndProc);
	ValidateRect(kbhWnd, NULL);
	if(st&512) EnableVsync(TRUE);

	InitGLState();

	MSG msg;
	BOOL isPressedEscape = FALSE;
	while(TRUE){
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
			if((msg.message==WM_KEYDOWN)&&(msg.wParam==VK_ESCAPE)){
				if(!isPressedEscape){
					isPressedEscape = TRUE;//fade out
					continue;
				}else{
					break;//while fade-out, quit immediately
				}
			}else{
				//TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}else{
			if(isPressedEscape){
				if(OnExitFade()){
					break;
				}
			}else{
				if(OnIdle()){
					break;
				}
			}
		}
	}

	//後処理
	ExitProcess(0);//全スレッド強制終了
}

//#ifdef NOT64K
int __stdcall WinMain(HINSTANCE h, HINSTANCE p, LPSTR s, int n){
	CodeStart();
	return 0;
}
//#endif