//CWindow class family -> CWindowDG
// code : c.r.v. 2005
// About   : DirectGraphics用ウィンドウの作成とそのイベント処理を担う
// Version : 0.3
// Date	   : 05/11/2005

#ifndef CWINDOWDG_H_CRV
#define CWINDOWDG_H_CRV

#include "CWindow.h"

#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9")//ライブラリをリンク
#pragma comment(lib, "d3dx9")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "winmm")

class CWindowDG : public CWindow
{
public:
	CWindowDG();
	virtual ~CWindowDG();
public:
	bool CCreateWindow(LONG width = 0, LONG height = 0, const TCHAR* szTitle = "", BOOL isWindow = TRUE);
	bool CCreateWindow(LONG x = 0, LONG y = 0, LONG width = 0, LONG height = 0, const TCHAR* szTitle = "", BOOL isWindow = TRUE);
	
	void ClearScreen(UCHAR r = 255, UCHAR g = 255, UCHAR b = 255);
	void RedrawScreen();
	LPDIRECT3DDEVICE9 GetDirect3DDevice();

	//デバコンGET
	void GetDC(HDC* hDC);
	void ReleaseDC(HDC hDC);

private:
	//DirectGraphicsインターフェイス
	LPDIRECT3D9			m_pD3D;
	//ビデオカードインターフェイス
	LPDIRECT3DDEVICE9	m_pD3DDev;

	//バックバッファ
	LPDIRECT3DSURFACE9	m_pBackBuffer;

	bool m_isActive;
private:
	bool InitD3D(HWND hWnd, LONG width, LONG height, BOOL isWindow);
	void CleanD3D();
	void ResizeWindow(long width, long height);

	void SetProjection(long x, long y, double sight, long width, long height, double zNear, double zFar);

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
#endif