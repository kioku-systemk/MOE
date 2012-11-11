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

// �E�B���h�E�n���h����CWindowBase�I�u�W�F�N�g�����ѕt����
bool CWindowBase::Attach(HWND hWnd)
{
	if (!hWnd)	return FALSE;

	m_hAttachedWnd = hWnd;
	// �_�C�A���O���E�B���h�E���𔻒肷��
	m_isDialog =	(GetWindowLong(hWnd, DWL_DLGPROC) != 0);
	int	nIndex = m_isDialog ? DWL_DLGPROC : GWL_WNDPROC;

	// �E�B���h�E�n���h����CWindowBase�I�u�W�F�N�g�����т���
	SetProp(m_hAttachedWnd, _T("CWindowBase"), (HANDLE)this);

	// �����̃E�B���h�E���T�u�N���X������ꍇ�́A�E�B���h�E(�_�C�A���O)
	// �v���V�[�W����BaseWndProc�ɒu��������
	if (GetWindowLong(m_hAttachedWnd, nIndex) != (LONG)BaseWndProc)
	{
		m_pOldWndProc = (WNDPROC)SetWindowLong(m_hAttachedWnd, nIndex, (LONG)BaseWndProc);
	}

	return TRUE;
}

// �E�B���h�E�n���h����CWindowBase�I�u�W�F�N�g����؂藣��
bool CWindowBase::Detach()
{
	if (!m_hAttachedWnd)
		return FALSE;

	// �E�B���h�E���T�u�N���X������Ă���ꍇ�́A�E�B���h�E(�_�C�A���O)
	// �v���V�[�W�������ɖ߂��B
	if (m_pOldWndProc)
	SetWindowLong(m_hAttachedWnd, (m_isDialog ? DWL_DLGPROC : GWL_WNDPROC), (DWORD)m_pOldWndProc);

	// �E�B���h�E�n���h����CWindowBase�I�u�W�F�N�g����؂藣��
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
