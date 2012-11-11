#include "stdafx.h"
#include "./CWindowBase.h"

CWindowBase::CWindowBase()
{
	m_hAttachedWnd = NULL;
	m_pOldWndProc = NULL;
	m_isDialog =	FALSE;
}

CWindowBase::~CWindowBase()
{
}

void CWindowBase::CMessageBox(const TCHAR* format, ...)
{
	va_list ap;
	va_start(ap,format);
#if _MSC_VER >= 1400
	TCHAR* str = (TCHAR*)malloc(sizeof(TCHAR) * (_vscprintf(format, ap)+1));
#else
	TCHAR str[1024];
#endif
	vsprintf(str,format,ap);
	va_end(ap);
	::MessageBox(NULL,str,"",0);
#if _MSC_VER >= 1400
	free(str);
#endif
}

void CWindowBase::CSetWindowText(HWND hWnd, const TCHAR* format, ...)
{
	va_list ap;
	va_start (ap,format);
#if _MSC_VER >= 1400
	TCHAR* str = (TCHAR*)malloc(sizeof(TCHAR) * (_vscprintf(format, ap)+1));
#else
	TCHAR str[1024];
#endif
	vsprintf(str,format,ap);
	va_end(ap);
 	::SetWindowText(hWnd, str);
#if _MSC_VER >= 1400
	free(str);
#endif
}

// ウィンドウハンドルとCWindowBaseオブジェクトを結び付ける
bool CWindowBase::Attach(HWND hWnd)
{
	if (!hWnd)	return FALSE;

	m_hAttachedWnd = hWnd;
	// ダイアログかウィンドウかを判定する
	m_isDialog =	(GetWindowLong(hWnd, DWL_DLGPROC) != 0);
	int	nIndex = m_isDialog ? DWL_DLGPROC : GWL_WNDPROC;

	// ウィンドウハンドルとCWindowBaseオブジェクトを結びつける
	SetProp(m_hAttachedWnd, _T("CWindowBase"), (HANDLE)this);

	// 既存のウィンドウをサブクラス化する場合は、ウィンドウ(ダイアログ)
	// プロシージャをBaseWndProcに置き換える
	if (GetWindowLong(m_hAttachedWnd, nIndex) != (LONG)BaseWndProc)
	{
		m_pOldWndProc = (WNDPROC)SetWindowLong(m_hAttachedWnd, nIndex, (LONG)BaseWndProc);
	}

	return TRUE;
}

// ウィンドウハンドルをCWindowBaseオブジェクトから切り離す
bool CWindowBase::Detach()
{
	if (!m_hAttachedWnd)
		return FALSE;

	// ウィンドウがサブクラス化されている場合は、ウィンドウ(ダイアログ)
	// プロシージャを元に戻す。
	if (m_pOldWndProc)
	SetWindowLong(m_hAttachedWnd, (m_isDialog ? DWL_DLGPROC : GWL_WNDPROC), (DWORD)m_pOldWndProc);

	// ウィンドウハンドルをCWindowBaseオブジェクトから切り離す
	RemoveProp(m_hAttachedWnd, _T("CWindowBase"));

	return TRUE;
}

LRESULT	CALLBACK CWindowBase::BaseWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWindowBase* pTargetWnd = (CWindowBase*)GetProp(hWnd, _T("CWindowBase"));

	if (!pTargetWnd)
	{
		if ((uMsg == WM_CREATE) || (uMsg == WM_NCCREATE))
			pTargetWnd = (CWindowBase*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		else if	( uMsg == WM_INITDIALOG	)
			pTargetWnd = (CWindowBase*)lParam;

		if (pTargetWnd)
			pTargetWnd->Attach(hWnd);
	}
	else
	{
		LRESULT	lResult	= pTargetWnd->WndProc(hWnd,	uMsg, wParam, lParam);
		if (uMsg ==	WM_DESTROY)
			pTargetWnd->Detach();
		return lResult;

		if(GetWindowLong(hWnd, DWL_DLGPROC))
			return FALSE;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
