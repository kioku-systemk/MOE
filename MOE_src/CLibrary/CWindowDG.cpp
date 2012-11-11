#include "./CWindowDG.h"


CWindowDG::CWindowDG() : CWindow()
{
	m_pD3D = NULL;
	m_pD3DDev =NULL;
	m_pBackBuffer = NULL;

	m_isActive = false;
}

CWindowDG::~CWindowDG()
{
	CleanD3D();
}

///////////////////
//表示関連
//////////////////
LPDIRECT3DDEVICE9 CWindowDG::GetDirect3DDevice(){ return m_pD3DDev; };

void CWindowDG::ClearScreen(UCHAR r, UCHAR g, UCHAR b)
{
	m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER ,D3DCOLOR_XRGB(r,g,b), 1.0f, 0);
}

void CWindowDG::RedrawScreen()
{
	m_pD3DDev->Present(NULL, NULL, NULL, NULL);
}

void CWindowDG::SetProjection(long x, long y, double sight, long width, long height, double zNear, double zFar)
{
}

////////////////////////////
//D3D初期化と開放
////////////////////////////
bool CWindowDG::InitD3D(HWND hWnd, LONG width, LONG height, BOOL isWindow)
{
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(m_pD3D == NULL) return FALSE;

	D3DDISPLAYMODE d3ddm;
	if(FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
		return FALSE;

	//フォーマットの設定
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp,sizeof(d3dpp));
	d3dpp.Windowed = isWindow;
	d3dpp.BackBufferWidth  = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = d3ddm.Format;
	d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if(FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &m_pD3DDev)))
		if(FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,hWnd ,D3DCREATE_SOFTWARE_VERTEXPROCESSING ,&d3dpp, &m_pD3DDev)))
			if(FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF,hWnd ,D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp,&m_pD3DDev)))
				return FALSE;

	if(FAILED(m_pD3DDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &m_pBackBuffer)))
	{
		return FALSE;
	}
	return TRUE;
}

void CWindowDG::CleanD3D()
{
	if(m_pD3DDev!=NULL) m_pD3DDev->Release();
	if(m_pD3D!=NULL) m_pD3D->Release();
	if(m_pBackBuffer!=NULL) m_pBackBuffer->Release();
}

void CWindowDG::GetDC(HDC* hDC)
{
	m_pBackBuffer->GetDC(hDC);
}

void CWindowDG::ReleaseDC(HDC hDC)
{
	m_pBackBuffer->ReleaseDC(hDC);
}

///////////////////////////////////
//ウィンドウ作成とプロシージャ
///////////////////////////////////
bool CWindowDG::CCreateWindow(LONG width, LONG height, const TCHAR* szTitle, BOOL isWindow)
{
	return CCreateWindow((GetSystemMetrics(SM_CXSCREEN) - width)/2, (GetSystemMetrics(SM_CYSCREEN) - height)/2, width, height, szTitle, isWindow);
}

//Create a Direct3D Window
//まだFullscreenに対応してない
bool CWindowDG::CCreateWindow(LONG x, LONG y, LONG width, LONG height, const TCHAR* szTitle, BOOL isWindow)
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
	if(!GetClassInfo(GetModuleHandle(NULL), _T("CWindowDG"), &wc))
	{
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC) CWindowBase::BaseWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName	= NULL;
		wc.lpszClassName	= _T("CWindowDG");

		if(!RegisterClass(&wc))
		{
			MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
			ExitProcess(0);
			return FALSE;
		}
	}
	
	if(m_isFullscreen)
	{
	}

	if(m_isFullscreen)
	{
		dwExStyle=WS_EX_APPWINDOW;
		dwStyle=WS_POPUP;
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	if(!(hWnd=CreateWindowEx(	dwExStyle,
								_T("CWindowDG"),
								szTitle,
								dwStyle,
								x, y,
								!m_isFullscreen ? WindowRect.right-WindowRect.left : 0,
								!m_isFullscreen ? WindowRect.bottom-WindowRect.top : 0,
								NULL,
								NULL,
								GetModuleHandle(NULL),
								(void*)this)))
	{
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		ExitProcess(0);
		return FALSE;
	}

	m_hWnd = hWnd;

	if(!InitD3D(m_hWnd, width, height, isWindow))
	{
		MessageBox(NULL,"Direct3D Initialize Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		ExitProcess(0);
		return FALSE;
	}

	ShowWindow(hWnd,SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	return TRUE;
}

LRESULT CALLBACK CWindowDG::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_ACTIVATE:
		{
			//フォーカスロスト時の処理
			if ((LOWORD(wParam) != WA_INACTIVE) && !((BOOL)HIWORD(wParam)))
				m_isActive=true;
			else
				m_isActive=false;
			return 0;
		}

		case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
				return 0;
			}
			break;
		}

		case WM_SIZE:
		{
			//リサイズの処理
			return 0;
		}
	}
	return CWindow::WndProc(hWnd, msg, wParam, lParam);
}

void CWindowDG::ResizeWindow(long width, long height)
{
	if(height==0) height=1;
	SetProjection(0, 0, 60.0, width, height, 0, 1000);
}