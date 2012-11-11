#ifndef CTREEVIEW_INCLUDED
#define CTREEVIEW_INCLUDED

#include "./CWindowBase.h"
#include <commctrl.h>
#include <zmouse.h>
#pragma comment(lib, "comctl32")

#define SAFE_CALLBACK(dest_func_ptr, arguments) if(dest_func_ptr)##dest_func_ptr##arguments
#define SAFE_CALLBACK_CHECK(return_val, dest_func_ptr, arguments) if(dest_func_ptr)##return_val##=##dest_func_ptr##arguments

#define ExternalToInternal(eval) (eval-1)
#define InternalToExternal(ival) (ival+1)

class CTreeView : public CWindowBase
{
public:
	CTreeView();
	~CTreeView();
public:
	void CreateTreeView(int x, int y, int width, int height, const TCHAR* szTreeTitle, HWND hParentWnd = NULL);

	int AddItem(const TCHAR* szObjName, int nDest, bool isSorting = false);
	int DeleteItem(int nDest);
	void DeleteAllItem();

	int GetAmount();
	int GetItemFromPos(long x, long y);
	int GetSelectedItem();
	TCHAR* GetItemName(int nDest);
	int GetItemIndex(HTREEITEM hItem);
	int GetItemIndex(int nDest);
	int GetItemNum();

	void EnsureVisible(int nDest);
	void ExpandItem(int nDest, bool isExpand);
	bool IsExpand(int nDest);
	int IsChecked(int nDest);
	void SetCheck(int nDest, bool isChecked = true);



	void SetItemName(const TCHAR* szNewName, int nDest);

	void SetCallbackFunctions(int OnNameEdit(int, const TCHAR*, bool) = NULL, void OnPopupRequest(HWND, WPARAM, LPARAM) = NULL, void OnWMCommand(HWND, WPARAM, LPARAM) = NULL, int OnKeyboardEvent(UCHAR key, bool isDown) = NULL, int OnMouseEvent(long x, long y, UINT btn) = NULL, void OnSelectItem(int, bool, int, bool) = NULL);
	void SetHookProcedure(LRESULT CALLBACK HookProc(HWND, UINT, WPARAM, LPARAM) = NULL);

	HWND GetHolderWnd(){ return m_hTreeViewHolderWnd; }
	HWND GetTreeWnd(){ return m_hTreeViewWnd; }
	HWND SetParent(HWND hNewParent);

	void LoadAccelerators(const TCHAR* szAccel);
	int SelectItem(int nDest);

protected:
	HWND m_hParentWnd;
	HWND m_hTreeViewHolderWnd;
	HWND m_hTreeViewWnd;
	HTREEITEM* m_hItemList;
	static int m_nItemListAllocateSize;

	//0î‘ÇÕÉãÅ[ÉgÇéwÇ∑
	int m_nTreeItemNo;
	DWORD m_dwTreeId;
	static DWORD s_dwTreeId;

	int (*m_pNameEditFunc)(int, const TCHAR*, bool);
	void (*m_pPopupFunc)(HWND, WPARAM, LPARAM);
	void (*m_pCommandFunc)(HWND, WPARAM, LPARAM);
	int (*m_pKeyboardFunc)(UCHAR, bool);
	int (*m_pMouseFunc)(long, long, UINT);
	void (*m_pOnSelectItem)(int, bool, int, bool);
	LRESULT (CALLBACK *m_pHookProc)(HWND, UINT, WPARAM, LPARAM);
	WNDPROC m_TreeViewWndProc;
	WNDPROC m_TreeViewOldWndProc;

	HACCEL m_hAccel;
protected:
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//	LRESULT CALLBACK TreeViewWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK TreeViewWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
#endif