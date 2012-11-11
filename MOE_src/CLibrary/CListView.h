#ifndef CLISTVIEW_INCLUDED
#define CLISTVIEW_INCLUDED

#include "./CWindowBase.h"
#include <commctrl.h>
#pragma comment(lib, "comctl32")

#define SAFE_CALLBACK(dest_func_ptr, arguments) if(dest_func_ptr)##dest_func_ptr##arguments
#define SAFE_CALLBACK_CHECK(return_val, dest_func_ptr, arguments) if(dest_func_ptr)##return_val##=##dest_func_ptr##arguments

class CListView : public CWindowBase
{
public:
	CListView();
	~CListView();
public:
	void CreateListView(int x, int y, int width, int height, const TCHAR* szTreeTitle, HWND hParentWnd = NULL);

	int AddItem(const TCHAR* szObjName, int nDest, bool isSorting = false);
	int DeleteItem(int nDest);
	void DeleteAllItem();

	void AddColumn(const char* szColName, int width, int nDest);

	int GetItemCount();
	int GetSelectedItem();
	int SelectItem(int nDest);
	TCHAR* GetItemName(int nDest);
	int GetItemFromPos(long x, long y);

	int IsChecked(int nDest);
	void SetCheck(int nDest, bool isChecked = true);

	void SetCallbackFunctions(int OnNameEdit(int, const TCHAR*, bool) = NULL, void OnPopupRequest(HWND, WPARAM, LPARAM) = NULL, void OnWMCommand(HWND, WPARAM, LPARAM) = NULL, int OnKeyboardEvent(UCHAR key, bool isDown) = NULL, int OnMouseEvent(long x, long y, UINT btn) = NULL, void OnChangeNotify(int index, bool isByMouse)=NULL);
	void SetHookProcedure(LRESULT CALLBACK HookProc(HWND, UINT, WPARAM, LPARAM) = NULL);

	HWND GetHolderWnd(){ return m_hListViewHolderWnd; }
	HWND GetListWnd(){ return m_hListViewWnd; }
	HWND SetParent(HWND hNewParent);

	void EnsureVisible(int n);
	void AddSubItem(int nDest, int nItemNo, const char* text);

protected:
	HWND m_hParentWnd;
	HWND m_hListViewHolderWnd;
	HWND m_hListViewWnd;
	int* m_nItemList;
	bool* m_nSortSubItemIsUpward;
	static int m_nItemListAllocateSize;

	int m_nListItemNo;
	DWORD m_dwListId;
	static DWORD s_dwListId;

	int (*m_pNameEditFunc)(int, const TCHAR*, bool);
	void (*m_pPopupFunc)(HWND, WPARAM, LPARAM);
	void (*m_pCommandFunc)(HWND, WPARAM, LPARAM);
	int (*m_pKeyboardFunc)(UCHAR, bool);
	int (*m_pMouseFunc)(long, long, UINT);
	void (*m_pChangeNorify)(int, bool);
	LRESULT (CALLBACK *m_pHookProc)(HWND, UINT, WPARAM, LPARAM);

	WNDPROC m_ListViewWndProc;
	WNDPROC m_ListViewOldWndProc;
protected:
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static int CALLBACK SortItemsProc(LPARAM lp1, LPARAM lp2, LPARAM lp3);
	static LRESULT CALLBACK ListViewWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
#endif