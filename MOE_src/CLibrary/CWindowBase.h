//CWindow class family -> CWindowBase
// code : c.r.v. 2005
// About   : CWindowファミリーすべての根底を成す、純仮想関数を持つクラス
// Version : 0.3
// Thanks  : Belution.com
// Date    : unknown

#ifndef CWINDOWBASE_H_CRV
#define CWINDOWBASE_H_CRV

//THE SHITTY VC 8.0(Visual Studio 2005) doesn't involve default libraries for Win32 App solution!
#if _MSC_VER >= 1400	//VC 8.0
#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "Ole32")
#pragma comment(lib, "OlePro32")
#endif
#pragma comment(linker, "/subsystem:windows")

#include <windows.h>
#include <zmouse.h>
#include <tchar.h>
#include <stdio.h>

class CWindowBase
{
public:
	CWindowBase();
	virtual ~CWindowBase();
public:
	/////////////////////////////////////////
	//utils
	/////////////////////////////////////////
	void CMessageBox(const TCHAR* format, ...);
	void CSetWindowText(HWND hWnd, const TCHAR* format, ...);


private:
	HWND m_hAttachedWnd;
	WNDPROC m_pOldWndProc;
	bool m_isDialog;

protected:
	//regist
	bool Attach(HWND hWnd);
	bool Detach();

	//Window Procedure
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	static LRESULT CALLBACK BaseWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
#endif