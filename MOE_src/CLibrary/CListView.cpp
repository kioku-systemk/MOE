#include "stdafx.h"
#include "./CListView.h"

DWORD CListView::s_dwListId = 0;
int CListView::m_nItemListAllocateSize = 1024;

CListView::CListView()
{
	m_hListViewHolderWnd	= NULL;
	m_hListViewWnd			= NULL;
	m_nItemList = (int*)malloc(CListView::m_nItemListAllocateSize * sizeof(int));
	int i;
	for(i=0; i<CListView::m_nItemListAllocateSize; i++)
	{
		m_nItemList[i] = -1;
	}
	//m_nSortSubItemIsUpward = (bool*)malloc(256 * sizeof(bool));

	m_nListItemNo = -1;
	m_dwListId = ++s_dwListId;
	
	m_hParentWnd = NULL;

	m_ListViewOldWndProc = m_ListViewWndProc = NULL;
}

CListView::~CListView()
{
	s_dwListId--;
	free(m_nItemList);
	free(m_nSortSubItemIsUpward);
}

void CListView::SetCallbackFunctions(int OnNameEdit(int, const TCHAR*, bool), void OnPopupRequest(HWND, WPARAM, LPARAM), void OnWMCommand(HWND, WPARAM, LPARAM), int OnKeyboardEvent(UCHAR key, bool isDown), int OnMouseEvent(long x, long y, UINT btn), void OnChangeNotify(int index, bool isByMouse))
{
	m_pNameEditFunc = OnNameEdit;
	m_pPopupFunc = OnPopupRequest;
	m_pCommandFunc = OnWMCommand;
	m_pKeyboardFunc = OnKeyboardEvent;
	m_pMouseFunc = OnMouseEvent;
	m_pChangeNorify = OnChangeNotify;
}

void CListView::SetHookProcedure(LRESULT CALLBACK HookProc(HWND, UINT, WPARAM, LPARAM))
{
	m_pHookProc = HookProc;
}

void CListView::SetCheck(int nDest, bool isChecked)
{
	if(nDest<0 || nDest>m_nListItemNo) return;
	ListView_SetCheckState(m_hListViewWnd, nDest, isChecked);
}

int CListView::IsChecked(int nDest)
{
	if(nDest<0 || nDest>m_nListItemNo) return FALSE;
	return ListView_GetCheckState(m_hListViewWnd, nDest);
}

int CListView::GetItemFromPos(long x, long y)
{
	LVHITTESTINFO ht;
	ZeroMemory(&ht, sizeof(TVHITTESTINFO));
	ht.pt.x = x;
	ht.pt.y = y;
	ListView_HitTest(m_hListViewWnd, &ht);
	return ht.iItem;
}

