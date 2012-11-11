#include "stdafx.h"
#include "commoncontrol.h"

const char* CCommonControl::SZ_CONTROL_CLASS_NAME = "CCOMMONCONTROL_CLASS";

LRESULT CALLBACK CCommonControl::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CCommonControl* wnd = (CCommonControl*)GetProp(hWnd,"CCommonControlInfo");
	
	//this‚Ì“à—e‚ªŽæ“¾‚Å‚«‚È‚¯‚ê‚Î‚»‚Ì‚Ü‚ÜDefWindowProc‚É“n‚·B
	if(!wnd) return DefWindowProc(hWnd, msg, wParam, lParam);

	if(wnd->WndProcFuncPtr!=NULL) wnd->WndProcFuncPtr(hWnd, msg, wParam, lParam);
	
	switch(msg)
	{
		case WM_CREATE:{
			
		}break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}



CCommonControl::CCommonControl(void)
{
	my_hwnd=NULL;
	owner_hwnd=NULL;
	WndProcFuncPtr=NULL;
}
CCommonControl::~CCommonControl(void)
{

}

void CCommonControl::SetWndProcCallback(void func(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam))
{
	WndProcFuncPtr = func;
}

HWND CCommonControl::GetHWnd()
{
	return my_hwnd;
}