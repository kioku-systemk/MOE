#include "stdafx.h"
#include "./CWindow.h"

CWindow::CWindow()
{
	m_pKeyFunc = NULL;
	m_pMouseFunc = NULL;
	m_pRenderFunc = NULL;
	m_pIdleFunc = NULL;
	m_pTimerFunc = NULL;
	m_pHookProc = NULL;

	m_hAccel = NULL;

	m_hWnd = NULL;
	m_isFullscreen = false;
	m_width = 0;
	m_height = 0;
}

CWindow::~CWindow()
{
}

bool CWindow::CCreateWindow(LONG width, LONG height, const TCHAR* szTitle, const TCHAR* szMenuName, BOOL isWindow)
{
	m_width = width;
	m_height = height;
	if(isWindow)
		return CCreateWindow((GetSystemMetrics(SM_CXSCREEN) - width)/2, (GetSystemMetrics(SM_CYSCREEN) - height)/2, width, height, szTitle, szMenuName, isWindow);
	return CCreateWindow(0, 0, width, height, szTitle, szMenuName, isWindow);
}

bool CWindow::CCreateWindow(LONG x, LONG y, LONG width, LONG height, const TCHAR* szTitle, const TCHAR* szMenuName, BOOL isWindow)
{
	WNDCLASS	wc;
	HWND		hWnd;
	DWORD		dwExStyle;
	DWORD		dwStyle;
	RECT		WindowRect;
	WindowRect.left  = 0;
	WindowRect.right = width;
	WindowRect.top   = 0;
	WindowRect.bottom= height;

	m_isFullscreen=!isWindow;

	//すでに登録されていなければ、登録する
	if(!GetClassInfo(GetModuleHandle(NULL), _T("CWindow"), &wc))
	{
//		memset(&wcx, 0, sizeof(WNDCLASSEX));
//		wcx.cbSize			= sizeof(WNDCLASSEX);
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		//staticなBaseWndProcを指定しているので、２個目のウィンドウとウィンドウクラスがかぶってもOK
		wc.lpfnWndProc		= (WNDPROC) CWindowBase::BaseWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName		= szMenuName;
		wc.lpszClassName	= _T("CWindow");

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
		//dmScreenSettings.dmDisplayFrequency = 60;
		dmScreenSettings.dmFields=DM_PELSWIDTH|DM_PELSHEIGHT;//|DM_DISPLAYFREQUENCY;

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
//		ShowCursor(FALSE);
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
	}

	//なんかこれ、うまくうごいてないっぽい
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	//本来の計算式
	//POINT window_err;
	//window_err.x = 2*GetSystemMetrics(SM_CXFIXEDFRAME);
	//window_err.y = GetSystemMetrics(SM_CYCAPTION) + 2*GetSystemMetrics(SM_CYFIXEDFRAME);

	if(!(hWnd=CreateWindowEx(	dwExStyle,
								_T("CWindow"),
								szTitle,
								dwStyle,// | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
								x, y,
								!m_isFullscreen ? WindowRect.right-WindowRect.left : 0,
								!m_isFullscreen ? WindowRect.bottom-WindowRect.top : 0,
								NULL,
								NULL,
								GetModuleHandle(NULL),
								(void*)this)))//BaseWndProcにthisを渡してやる
	{
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		ExitProcess(0);
		return FALSE;
	}

	m_hWnd = hWnd;

	ShowWindow(hWnd,SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	return TRUE;
}

void CWindow::LoadAccelerators(const TCHAR* szAccel)
{
	m_hAccel = ::LoadAccelerators(GetModuleHandle(NULL), szAccel);
}

WPARAM CWindow::CMessageLoop()
{
	//アクセラレータ対応済み
	MSG msg;

	while(1)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			//WM_QUITを明示的に処理する
			if(msg.message==WM_QUIT) break;
			if(m_hAccel){
				if(!TranslateAccelerator(CWindow::CGethWnd(), m_hAccel, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}else{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
			}
		}
		else
		{
			if(m_pIdleFunc==NULL)
			{//IdleFuncを設定してないときに
				while (GetMessage(&msg, NULL, 0, 0))//CPUが100%になるのをおさえれるこっちのループを使う
				{
					if(m_hAccel){
						if(!TranslateAccelerator(CWindow::CGethWnd(), m_hAccel, &msg))
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}else{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				if(msg.message==WM_QUIT) break;//終了のメッセージなら外側のループからでる
			}
			else
			{//他になにも処理がないのでIdle関数を実行する
				(*m_pIdleFunc)();
			}
		}
	}

	if(m_isFullscreen)
	{
		ChangeDisplaySettings(NULL,0);
		ShowCursor(TRUE);
	}
	return msg.wParam;
}

void CWindow::CSetCallbackFunctions(void key(UCHAR, bool), void click(long, long, int, bool), void render(HDC), void idle(), void timer(), long interval)
{
	m_pKeyFunc = key;
	m_pMouseFunc = click;
	m_pRenderFunc = render;
	m_pIdleFunc = idle;
	if(timer != NULL)
	{
		if(m_pTimerFunc == NULL)
		{
			SetTimer(m_hWnd, 1, interval, NULL);
		}
		else
		{
			KillTimer(m_hWnd, 1);
			SetTimer(m_hWnd, 1, interval, NULL);
		}
		m_pTimerFunc = timer;
	}
}

void CWindow::CSetHookProcedure(LRESULT CALLBACK HookProc(HWND, UINT, WPARAM, LPARAM))
{
	m_pHookProc = HookProc;
}

void CWindow::CSetIdleFunc(void idle()){
	this->m_pIdleFunc = idle;
}

POINT* CWindow::GetWindowPos()
{
	return &m_lastpos;
}

//Static Callback
LRESULT CALLBACK CWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	long x,y;
	
	if(m_pHookProc != NULL)
	{
		int ret = 0;
		ret = m_pHookProc(hWnd, msg, wParam, lParam);
		if(ret!=TRUE) return ret;
	}
/*
	int ret = 0;
	SAFE_CALLBACK_CHECK(ret, m_pHookProc, (hWnd, msg, wParam, lParam));
	if(ret!=TRUE)
		//return DefWindowProc(hWnd, msg, wParam, lParam);
		return ret;
*/
	switch( msg )
	{
		case WM_CREATE:
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			if(m_pRenderFunc != NULL) m_pRenderFunc(hdc);
			EndPaint(hWnd, &ps);
		break;

		case WM_TIMER:
			if(wParam == 1)
				if(m_pTimerFunc != NULL) m_pTimerFunc();
		break;

		case WM_LBUTTONDOWN:
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if(m_pMouseFunc != NULL) m_pMouseFunc(x, y, LBUTTON, TRUE);
		break;
		case WM_LBUTTONUP:
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if(m_pMouseFunc != NULL) m_pMouseFunc(x, y, LBUTTON, FALSE);
		break;

		case WM_RBUTTONDOWN:
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if(m_pMouseFunc != NULL) m_pMouseFunc(x, y, RBUTTON, TRUE);
		break;
		case WM_RBUTTONUP:
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if(m_pMouseFunc != NULL) m_pMouseFunc(x, y, RBUTTON, FALSE);
		break;

		case WM_MBUTTONDOWN:
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if(m_pMouseFunc != NULL) m_pMouseFunc(x, y, MBUTTON, TRUE);
		break;
		case WM_MBUTTONUP:
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if(m_pMouseFunc != NULL) m_pMouseFunc(x, y, MBUTTON, FALSE);
		break;

		case WM_MOUSEWHEEL:
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if(m_pMouseFunc!=NULL){
				if((int)wParam>0){
					m_pMouseFunc(x, y, WHEEL, FALSE);
				}else{
					m_pMouseFunc(x, y, WHEEL, TRUE);
				}
			}
		break;

		case WM_MOUSEMOVE:
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if(m_pMouseFunc != NULL) m_pMouseFunc(x, y, MOUSEMOVE, FALSE);
		break;

		case WM_KEYDOWN:
			if(m_pKeyFunc != NULL) m_pKeyFunc((UCHAR)wParam, TRUE);
		break;
		case WM_KEYUP:
			if(m_pKeyFunc != NULL) m_pKeyFunc((UCHAR)wParam, FALSE);
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
		break;

		case WM_MOVE:
		{
			m_lastpos.x = LOWORD(lParam);
			m_lastpos.y = HIWORD(lParam);
			break;
		}
		case WM_SIZE:
		{
			RECT crect;
			GetClientRect(hWnd, &crect);
			m_width = crect.right - crect.left;
			m_height = crect.bottom - crect.top;
		}
		break;

	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

HWND CWindow::CGethWnd(){ return m_hWnd; }
void CWindow::CExit(){ CExit(0); }
void CWindow::CExit(int exit){ PostQuitMessage(exit); }
HMENU CWindow::GetMenu(){ return ::GetMenu(m_hWnd); }