LRESULT CALLBACK CListView::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static LV_DISPINFO* pLvdi;
	static HWND hEdit;
	static char str[512];

	if(m_pHookProc != NULL)
	{
		int ret = 0;
		ret = m_pHookProc(hWnd, msg, wParam, lParam);
		if(ret!=TRUE) return ret;
	}

	RECT rect;
	switch(msg)
	{
	case WM_CREATE:
		InitCommonControls();
		break;
	case WM_SIZE:
		GetClientRect(hWnd, &rect);
		//リストビューをウィンドウにフィットさせる
		MoveWindow(m_hListViewWnd, 0, 0, rect.right, rect.bottom, TRUE);
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		return 0;
	case WM_NOTIFY:
		//自分のリストIDかどうか
		if(wParam == m_dwListId)
		{
			NMHDR* pHdr = (NMHDR*)lParam;
			
			switch(pHdr->code)
			{
				case NM_CLICK:
				{
					NMCLICK* pClick = (NMCLICK*)lParam;

					int ret = 0;
					POINT dpt;// = pClick->pt;
					GetCursorPos(&dpt);
//					ScreenToClient(m_hListViewHolderWnd, &dpt);
					SAFE_CALLBACK_CHECK(ret, m_pMouseFunc, (dpt.x, dpt.y, (GetAsyncKeyState(VK_LBUTTON)) ? WM_LBUTTONDOWN : WM_LBUTTONUP));
					if(ret!=0)//TRUE
					{
//						dpt = pClick->pt;//再代入が必要
						GetCursorPos(&dpt);
						POINT dpt = pClick->pt;
//						ScreenToClient(m_hParentWnd, &dpt);
						if(m_hParentWnd)
							CallWindowProcA((WNDPROC)GetWindowLong(m_hParentWnd, GWL_WNDPROC), m_hParentWnd, (GetAsyncKeyState(VK_RBUTTON)) ? WM_LBUTTONDOWN : WM_LBUTTONUP, (WPARAM)0, (LPARAM)MAKEWORD(dpt.x, dpt.y));
					}
					break;
				}
				case NM_DBLCLK:
				{
					NMCLICK* pClick = (NMCLICK*)lParam;

					int ret = 0;
					POINT dpt;// = pClick->pt;
					GetCursorPos(&dpt);
					ScreenToClient(m_hListViewHolderWnd, &dpt);
					SAFE_CALLBACK_CHECK(ret, m_pMouseFunc, (dpt.x, dpt.y, WM_LBUTTONDBLCLK));
					if(ret!=0)//TRUE
					{
//						dpt = pClick->pt;//再代入が必要
						GetCursorPos(&dpt);
						POINT dpt = pClick->pt;
//						ScreenToClient(m_hParentWnd, &dpt);
						if(m_hParentWnd)
							CallWindowProcA((WNDPROC)GetWindowLong(m_hParentWnd, GWL_WNDPROC), m_hParentWnd, WM_LBUTTONDBLCLK, (WPARAM)0, (LPARAM)MAKEWORD(dpt.x, dpt.y));
					}
					break;
				}
				case NM_RCLICK:
				{
					NMCLICK* pClick = (NMCLICK*)lParam;

					int ret = 0;
					POINT dpt;// = pClick->pt;
					GetCursorPos(&dpt);
//					ScreenToClient(m_hListViewHolderWnd, &dpt);
					SAFE_CALLBACK_CHECK(ret, m_pMouseFunc, (dpt.x, dpt.y, ((GetAsyncKeyState(VK_RBUTTON)&0x8000) == 0x8000) ? WM_RBUTTONDOWN : WM_RBUTTONUP));
					if(ret!=0)//TRUE
					{
//						dpt = pClick->pt;//再代入が必要
						GetCursorPos(&dpt);
						POINT dpt = pClick->pt;
//						ScreenToClient(m_hParentWnd, &dpt);
						if(m_hParentWnd)
							CallWindowProcA((WNDPROC)GetWindowLong(m_hParentWnd, GWL_WNDPROC), m_hParentWnd, (GetAsyncKeyState(VK_RBUTTON)) ? WM_RBUTTONDOWN : WM_RBUTTONUP, (WPARAM)0, (LPARAM)MAKEWORD(dpt.x, dpt.y));
					}
					break;
				}
				case NM_KEYDOWN:
				{
					NMKEY* pKey = (NMKEY*)lParam;
					CallWindowProcA((WNDPROC)GetWindowLong(m_hParentWnd, GWL_WNDPROC), m_hParentWnd, WM_KEYDOWN, (WPARAM)pKey->nVKey, (LPARAM)pKey->uFlags);
					break;
				}
				case LVN_BEGINLABELEDIT:
				{
					hEdit = ListView_GetEditControl(m_hListViewWnd);
					GetWindowText(hEdit, str, sizeof(str));
					int ret;
					SAFE_CALLBACK_CHECK(ret, m_pNameEditFunc, (GetSelectedItem(), str, TRUE));
					if(ret == TRUE)
					{
						pLvdi = (LV_DISPINFO*)lParam;
						ListView_SetItemText(m_hListViewWnd, pLvdi->item.iItem, 0, str);
						return FALSE; //FALSE == let user to change value.
					}
					return TRUE; //prevent user from changing value.
				}
				case LVN_ENDLABELEDIT:
				{
					GetWindowText(hEdit, str, sizeof(str));
					int ret;
					SAFE_CALLBACK_CHECK(ret, m_pNameEditFunc, (GetSelectedItem(), str, FALSE));
					if(ret == TRUE)
					{
						pLvdi = (LV_DISPINFO*)lParam;
						pLvdi->item.pszText = str;
						ListView_SetItemText(m_hListViewWnd, pLvdi->item.iItem, 0, str);
						return TRUE;
					}
					return FALSE; //If the pszText member of the LVITEM structure is non-NULL, return TRUE to set the item's label to the edited text. Return FALSE to reject the edited text and revert to the original label. 
				}
				case LVN_KEYDOWN:
				{
					NMLVKEYDOWN* pLvkd = (NMLVKEYDOWN*)lParam;

					int ret;
					//SAFE_CALLBACK_CHECK(ret, m_pKeyboardFunc, ((UCHAR)pLvkd->wVKey, (bool)(GetKeyState(pTvkd->wVKey) & 0x80)));
					SAFE_CALLBACK_CHECK(ret, m_pKeyboardFunc, ((UCHAR)pLvkd->wVKey, TRUE));
					if(ret!=0)//TRUE
					{
						if(m_hParentWnd)
							CallWindowProcA((WNDPROC)GetWindowLong(m_hParentWnd, GWL_WNDPROC), m_hParentWnd, WM_KEYDOWN, (WPARAM)pLvkd->wVKey, (LPARAM)pLvkd->flags);
					}
					break;
				}
				case LVN_COLUMNCLICK:
				{
					//NM_LISTVIEW *pNMLV;
					//pNMLV = (NM_LISTVIEW*)lParam;
					//if(m_nSortSubItemIsUpward[pNMLV->iSubItem])
					//	m_nSortSubItemIsUpward[pNMLV->iSubItem] = false;
					//else
					//	m_nSortSubItemIsUpward[pNMLV->iSubItem] = true;
					//ListView_SortItems(m_hListViewWnd, SortItemsProc, pNMLV->iSubItem);
					//break;
				}
				case LVN_ITEMCHANGING:
				case LVN_ITEMCHANGED:
				{
					NMLISTVIEW* nLv = (NM_LISTVIEW*)lParam;
					//SAFE_CALLBACK(m_pChangeNorify, (nLv->iItem, GetAsyncKeyState(VK_LBUTTON)));
					SAFE_CALLBACK(m_pChangeNorify, (nLv->iItem, (GetAsyncKeyState(VK_LBUTTON)!=0) ? true:false));
					break;
				}
			}
		}
		break;
    case WM_CONTEXTMENU:
		SAFE_CALLBACK(m_pPopupFunc, (hWnd, wParam, lParam));
        break;
    case WM_COMMAND:
		SAFE_CALLBACK(m_pCommandFunc, (hWnd, wParam, lParam));
        break;
/*
	case WM_KEYDOWN:
	case WM_KEYUP:
		CallWindowProc((WNDPROC)GetWindowLong(m_hParentWnd, GWL_WNDPROC), m_hParentWnd, msg, wParam, lParam);
		break;
*/
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int CALLBACK CListView::SortItemsProc(LPARAM lp1, LPARAM lp2, LPARAM lp3)
{
/*
//	static HWND hList = ;
	static LV_FINDINFO lvf;
	static int nItem1, nItem2;
	static char buf1[30], buf2[30];

	lvf.flags = LVFI_PARAM;
	lvf.lParam = lp1;
	nItem1 = ListView_FindItem(hList, -1, &lvf);

	lvf.lParam = lp2;
	nItem2 = ListView_FindItem(hList, -1, &lvf);
	
	ListView_GetItemText(hList, nItem1, (int)lp3, buf1, sizeof(buf1));
	ListView_GetItemText(hList, nItem2, (int)lp3, buf2, sizeof(buf2));
//	if (sortsubno[(int)lp3])
		return(strcmp(buf1, buf2));
	else
		return(strcmp(buf1, buf2) * -1);
*/
	return -1;
}

HWND CListView::SetParent(HWND hNewParent)
{
	m_hParentWnd = hNewParent;
	return ::SetParent(m_hListViewHolderWnd, hNewParent);
}

void CListView::CreateListView(int x, int y, int width, int height, const TCHAR* szTreeTitle, HWND hParentWnd)
{
	m_hParentWnd = hParentWnd;
	
	//リストビューを搭載するウィンドウを作る
	WNDCLASSEX wc;
	if(!GetClassInfoEx(GetModuleHandle(NULL), _T("CListView"), &wc))
	{
		memset(&wc, 0, sizeof(WNDCLASSEX));
		wc.cbSize			= sizeof(WNDCLASSEX);
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)CWindowBase::BaseWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= _T("CListView");
		wc.hIconSm			= LoadIcon(NULL, IDI_APPLICATION);

		if(!RegisterClassEx(&wc))
		{
			ExitProcess(0);
			return;
		}
	}
	
	//TIPS: CreateWindowでは小さく作っておく(サイズを適当に-2しておく)
	if(!(m_hListViewHolderWnd=CreateWindowEx(
							//(!hParentWnd) ? 0 : WS_EX_TOOLWINDOW,
							WS_EX_TOOLWINDOW,
							_T("CListView"),
							szTreeTitle,
							(!hParentWnd) ? (WS_OVERLAPPEDWINDOW) : (WS_CHILD | WS_VISIBLE | WS_THICKFRAME | WS_CAPTION),
							x, y, width-2, height-2,
							hParentWnd,
							NULL,
							GetModuleHandle(NULL),
							(void*)this)))
	{
		ExitProcess(0);
		return;
	}
	//m_hParentWnd = GetParent(m_hListViewHolderWnd);
	ShowWindow(m_hListViewHolderWnd, SW_SHOWNORMAL);
	UpdateWindow(m_hListViewHolderWnd);

	//リストビューそのものの生成
	m_hListViewWnd = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
					WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL| 
					LVS_REPORT | LVS_EDITLABELS|  LVS_SHOWSELALWAYS | LVS_SINGLESEL,
					0, 0, 10, 10,
					m_hListViewHolderWnd, (HMENU)m_dwListId, GetModuleHandle(NULL), NULL);
	m_ListViewOldWndProc = (WNDPROC)GetWindowLong(m_hListViewWnd, GWL_WNDPROC);
	SetProp(m_hListViewWnd, "CListView", this);
	m_ListViewWndProc = ListViewWndProc;
	SetWindowLong(m_hListViewWnd, GWL_WNDPROC, (LONG)m_ListViewWndProc);

	//ツリービューを親ウィンドウにフィットさせる
	RECT rect;
	//貼り付け先のウィンドウの矩形をGetする
	GetWindowRect(m_hListViewHolderWnd, &rect);
	//まず、ツリービューを貼り付ける
	MoveWindow(m_hListViewWnd, 0, 0, rect.right, rect.bottom, FALSE);
	//最後に、親ウィンドウを指定された大きさに変更することで大きさを合わせる
	//これを怠ると、Vスクロールバーがうまく表示されない現象が生じる
	MoveWindow(m_hListViewHolderWnd, x, y, width, height, TRUE);
}

