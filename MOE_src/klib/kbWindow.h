/*
	4KB/64KB OpenGL Intro Template
	coded by kioku@Cyber K
*/

//浮動小数点使用のため必要
#ifdef NOT64K
#elif _DEBUG
#else
#ifdef __cplusplus
extern "C" { 
#endif
	int _fltused=1; 
	void _cdecl _check_commonlanguageruntime_version(){}
#ifdef __cplusplus
}
#endif
#endif
//基本的なライブラリの宣言
//#pragma comment(linker,"/subsystem:windows")
//#pragma comment(linker,"/NODEFAULTLIB")
//プロジェクト設定のリンカの方で設定
//#pragma comment(lib,"winmm.lib")
//#pragma comment(lib,"opengl32.lib")
//#pragma comment(lib,"glu32.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "../GL/glext.h"
#include "../GL/wglext.h"

extern PFNGLISRENDERBUFFEREXTPROC		glIsRenderbufferEXT;
extern PFNGLBINDRENDERBUFFEREXTPROC		glBindRenderbufferEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC	glDeleteRenderbuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC		glGenRenderbuffersEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC	glRenderbufferStorageEXT;
extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT;
extern PFNGLISFRAMEBUFFEREXTPROC		glIsFramebufferEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC		glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC	glDeleteFramebuffersEXT;
extern PFNGLGENFRAMEBUFFERSEXTPROC		glGenFramebuffersEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
extern PFNGLGENERATEMIPMAPEXTPROC		glGenerateMipmapEXT;

extern PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD3FARBPROC		glMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD4FARBPROC		glMultiTexCoord4fARB;
extern PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
extern PFNWGLCHOOSEPIXELFORMATARBPROC	wglChoosePixelFormatARB;

//================================================================================================
//ユーティリティ
#define MsgBox(str1,str2)		MessageBox(NULL,str1,str2,MB_OK)
//#define kbCreateWindowFromDevMode(__x_in_int, __y_in_int, __devmode_in_devmode, __title_in_char_ptr, __style_in_ulong)\
//	kbCreateWindow(__x_in_int, __y_in_int, __devmode_in_devmode.dmPelsWidth, __devmode_in_devmode.dmPelsHeight, __title_in_char_ptr, __style_in_ulong)
//#define kbGoFullScreen

//==========================Window関連と初期化処理===============================================
//HINSTANCE kb_hInstance;
HWND kbhWnd;
HDC  kbhDC;//デバイスコンテキスト
HGLRC kbhRC;

static bool disp_change=false;
int MultiSamplePixelFormat = 0;

inline HGLRC __fastcall kbInit_Pixel(HWND hWnd)
{
	HGLRC hRC;
	int pixelformat;
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),		//この構造体のサイズ
		1,									//OpenGLバージョン
		PFD_DRAW_TO_WINDOW |				//Windowスタイル
		PFD_SUPPORT_OPENGL |				//OpenGL
		PFD_DOUBLEBUFFER,					//ダブルバッファ使用可能
		PFD_TYPE_RGBA,						//RGBAカラー
		24,									//色数
		0, 0,								//RGBAのビットとシフト設定        
		0, 0,								//G
		0, 0,								//B
		0, 0,								//A
		0,									//アキュムレーションバッファ
		0, 0, 0, 0,							//RGBAアキュムレーションバッファ
		24,									//Zバッファ    
		0,									//ステンシルバッファ
		0,									//使用しない
		PFD_MAIN_PLANE,						//レイヤータイプ
		0,									//予約
		0, 0, 0								//レイヤーマスクの設定・未使用
	};
	if(MultiSamplePixelFormat==0){		
		//ピクセルフォーマットの指定 //OpenGLレンダリングコンテキストの作成
		if(((pixelformat = ChoosePixelFormat(kbhDC, &pfd)) == 0)
		|| ((SetPixelFormat(kbhDC, pixelformat, &pfd) == FALSE))
		|| (!(hRC=wglCreateContext(kbhDC))))    return NULL;
	}else{
		if((SetPixelFormat(kbhDC, MultiSamplePixelFormat, &pfd) == FALSE)
			||(!(hRC=wglCreateContext(kbhDC))))
		{
			return NULL;
		}
	}
	kbhRC = hRC;
	return hRC;
}


