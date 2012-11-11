/*
	4KB/64KB OpenGL Intro Template
	coded by kioku@Cyber K
*/

//���������_�g�p�̂��ߕK�v
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
//��{�I�ȃ��C�u�����̐錾
//#pragma comment(linker,"/subsystem:windows")
//#pragma comment(linker,"/NODEFAULTLIB")
//�v���W�F�N�g�ݒ�̃����J�̕��Őݒ�
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
//���[�e�B���e�B
#define MsgBox(str1,str2)		MessageBox(NULL,str1,str2,MB_OK)
//#define kbCreateWindowFromDevMode(__x_in_int, __y_in_int, __devmode_in_devmode, __title_in_char_ptr, __style_in_ulong)\
//	kbCreateWindow(__x_in_int, __y_in_int, __devmode_in_devmode.dmPelsWidth, __devmode_in_devmode.dmPelsHeight, __title_in_char_ptr, __style_in_ulong)
//#define kbGoFullScreen

//==========================Window�֘A�Ə���������===============================================
//HINSTANCE kb_hInstance;
HWND kbhWnd;
HDC  kbhDC;//�f�o�C�X�R���e�L�X�g
HGLRC kbhRC;

static bool disp_change=false;
int MultiSamplePixelFormat = 0;

inline HGLRC __fastcall kbInit_Pixel(HWND hWnd)
{
	HGLRC hRC;
	int pixelformat;
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),		//���̍\���̂̃T�C�Y
		1,									//OpenGL�o�[�W����
		PFD_DRAW_TO_WINDOW |				//Window�X�^�C��
		PFD_SUPPORT_OPENGL |				//OpenGL
		PFD_DOUBLEBUFFER,					//�_�u���o�b�t�@�g�p�\
		PFD_TYPE_RGBA,						//RGBA�J���[
		24,									//�F��
		0, 0,								//RGBA�̃r�b�g�ƃV�t�g�ݒ�        
		0, 0,								//G
		0, 0,								//B
		0, 0,								//A
		0,									//�A�L�������[�V�����o�b�t�@
		0, 0, 0, 0,							//RGBA�A�L�������[�V�����o�b�t�@
		24,									//Z�o�b�t�@    
		0,									//�X�e���V���o�b�t�@
		0,									//�g�p���Ȃ�
		PFD_MAIN_PLANE,						//���C���[�^�C�v
		0,									//�\��
		0, 0, 0								//���C���[�}�X�N�̐ݒ�E���g�p
	};
	if(MultiSamplePixelFormat==0){		
		//�s�N�Z���t�H�[�}�b�g�̎w�� //OpenGL�����_�����O�R���e�L�X�g�̍쐬
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
	const float aspect = (GLfloat)width/(GLfloat)height;//�A�X�y�N�g��̏�����
	glMatrixMode( GL_PROJECTION );//�v���W�F�N�V�������[�h�Ŏˉe
	gluPerspective( 60.0f, aspect, zNear, zFar);
	glMatrixMode( GL_MODELVIEW );//�m�[�}���̃��f���r���[���[�h�ֈڍs
}

#define kbSetViewport(hWnd, w, h)	kbSetViewPerspective(0, 0, w, h ,0.1f,1000.0f)

#define kbSwapBuffers()		SwapBuffers(kbhDC)

//�E�C���h�E���쐬����
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
	//HWND kbhWnd;//�E�C���h�E�n���h��
	kbhWnd = CreateWindowEx(WS_EX_APPWINDOW,"static" , title ,//EDIT�N���X�ŃE�C���h�E�쐬
		//WS_POPUP,
		//WS_OVERLAPPEDWINDOW,
			style,
			x, y,
			width,height,
			NULL, NULL, GetModuleHandle(NULL), NULL);
			//NULL, NULL, kb_hInstance, NULL);

	//if (!kbhWnd) return;//�쐬���s
	kbhDC = GetDC(kbhWnd);
	
	//PixelFormat������
	wglMakeCurrent(kbhDC,kbInit_Pixel (kbhWnd));

	//Viewport�̐ݒ�
	kbSetViewport(kbhWnd,width,height);
	
	//ShowWindow(kbhWnd, SW_SHOWNORMAL);
	//SetWindowPos(kbhWnd,(HWND)-1,0,0,width,height,SWP_SHOWWINDOW);
	//ShowCursor(FALSE);
	return;
}

//�𑜓x�ύX
inline bool __fastcall kbChangeDisplaySetting(const int width,const int height,const int bpp, const int freq)
{
	//�ݒ�
	DEVMODE devmode;
	devmode.dmSize = sizeof(devmode);
	devmode.dmDriverExtra = 0;
	devmode.dmPelsWidth = width;
	devmode.dmPelsHeight = height;
	devmode.dmBitsPerPel = bpp;
	devmode.dmDisplayFrequency = freq;
	devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	
	//�ύX
	return (ChangeDisplaySettings(&devmode,CDS_FULLSCREEN)==DISP_CHANGE_SUCCESSFUL);
}

//�t���X�N���[���\��
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
