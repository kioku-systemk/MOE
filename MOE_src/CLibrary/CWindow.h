//CWindow class family -> CWindow
// code : c.r.v. 2005
// About   : 通常ウィンドウの作成とそのイベント処理を担う
// Version : 0.5
// Date    : 06/17/2005

#ifndef CWINDOW_H_CRV
#define CWINDOW_H_CRV

//#include <windows.h>
//#include <tchar.h>
#include "./CWindowBase.h"

//entry point
#define cmain() CMain()
#define Cmain() CMain()
#define CMain() WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)

//mouse events
#define MOUSEMOVE 0x00
#define LBUTTON    0x01
#define RBUTTON    0x02
#define MBUTTON    0x03
#define WHEEL     0x04

#define SAFE_CALLBACK(dest_func_ptr, arguments) if(dest_func_ptr)##dest_func_ptr##arguments
#define SAFE_CALLBACK_CHECK(return_val, dest_func_ptr, arguments) if(dest_func_ptr)##return_val##=##dest_func_ptr##arguments

class CWindow : public CWindowBase
{
public:
	CWindow();
	virtual ~CWindow();
public:
	/////////////////////////////////////////
	//basic functions for Window-Management//
	/////////////////////////////////////////
	//Create a window at center of the screen
	virtual bool CCreateWindow(LONG width = 0, LONG height = 0, const TCHAR* szTitle = "", const TCHAR* szMenuName = NULL, BOOL isWindow = TRUE);
	//Create a window at specified position of the screen
	virtual bool CCreateWindow(LONG x = 0, LONG y = 0, LONG width = 0, LONG height = 0, const TCHAR* szTitle = "", const TCHAR* szMenuName = NULL, BOOL isWindow = TRUE);
	//Message-loop
	virtual WPARAM CMessageLoop();
	//Key, Mouse, Render, Timer, Interval for timer, Idle
	virtual void CSetCallbackFunctions(void key(UCHAR, bool) = NULL, void mouse(long, long, int, bool) = NULL, void render(HDC) = NULL, void idle() = NULL, void timer() = NULL, long interval = 0);
	virtual void CSetHookProcedure(LRESULT CALLBACK HookProc(HWND, UINT, WPARAM, LPARAM) = NULL);
	virtual void CSetIdleFunc(void idle());

	virtual void LoadAccelerators(const TCHAR* szAccel);
	virtual HMENU GetMenu();

	virtual void ClearScreen(UCHAR r, UCHAR g, UCHAR b, UCHAR a){}
	virtual void RedrawScreen(){}

	/////////////////////////////////////////
	//additional functions for user        //
	/////////////////////////////////////////
	//Get hWnd
	HWND CGethWnd();
	//Terminate
	void CExit();
	void CExit(int exit);

	POINT* GetWindowPos();

	int GetWidth()
	{
		return m_width;
	}
	int GetHeight()
	{
		return m_height;
	}
protected:
	void (*m_pKeyFunc)(UCHAR key, bool isDown);
	void (*m_pMouseFunc)(long x, long y, int id, bool isDown);
	void (*m_pRenderFunc)(HDC hdc);
	void (*m_pIdleFunc)();
	void (*m_pTimerFunc)();
	LRESULT (CALLBACK *m_pHookProc)(HWND, UINT, WPARAM, LPARAM);

	HACCEL m_hAccel;
	HWND m_hWnd;
	bool m_isFullscreen;

	int m_width;
	int m_height;

	POINT m_lastpos;
protected:
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
#endif