inline void __fastcall kbSetViewPerspective(int x,short y, int width, int height,float zNear,float zFar)
{
	glViewport(x, y, width, height);
	const float aspect = (GLfloat)width/(GLfloat)height;//アスペクト比の初期化
	glMatrixMode( GL_PROJECTION );//プロジェクションモードで射影
	gluPerspective( 60.0f, aspect, zNear, zFar);
	glMatrixMode( GL_MODELVIEW );//ノーマルのモデルビューモードへ移行
}

#define kbSetViewport(hWnd, w, h)	kbSetViewPerspective(0, 0, w, h ,0.1f,1000.0f)

#define kbSwapBuffers()		SwapBuffers(kbhDC)

//ウインドウを作成する
inline void __fastcall kbCreateWindow(int x, int y, int width, int height, const char* title, DWORD style)
{
	//RECT rect;
	//rect.bottom = height;
	//rect.top	= y;
	//rect.right  = width;
	//rect.left   = x;
	//AdjustWindowRectEx(&rect, style, FALSE, WS_EX_APPWINDOW);
	//width = rect.right - rect.left;
	//height = rect.bottom - rect.top;

	//Window Create
	//HWND kbhWnd;//ウインドウハンドル
	kbhWnd = CreateWindowEx(WS_EX_APPWINDOW,"static" , title ,//EDITクラスでウインドウ作成
		//WS_POPUP,
		//WS_OVERLAPPEDWINDOW,
			style,
			x, y,
			width,height,
			NULL, NULL, GetModuleHandle(NULL), NULL);
			//NULL, NULL, kb_hInstance, NULL);

	//if (!kbhWnd) return;//作成失敗
	kbhDC = GetDC(kbhWnd);
	
	//PixelFormat初期化
	wglMakeCurrent(kbhDC,kbInit_Pixel (kbhWnd));

	//Viewportの設定
	kbSetViewport(kbhWnd,width,height);
	
	//ShowWindow(kbhWnd, SW_SHOWNORMAL);
	//SetWindowPos(kbhWnd,(HWND)-1,0,0,width,height,SWP_SHOWWINDOW);
	//ShowCursor(FALSE);
	return;
}

//解像度変更
inline bool __fastcall kbChangeDisplaySetting(const int width,const int height,const int bpp, const int freq)
{
	//設定
	DEVMODE devmode;
	devmode.dmSize = sizeof(devmode);
	devmode.dmDriverExtra = 0;
	devmode.dmPelsWidth = width;
	devmode.dmPelsHeight = height;
	devmode.dmBitsPerPel = bpp;
	devmode.dmDisplayFrequency = freq;
	devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	
	//変更
	return (ChangeDisplaySettings(&devmode,CDS_FULLSCREEN)==DISP_CHANGE_SUCCESSFUL);
}

//フルスクリーン表示
inline void __fastcall kbGoFullScreen(const int width,const int height, const int bpp, const int freq)
{
	if(!disp_change) disp_change=kbChangeDisplaySetting(width ,height,bpp, freq);
}

inline BOOL kbInitMultiSample(int sample_cnt){//FSAA
	BOOL isOK = FALSE;
	
	int		pixelFormat;
	UINT	numFormats;
	float	fAttributes[] = {0,0};

	const int iAttributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
		WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,24,
		WGL_ALPHA_BITS_ARB,8,
		WGL_DEPTH_BITS_ARB,16,
		WGL_STENCIL_BITS_ARB,0,
		WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
		WGL_SAMPLES_ARB,sample_cnt,
		0,0
	};

	kbCreateWindow(0, 0, 1, 1, "", WS_POPUP);//dummy window
	HDC hDC = GetDC(kbhWnd);
	if(wglChoosePixelFormatARB(hDC, iAttributes, fAttributes,1,&pixelFormat,&numFormats) && numFormats>=1){
		MultiSamplePixelFormat = pixelFormat;
	}
	DestroyWindow(kbhWnd);
	return isOK;
}
