#include "stdafx.h"
#include "./CWindowGL.h"

CWindowGL::CWindowGL() : CWindow()
{
}

CWindowGL::~CWindowGL()
{
}

void CWindowGL::ClearScreen(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void CWindowGL::RedrawScreen()
{
	if(m_hDC != wglGetCurrentDC())
	{
		ResizeGLWindow(m_lastsize.x , m_lastsize.y);
		wglMakeCurrent(m_hDC, m_hRC);
	}
	SwapBuffers(m_hDC);
}

bool CWindowGL::CCreateWindow(LONG width, LONG height, const TCHAR* szTitle, const TCHAR* szMenuName, BOOL isWindow)
{
	if(isWindow)
		return CCreateWindow((GetSystemMetrics(SM_CXSCREEN) - width)/2, (GetSystemMetrics(SM_CYSCREEN) - height)/2, width, height, szTitle, szMenuName, isWindow);
	return CCreateWindow(0, 0, width, height, szTitle, szMenuName, isWindow);
}

//Create a OpenGL Window
bool CWindowGL::CCreateWindow(LONG x, LONG y, LONG width, LONG height, const TCHAR* szTitle, const TCHAR* szMenuName, BOOL isWindow)
{
	//normal window
	WNDCLASS	wc;
	HWND		hWnd;
	DWORD		dwExStyle;
	DWORD		dwStyle;
	RECT		WindowRect;
	WindowRect.left  = 0;
	WindowRect.right = width;
	WindowRect.top   = 0;
	WindowRect.bottom= height;

	//OpenGL
	int			PixelFormat;
	
	m_isFullscreen=!isWindow;

	//すでに登録されていなければ、登録する
	if(!GetClassInfo(GetModuleHandle(NULL), _T("CWindowGL"), &wc))
	{
//		memset(&wcx, 0, sizeof(WNDCLASSEX));
//		wcx.cbSize			= sizeof(WNDCLASSEX);
		wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		//staticなBaseWndProcを指定しているので、２個目のウィンドウとウィンドウクラスがかぶってもOK
		wc.lpfnWndProc		= (WNDPROC) CWindowBase::BaseWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName	    = szMenuName;
		wc.lpszClassName	= _T("CWindowGL");

		if(!RegisterClass(&wc))
		{
			MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
			ExitProcess(0);
			return FALSE;
		}
	}

	if(m_isFullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings,0,sizeof(DEVMODE));
		dmScreenSettings.dmSize=sizeof(DEVMODE);
		dmScreenSettings.dmPelsWidth	= width;
		dmScreenSettings.dmPelsHeight	= height;
		dmScreenSettings.dmDisplayFrequency = 60;
		dmScreenSettings.dmFields=DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;

		if(ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			if(MessageBox(NULL,"Failed To Switch To Fullsereen Mode. Run In Window Mode?","ERROR",MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
			{
				m_isFullscreen=FALSE;
			}
			else
			{
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				ExitProcess(0);
				return FALSE;
			}
		}
	}

	if(m_isFullscreen)
	{
		dwExStyle=WS_EX_APPWINDOW;
		dwStyle=WS_POPUP;
		ShowCursor(FALSE);
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	}

	//なんかこれ、うまくうごいてないっぽい
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	//本来の計算式
	//POINT window_err;
	//window_err.x = 2*GetSystemMetrics(SM_CXFIXEDFRAME);
	//window_err.y = GetSystemMetrics(SM_CYCAPTION) + 2*GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYMENU);

	if(!(hWnd=CreateWindowEx(	dwExStyle,
								_T("CWindowGL"),
								szTitle,
								dwStyle,
								x, y,
								!m_isFullscreen ? WindowRect.right-WindowRect.left : width,
								!m_isFullscreen ? WindowRect.bottom-WindowRect.top : height,
								NULL,
								NULL,
								GetModuleHandle(NULL),
								(void*)this)))//BaseWndProcにthisを渡してやる
	{
		KillGLWindow();
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		ExitProcess(0);
		return FALSE;
	}

	static	PIXELFORMATDESCRIPTOR pfd=
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,	//Z-Buffer
		0,  //Stencil Buffer
		0,
		PFD_MAIN_PLANE,
		0, //Reserved
		0, 0, 0	
	};

	if (!(m_hDC=::GetDC(hWnd)))
	{
		KillGLWindow();
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(PixelFormat=ChoosePixelFormat(m_hDC,&pfd)))
	{
		KillGLWindow();
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!SetPixelFormat(m_hDC,PixelFormat,&pfd))
	{
		KillGLWindow();
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(m_hRC=wglCreateContext(m_hDC)))
	{
		KillGLWindow();
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!wglMakeCurrent(m_hDC,m_hRC))
	{
		KillGLWindow();
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	m_hWnd = hWnd;

	ShowWindow(hWnd,SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	ResizeGLWindow(width, height);

	if(!InitGL())
	{
		KillGLWindow();
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	return TRUE;
}
/*
WPARAM CWindowGL::CMessageLoop()
{
	MSG msg;

	while(1)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			//WM_QUITを明示的に処理する
			if(msg.message==WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if(m_pIdleFunc==NULL)
			{//IdleFuncを設定してないときに
				while (GetMessage(&msg, NULL, 0, 0))//CPUが100%になるのをおさえれるこっちのループを使う
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				if(msg.message==WM_QUIT) break;//終了のメッセージなら外側のループからでる
			}
			else
			{
				if(m_isActive)
				{
					//他になにも処理がないのでIdle関数を実行する
					(*m_pIdleFunc)();
				}
			}
		}
	}

	KillGLWindow();
	return msg.wParam;
}
*/
void CWindowGL::KillGLWindow()
{
	if (m_isFullscreen)
	{
		ChangeDisplaySettings(NULL,0);
		ShowCursor(TRUE);
	}

	if (m_hRC)
	{
		if (!wglMakeCurrent(NULL,NULL))
		{
//			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(m_hRC))
		{
//			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		m_hRC=NULL;
	}

	if (m_hDC && !ReleaseDC(m_hWnd,m_hDC))
	{
//		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		m_hDC=NULL;
	}

	if (m_hWnd && !DestroyWindow(m_hWnd))
	{
//		MessageBox(NULL,"Could Not Release m_hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		m_hWnd=NULL;
	}

	if (!UnregisterClass(_T("CWindowGL"),GetModuleHandle(NULL)))
	{
//		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
	}
}

void CWindowGL::ReCreateWindow()
{
	KillGLWindow();
//	CCreateWindow(
}

bool CWindowGL::InitGL()
{
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	return true;
}

void CWindowGL::ResizeGLWindow(long width, long height)
{
	if(height==0) height=1;
	SetProjection(0, 0, 60.0, width, height, 0, 1000);
}

void CWindowGL::SetProjection(long x, long y, double sight, long width, long height, double zNear, double zFar)
{
	glViewport(x,y,width,height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,1.0f,1000.0f);
	gluLookAt(0, 0, 200, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

bool CWindowGL::EnableVsync(bool isvsync)
{
	typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
	PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = NULL;
	DWORD LastError = 0;

	const unsigned char *extensions = glGetString( GL_EXTENSIONS );

	if( strstr( (const char *)extensions, "WGL_EXT_swap_control" ) == 0 )
	{
		MessageBox(NULL,"WGL_EXT_swap_control is not supported.","ERROR",MB_ICONEXCLAMATION);
		return FALSE;
	}
	else
	{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

		if( wglSwapIntervalEXT )
			wglSwapIntervalEXT( (isvsync==true) ? 1:0 );
		else MessageBox(NULL,"couldn't get proc address.","",0);
		LastError = GetLastError();
		
		if(LastError==ERROR_INVALID_DATA)
		{
			MessageBox(NULL,"invalid stat.","ERROR",MB_ICONEXCLAMATION);
			return FALSE;
		}else if(LastError==ERROR_DC_NOT_FOUND){
			MessageBox(NULL,"DC or RC not found.","ERROR",MB_ICONEXCLAMATION);
			return FALSE;
		}
	}
	return TRUE;
}

LRESULT CALLBACK CWindowGL::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			// LoWord Can Be WA_INACTIVE, WA_ACTIVE, WA_CLICKACTIVE,
			// The High-Order Word Specifies The Minimized State Of The Window Being Activated Or Deactivated.
			// A NonZero Value Indicates The Window Is Minimized.
			if ((LOWORD(wParam) != WA_INACTIVE) && !((BOOL)HIWORD(wParam)))
				m_isActive=true;						// Program Is Active
			else
				m_isActive=false;						// Program Is No Longer Active

			return 0;								// Return To The Message Loop
		}

		//case WM_DESTROY:
		//	return 0;
		//case WM_CLOSE:
		//	PostQuitMessage(0);
		//	return 0;

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;
		}

		case WM_SIZE:
		{
			m_lastsize.x = LOWORD(lParam);
			m_lastsize.y = HIWORD(lParam);
			ResizeGLWindow(LOWORD(lParam),HIWORD(lParam));

			RECT crect;
			GetClientRect(hWnd, &crect);
			m_width = crect.right - crect.left;
			m_height = crect.bottom - crect.top;
			break;
		}
	}

	//Call CWindow::WndProc() for normal event.
	return CWindow::WndProc(hWnd, msg, wParam, lParam);
}