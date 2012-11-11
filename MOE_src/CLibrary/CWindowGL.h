//CWindow class family -> CWindowGL
// code : c.r.v. 2005
// About   : OpenGL用ウィンドウの作成とそのイベント処理を担う(複数ウィンドウには対応しない
// Version : 0.6
// Date    : 06/17/2005

#ifndef CWINDOWGL_H_CRV
#define CWINDOWGL_H_CRV

#include "./CWindow.h"
#include <gl\gl.h>
#include <gl\glu.h>

#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "OpenGL32.lib")

class CWindowGL : public CWindow
{
public:
	CWindowGL();
	virtual ~CWindowGL();
public:
	///////////////////////////////////////////////////
	//basic functions for Window-Management in OpenGL//
	///////////////////////////////////////////////////
	//Create a window at center of the screen
	bool CCreateWindow(LONG width = 0, LONG height = 0, const TCHAR* szTitle = "", const TCHAR* szMenuName = NULL, BOOL isWindow = TRUE);
	//Create a window at specified position of the screen
	bool CCreateWindow(LONG x = 0, LONG y = 0, LONG width = 0, LONG height = 0, const TCHAR* szTitle = "", const TCHAR* szMenuName = NULL, BOOL isWindow = TRUE);
	//WPARAM CMessageLoop();

	/////////////////////////////////////////
	//Basic controls for OpenGL            //
	/////////////////////////////////////////
	void SetProjection(long x, long y, double sight, long width, long height, double zNear, double zFar);

	/////////////////////////////////////////
	//Additional controls for OpenGL       //
	/////////////////////////////////////////
	bool EnableVsync(bool isvsync = true);

	void ClearScreen(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f);
	void RedrawScreen();

	HGLRC GetRC(){ return m_hRC; }
	HDC GetDC(){ return m_hDC; }

	void ReCreateWindow();
protected:
	HDC m_hDC;
	HGLRC m_hRC;
	bool m_isActive;
	POINT m_lastsize;

protected:
	///////////////////////////////////////////////////
	//basic functions for Window-Management in OpenGL//
	///////////////////////////////////////////////////
	//Release handles and close the window
	void KillGLWindow();
	//Resize window and re-init.
	void ResizeGLWindow(long width, long height);
	
	/////////////////////////////////////////
	//Basic controls for OpenGL            //
	/////////////////////////////////////////
	bool InitGL();

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
#endif