int CListView::GetItemCount()
{
	return ListView_GetItemCount(m_hListViewWnd);
}

LRESULT CALLBACK CListView::ListViewWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CListView* lv = (CListView*)GetProp(hWnd, "CListView");
	if(!lv)	return DefWindowProc(hWnd, msg, wParam, lParam);

	switch(msg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
	{
		UINT iKeys[] = { VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_F2 };
		for(int i=0; i<5; i++)
		{
			if(wParam == iKeys[i])
			{
				return CallWindowProc(lv->m_ListViewOldWndProc, lv->m_hListViewWnd, msg, wParam, lParam);
			}
		}
		return CallWindowProc((WNDPROC)GetWindowLong(lv->m_hParentWnd, GWL_WNDPROC), lv->m_hParentWnd, msg, wParam, lParam);
	}
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDOWN:
	{
		int ret = 0;
		SAFE_CALLBACK_CHECK(ret, lv->m_pMouseFunc, (LOWORD(lParam), HIWORD(lParam), msg));
		if(ret!=0){
		//if((GetAsyncKeyState(VK_LBUTTON)&0x8000) != 0x8000)
			CallWindowProc((WNDPROC)GetWindowLong(lv->m_hParentWnd, GWL_WNDPROC), lv->m_hParentWnd, msg, wParam, lParam);
		}
		break;
	}
	}
	return CallWindowProc(lv->m_ListViewOldWndProc, lv->m_hListViewWnd, msg, wParam, lParam);
}

