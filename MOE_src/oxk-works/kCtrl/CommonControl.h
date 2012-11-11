/*
	Common Control
	2005/6/19 coded by kioku
*/
#pragma once
#include <windows.h>

class CCommonControl
{
	private:
		static const char* SZ_CONTROL_CLASS_NAME;
		
	protected:
		HWND my_hwnd, owner_hwnd;
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void (*WndProcFuncPtr)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);//コールバックフック関数のポインタ
		virtual void OnControlDraw(HDC hdc)=0;//自前描画
	public:
		CCommonControl(void);
		~CCommonControl(void);
		virtual void Create(HWND OwnerhWnd, int x, int y, int width, int height, const char* szTitle)=0;
		void SetWndProcCallback(void func(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam));
		HWND GetHWnd();
};