int CListView::SelectItem(int nDest){
	SetFocus(m_hListViewWnd);
	ListView_SetItemState(m_hListViewWnd, nDest, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);

	//ListView_SetItemState(m_hListViewWnd, nDest, LVIS_SELECTED, LVIS_SELECTED);
	//ListView_EnsureVisible(m_hListViewWnd, m_nItemList[nDest-1], TRUE);
	return nDest;
}

int CListView::GetSelectedItem()
{
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));
	while(1)
	{
		item.iItem = ListView_GetNextItem(m_hListViewWnd, -1, LVNI_ALL | LVNI_SELECTED);
		if(item.iItem == -1) return -1;
		else break;
	}
	return item.iItem;
}

void CListView::AddColumn(const char* szColName, int width,int nDest)
{
	HWND hList = m_hListViewWnd;
	LV_COLUMN lvcol;
	ZeroMemory(&lvcol, sizeof(LV_COLUMN));

	lvcol.mask = LVCF_FMT | LVCF_TEXT | LVCF_ORDER;
	lvcol.cx = 0;
	
	if(width > 0)
	{
		lvcol.mask |= LVCF_WIDTH;
		lvcol.cx = width;		
	}

	lvcol.fmt = LVCFMT_LEFT;
	lvcol.pszText = (TCHAR*)malloc(sizeof(TCHAR) * (lstrlen(szColName)+1));
	lstrcpy(lvcol.pszText, szColName);
	lvcol.iOrder = nDest;
	lvcol.iSubItem = 0;
    ListView_InsertColumn(hList, nDest, &lvcol);
	free(lvcol.pszText);
	ListView_SetColumnWidth(hList, nDest, LVSCW_AUTOSIZE_USEHEADER);
}

int CListView::AddItem(const TCHAR* szObjName, int nDest, bool isSorting)
{
	HWND hList = m_hListViewWnd;
	HTREEITEM hItem = NULL;
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	if(nDest < 0)
	{
		while(1)
		{
			item.iItem = GetSelectedItem();
			//見つからなかったら
			if(item.iItem == -1)
			{
				//末尾につけちゃる
				item.iItem = ListView_GetItemCount(hList);
				if(item.iItem == -1)
					return -1;
				else break;
			}
			else break;
		}
	}else{
		if(nDest >= CListView::m_nItemListAllocateSize)
			return -1;
//		if(m_nItemList[nDest] == 0)
		{
			item.iItem = nDest;
			m_nItemList[nDest] = 1;
		}
//		else return -1;
	}

	//配列は0から始まっているので、-1しなくちゃだめ
	//if(++m_nListItemNo >= CListView::m_nItemListAllocateSize-1)
	//{
	//	m_nItemList = (int*)realloc(&m_nItemList, sizeof(int) * CListView::m_nItemListAllocateSize);
	//	CListView::m_nItemListAllocateSize *=2;
	//}
	++m_nListItemNo;

    item.mask = LVIF_TEXT;
    item.iSubItem = 0;
	if(szObjName == NULL){
		char str[64];
		wsprintf(str, "%d",	m_nListItemNo);
		item.pszText = (TCHAR*)malloc(sizeof(TCHAR) * (lstrlen(str)+1));
		lstrcpy(item.pszText, str);
	}else{
		item.pszText = (TCHAR*)malloc(sizeof(TCHAR) * (lstrlen(szObjName)+1));
		lstrcpy(item.pszText, szObjName);
	}	
	
	m_nItemList[m_nListItemNo] = ListView_InsertItem(hList, &item);
	ListView_EnsureVisible(hList, m_nItemList[m_nListItemNo], TRUE);
	free(item.pszText);
	ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);
	return m_nListItemNo;
}

void CListView::EnsureVisible(int n)
{
	SelectItem(n);
	ListView_EnsureVisible(m_hListViewWnd, n, TRUE);
}

void CListView::AddSubItem(int nDest, int nItemNo, const char* text){
	ListView_SetItemText(m_hListViewWnd, nDest, nItemNo, (char*)text);
	ListView_SetColumnWidth(m_hListViewWnd, nDest, LVSCW_AUTOSIZE);
}

void CListView::DeleteAllItem()
{
	for(int i=0; i<CListView::m_nItemListAllocateSize; i++)
	{
		if(m_nItemList[i] != -1)
		{
			m_nItemList[i] = -1;
		}
		//ListView_DeleteItem(m_hListViewWnd, i);
	}
	if(m_nListItemNo > -1)
		ListView_DeleteAllItems(m_hListViewWnd);
	m_nListItemNo = -1;
}

int CListView::DeleteItem(int nDest)
{
	HWND hList = m_hListViewWnd;
	HTREEITEM hItem = NULL;
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	if(nDest < 0)
	{
		item.iItem = GetSelectedItem();
	}else{
		if(nDest >= CListView::m_nItemListAllocateSize)
			return -1;
		if(m_nItemList[nDest] != 0)
		{
			item.iItem = nDest;
			m_nItemList[nDest] = 0;
		}
	}

	ListView_DeleteItem(hList, item.iItem);
	m_nListItemNo--;

	//成功したら、削除されたハンドルを返す
	return nDest;
}

TCHAR* CListView::GetItemName(int nDest)
{
	HWND hList = m_hListViewWnd;
	static TCHAR szItemName[512];
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	if(nDest < 0)
	{
		item.iItem = GetSelectedItem();
//			ListView_DeleteItem(hList, nItem);
	}else{
		if(nDest >= CListView::m_nItemListAllocateSize)
			return "";
//		if(m_nItemList[nDest] != 0)
//		{
			item.iItem = nDest;
//		}
//		else return "";
	}

	item.mask = LVIF_TEXT;
	item.pszText = &szItemName[0];
	item.cchTextMax = 512;
	ListView_GetItem(hList, &item);

	return szItemName;
}

