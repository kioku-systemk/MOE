#include "stdafx.h"
#include "./CTreeView.h"

DWORD CTreeView::s_dwTreeId = 0;
int CTreeView::m_nItemListAllocateSize = 1024;


CTreeView::CTreeView()
{
	m_hParentWnd			= NULL;
	m_hTreeViewHolderWnd	= NULL;
	m_hTreeViewWnd			= NULL;
	m_hItemList = (HTREEITEM*)calloc(CTreeView::m_nItemListAllocateSize, sizeof(HTREEITEM));
	m_nTreeItemNo = -1;
	m_dwTreeId = ++s_dwTreeId;

	m_hAccel = NULL;
}

CTreeView::~CTreeView()
{
	s_dwTreeId--;
	free(m_hItemList);
}

void CTreeView::LoadAccelerators(const TCHAR* szAccel)
{
	m_hAccel = ::LoadAccelerators(GetModuleHandle(NULL), szAccel);
}

void CTreeView::SetHookProcedure(LRESULT CALLBACK HookProc(HWND, UINT, WPARAM, LPARAM))
{
	m_pHookProc = HookProc;
}

void CTreeView::SetCheck(int nDest, bool isChecked)
{
	if(ExternalToInternal(nDest)<0 || ExternalToInternal(nDest)>m_nTreeItemNo) return;
	HTREEITEM hItem = m_hItemList[ExternalToInternal(nDest)];
	TreeView_SetCheckState(m_hTreeViewWnd, hItem, isChecked);
}

int CTreeView::IsChecked(int nDest)
{
    TVITEM tvItem;
	if(ExternalToInternal(nDest)<0) return FALSE;
	HTREEITEM hItem = m_hItemList[ExternalToInternal(nDest)];
    char szStr[256];

    memset(&tvItem, 0, sizeof(TVITEM));

    //hItem = TreeView_GetRoot(hTree);

    tvItem.mask = TVIF_HANDLE | TVIF_STATE | TVIF_TEXT;
    tvItem.hItem = hItem;
    tvItem.stateMask = TVIS_STATEIMAGEMASK;
    tvItem.pszText = szStr;
    tvItem.cchTextMax = 256;

    //TreeView_Expand(hTree, hItem, TVE_EXPAND);

    
//    while (1) {
	TreeView_GetItem(m_hTreeViewWnd, &tvItem);
        if((BOOL)(tvItem.state >> 12) -1) {
			//TreeView_EnsureVisible(m_hTreeViewWnd, hItem);
            //MessageBox(hTree, szStr, "OK", MB_OK);
			return TRUE;
        }
//        hItem = TreeView_GetNextItem(hTree, hItem, TVGN_NEXTVISIBLE);
        //TreeView_Expand(hTree, hItem, TVE_EXPAND);
  //      tvItem.hItem = hItem;
     //   if (hItem == NULL)
    //       return;
//    }
    return FALSE;
}

LRESULT CALLBACK CTreeView::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static TV_DISPINFO* pTvdi;

	if(m_pHookProc != NULL)
	{
		int ret = 0;
		ret = m_pHookProc(hWnd, msg, wParam, lParam);
		if(ret!=TRUE) return ret;
	}

	switch(msg)
	{
	case WM_CREATE:
		InitCommonControls();
		break;
	case WM_SIZE:
		//ツリービューをウィンドウにフィットさせる
		MoveWindow(m_hTreeViewWnd, 0, 0, LOWORD(lParam), HIWORD(lParam), FALSE);
		//MoveWindow(m_hTreeViewWnd, 0, 0, LOWORD(lParam), HIWORD(lParam)+GetSystemMetrics(SM_CYMENU), FALSE);
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		return 0;//PostQuitMessage(0);
		//break;
	case WM_NOTIFY:
		//自分のツリーIDかどうか
		if(wParam == m_dwTreeId)
		{
			NMHDR* pHdr = (NMHDR*)lParam;
			
			switch(pHdr->code)
			{
//				case NM_HOVER:
//				{
//					NMMOUSE* pMouse = (NMMOUSE*)lParam;
//
//					int ret = 0;
//					POINT dpt;// = pClick->pt;
//					GetCursorPos(&dpt);
////					ScreenToClient(m_hListViewHolderWnd, &dpt);
//					SAFE_CALLBACK_CHECK(ret, m_pMouseFunc, (dpt.x, dpt.y, (GetAsyncKeyState(VK_LBUTTON)) ? WM_LBUTTONDOWN : WM_LBUTTONUP));
//					if(ret!=0)//TRUE
//					{
////						dpt = pClick->pt;//再代入が必要
//						GetCursorPos(&dpt);
//						POINT dpt = pMouse->pt;
////						ScreenToClient(m_hParentWnd, &dpt);
//						if(m_hParentWnd)
//							CallWindowProcA((WNDPROC)GetWindowLong(m_hParentWnd, GWL_WNDPROC), m_hParentWnd, (GetAsyncKeyState(VK_RBUTTON)) ? WM_LBUTTONDOWN : WM_LBUTTONUP, (WPARAM)0, (LPARAM)MAKEWORD(dpt.x, dpt.y));
//					}
//					break;
//				}
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
				case WM_DROPFILES:
				{
					int a;
					a=20;
					break;
				}
				case NM_KEYDOWN:
				{
//					NMKEY* pKey = (NMKEY*)lParam;
//					SendMessage(m_hParentWnd, WM_KEYDOWN, (WPARAM)pKey->nVKey, (LPARAM)pKey->uFlags);
					return 0;
				}
				case TVN_SELCHANGED:
				case TVN_SELCHANGING:
				{
					NMTREEVIEW* pTv = (NMTREEVIEW*)lParam;
					SAFE_CALLBACK(m_pOnSelectItem, (GetItemIndex(pTv->itemNew.hItem), (pTv->action == TVC_BYMOUSE), GetItemIndex(pTv->itemOld.hItem), FALSE));
					break;
				}
				case TVN_KEYDOWN:
				{
					NMTVKEYDOWN* pTvkd = (NMTVKEYDOWN*)lParam;

					int ret;
					//SAFE_CALLBACK_CHECK(ret, m_pKeyboardFunc, ( (UCHAR)pTvkd->wVKey, ((GetKeyState(pTvkd->wVKey) & 0x80)) ));
					SAFE_CALLBACK_CHECK(ret, m_pKeyboardFunc, ((UCHAR)pTvkd->wVKey, TRUE));
					//if(ret!=0)//TRUE
					//{
					//	if(m_hParentWnd)
					//		SendMessage(m_hParentWnd, WM_KEYDOWN, (WPARAM)pTvkd->wVKey, (LPARAM)pTvkd->flags);
					//}

					switch(pTvkd->wVKey)
					{
						case VK_F2:
							TreeView_EditLabel(m_hTreeViewWnd, TreeView_GetSelection(m_hTreeViewWnd));
							break;
					}
					break;
				}
				case TVN_BEGINLABELEDIT:
				{
					pTvdi = (TV_DISPINFO*)lParam;
					int ret;
					//SAFE_CALLBACK_CHECK(ret, m_pNameEditFunc, (GetItemIndex(TreeView_GetSelection(m_hTreeViewWnd)), pTvdi->item.pszText));
					SAFE_CALLBACK_CHECK(ret, m_pNameEditFunc, (GetItemIndex(pTvdi->item.hItem), pTvdi->item.pszText, TRUE));
					if(ret == TRUE)
					{
						return FALSE; //Returns TRUE to cancel label editing.
					}
					return TRUE;
				}
				case TVN_ENDLABELEDIT:	//ノード名変更の通知
				{
					pTvdi = (TV_DISPINFO*)lParam;
					int ret;
					//SAFE_CALLBACK_CHECK(ret, m_pNameEditFunc, (GetItemIndex(TreeView_GetSelection(m_hTreeViewWnd)), pTvdi->item.pszText));
					SAFE_CALLBACK_CHECK(ret, m_pNameEditFunc, (GetItemIndex(pTvdi->item.hItem), pTvdi->item.pszText, FALSE));
					if(ret == TRUE)
					{
						return TRUE;
						//TreeView_SetItem(m_hTreeViewWnd, &pTvdi->item);
					}
					return FALSE;//If the pszText member is non-NULL, return TRUE to set the item's label to the edited text. Return FALSE to reject the edited text and revert to the original label.
				}
				case TVN_BEGINDRAG:
				{
					NMTREEVIEW* pTv = (NMTREEVIEW*)lParam;
					SAFE_CALLBACK(m_pOnSelectItem, (GetItemIndex(pTv->itemNew.hItem), (pTv->action == TVC_BYMOUSE), GetItemIndex(pTv->itemOld.hItem), TRUE));
					break;
				}
				//case TVN_ITEMEXPANDING:
				//{
				//	if(nm->action & TVE_EXPAND)
				//		TreeView_Expand(hTree, m_hItemList[m_nTreeItemNo], TVE_EXPAND);
				//	else if(nm->action & TVE_COLLAPSE)
				//		TreeView_Expand(hTree, m_hItemList[m_nTreeItemNo], TVE_COLLAPSE);
				//}
			}
		}
		break;
    case WM_CONTEXTMENU:
		SAFE_CALLBACK(m_pPopupFunc, (hWnd, wParam, lParam));
        break;
    case WM_COMMAND:
		SAFE_CALLBACK(m_pCommandFunc, (hWnd, wParam, lParam));
        break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CTreeView::EnsureVisible(int nDest)
{
	TreeView_EnsureVisible(m_hTreeViewWnd, m_hItemList[ExternalToInternal(nDest)]);
}

void CTreeView::SetCallbackFunctions(int OnNameEdit(int, const TCHAR*, bool), void OnPopupRequest(HWND, WPARAM, LPARAM), void OnWMCommand(HWND, WPARAM, LPARAM), int OnKeyboardEvent(UCHAR key, bool isDown), int OnMouseEvent(long x, long y, UINT btn), void OnSelectItem(int, bool, int, bool))
{
	m_pNameEditFunc = OnNameEdit;
	m_pPopupFunc = OnPopupRequest;
	m_pCommandFunc = OnWMCommand;
	m_pKeyboardFunc = OnKeyboardEvent;
	m_pMouseFunc = OnMouseEvent;
	m_pOnSelectItem = OnSelectItem;
}

HWND CTreeView::SetParent(HWND hNewParent)
{
	m_hParentWnd = hNewParent;
	return ::SetParent(m_hTreeViewHolderWnd, hNewParent);
}

void CTreeView::CreateTreeView(int x, int y, int width, int height, const TCHAR* szTreeTitle, HWND hParentWnd)
{
	m_hParentWnd = hParentWnd;

	//ツリービューを搭載するウィンドウを作る
	WNDCLASSEX wc;
	if(!GetClassInfoEx(GetModuleHandle(NULL), _T("CTreeView"), &wc))
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
		wc.lpszClassName	= _T("CTreeView");
		wc.hIconSm			= LoadIcon(NULL, IDI_APPLICATION);

		if(!RegisterClassEx(&wc))
		{
			ExitProcess(0);
			return;
		}
	}
	
	//TIPS: CreateWindowでは小さく作っておく(サイズを適当に-2しておく)
	if(!(m_hTreeViewHolderWnd=CreateWindowEx(	
							WS_EX_TOOLWINDOW,
        					_T("CTreeView"),
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
	ShowWindow(m_hTreeViewHolderWnd, SW_SHOWNORMAL);
	UpdateWindow(m_hTreeViewHolderWnd);

	//ツリービューそのものの生成
	m_hTreeViewWnd = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, "",
					WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL| WS_HSCROLL| TVS_HASLINES |
					TVS_HASBUTTONS | TVS_LINESATROOT| TVS_SHOWSELALWAYS |  TVS_EDITLABELS,
					0, 0, 10, 10,
					m_hTreeViewHolderWnd, (HMENU)m_dwTreeId, GetModuleHandle(NULL), NULL);
	//ツリービューのウィンドウプロシージャをフック
	m_TreeViewOldWndProc = (WNDPROC)GetWindowLong(m_hTreeViewWnd, GWL_WNDPROC);
	m_TreeViewWndProc = TreeViewWndProc;
	SetProp(m_hTreeViewWnd, "CTreeView", this);
	SetWindowLong(m_hTreeViewWnd , GWL_WNDPROC, (LONG)m_TreeViewWndProc);

	//ツリービューを親ウィンドウにフィットさせる
	RECT rect;
	//貼り付け先のウィンドウの矩形をGetする
	GetWindowRect(m_hTreeViewHolderWnd, &rect);
	//まず、ツリービューを貼り付ける
	MoveWindow(m_hTreeViewWnd, 0, 0, rect.right, rect.bottom, FALSE);
	//最後に、親ウィンドウを指定された大きさに変更することで大きさを合わせる
	//これを怠ると、Vスクロールバーがうまく表示されない現象が生じる
	MoveWindow(m_hTreeViewHolderWnd, x, y, width, height, TRUE);
}

int CTreeView::GetAmount()
{
	return InternalToExternal(m_nTreeItemNo);
	//return TreeView_GetVisibleCount(m_hTreeViewWnd);
}

int CTreeView::GetItemFromPos(long x, long y)
{
	TVHITTESTINFO ht;
	ZeroMemory(&ht, sizeof(TVHITTESTINFO));
	ht.pt.x = x;
	ht.pt.y = y;
	TreeView_HitTest(m_hTreeViewWnd, &ht);
	return GetItemIndex(ht.hItem);
}

LRESULT CALLBACK CTreeView::TreeViewWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CTreeView* tv = (CTreeView*)GetProp(hWnd, "CTreeView");
	if(!tv)	return DefWindowProc(hWnd, msg, wParam, lParam);

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
				return CallWindowProc(tv->m_TreeViewOldWndProc, tv->m_hTreeViewWnd, msg, wParam, lParam);
			}
		}
		//case WM_MOUSEWHEEL:
		//SetFocus(tv->m_hParentWnd);
		//DefWindowProc(hWnd, msg, wParam, lParam);
		return CallWindowProc((WNDPROC)GetWindowLong(tv->m_hParentWnd, GWL_WNDPROC), tv->m_hParentWnd, msg, wParam, lParam);
	}

	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
	case WM_LBUTTONDOWN:
	{
		SAFE_CALLBACK(tv->m_pMouseFunc, (LOWORD(lParam), HIWORD(lParam), msg));
		if(!(GetAsyncKeyState(VK_LBUTTON)&0x8000)) CallWindowProc((WNDPROC)GetWindowLong(tv->m_hParentWnd, GWL_WNDPROC), tv->m_hParentWnd, msg, wParam, lParam);
		break;
	}
	}
	return CallWindowProc(tv->m_TreeViewOldWndProc, tv->m_hTreeViewWnd, msg, wParam, lParam);
}

int CTreeView::AddItem(const TCHAR* szObjName, int nDest, bool isSorting)
{
	HWND hTree = m_hTreeViewWnd;
	HTREEITEM hItem = NULL;
	TV_INSERTSTRUCT tvis;
	ZeroMemory(&tvis, sizeof(TV_INSERTSTRUCT));

	if(szObjName == NULL)
		return -1;

	//デスティネーションヒエラルキーを設定
	if(nDest < 0){
		//現在の選択位置をデスティネーションに設定
		hItem = TreeView_GetSelection(hTree);
		if(!hItem) return -1;
		tvis.hParent = hItem;
	}else if(nDest == 0){
		tvis.hParent = TVI_ROOT;
	}else{
		nDest = ExternalToInternal(nDest);
		if(nDest >= CTreeView::m_nItemListAllocateSize)
			return -1;
		if(m_hItemList[nDest])
			tvis.hParent = m_hItemList[nDest];
		else
			return -1;
	}

	//ソートするか
	tvis.hInsertAfter = isSorting ? TVI_SORT : TVI_LAST;

//	if(isExpand)
//	{
//		tvis.item.mask = TVIF_TEXT | TVIF_STATE;
//		tvis.item.state |= TVIS_EXPANDED;
//		tvis.item.stateMask |= TVIS_EXPANDED;
//	}else{
		tvis.item.mask = TVIF_TEXT;
//	}
	
	//
	tvis.item.pszText = (TCHAR*)malloc(sizeof(TCHAR) * (lstrlen(szObjName)+1));
	lstrcpy(tvis.item.pszText, szObjName);

	//配列は0から始まっているので、-1しなくちゃだめ
	if(++m_nTreeItemNo >= ExternalToInternal(CTreeView::m_nItemListAllocateSize-1))
	{
		m_hItemList = (HTREEITEM*)realloc(&m_hItemList, sizeof(HTREEITEM) * CTreeView::m_nItemListAllocateSize);
		CTreeView::m_nItemListAllocateSize *= 2;
	}
	m_hItemList[m_nTreeItemNo] = TreeView_InsertItem(hTree, &tvis);
	//OutTraceLine();
	//OutTrace("AddItem");

	//OutTrace("m_hItemList[%d]: %x", m_nTreeItemNo, m_hItemList[m_nTreeItemNo]);
	//if(isExpand){
	//	TreeView_Expand(hTree, m_hItemList[m_nTreeItemNo], TVE_EXPAND);
		TreeView_EnsureVisible(hTree, m_hItemList[m_nTreeItemNo]);
	//}else{
	//	TreeView_Expand(hTree, m_hItemList[m_nTreeItemNo], TVE_COLLAPSE);
	//}
	free(tvis.item.pszText);
	return InternalToExternal(m_nTreeItemNo);
}

int CTreeView::DeleteItem(int nDest)
{
	HWND hTree = m_hTreeViewWnd;
	HTREEITEM hItem = NULL;

	//デスティネーションヒエラルキーを設定
	if(nDest < 0){
		//現在の選択位置から求める
		hItem = TreeView_GetSelection(hTree);
		if(!hItem) return -1;
	}else if(nDest == 0){
		//ルートが削除されたら、すべて無効にする
		hItem = TVI_ROOT;
		for(int i=0; i<CTreeView::m_nItemListAllocateSize; i++)
		{
			m_hItemList[i] = NULL;
		}
		m_nTreeItemNo = -1;
	}else{
		nDest = ExternalToInternal(nDest);
		if(nDest >= CTreeView::m_nItemListAllocateSize)
			return -1;
		if(m_hItemList[nDest])
		{
			hItem = m_hItemList[nDest];
			m_hItemList[nDest] = NULL;
		}
		else return -1;
	}

	if(!hItem) return -1;

	TreeView_DeleteItem(hTree, hItem);
	if(hItem != TVI_ROOT)	m_nTreeItemNo--;
	else					m_nTreeItemNo = -1;

	//成功したら、削除されたハンドルを返す
	return InternalToExternal(nDest);
}

void CTreeView::DeleteAllItem()
{
	//OutTraceLine();
	//OutTrace("DeleteAllItem");
	for(int i=0; i<CTreeView::m_nItemListAllocateSize; i++)
	{
		m_hItemList[i] = NULL;
	}
	//初期化
	m_nTreeItemNo = -1;
	HTREEITEM hItem = TreeView_GetRoot(m_hTreeViewWnd);
	if(hItem!=NULL)
		TreeView_DeleteItem(m_hTreeViewWnd, TVI_ROOT);
}

int CTreeView::GetItemIndex(HTREEITEM hItem)
{
	if(hItem == NULL){
		return -1;
	}
	//OutTraceLine();
	//OutTrace("GetItemIndex");
	for(int i=0; i<CTreeView::m_nItemListAllocateSize; i++)
	{
		//OutTrace("hItem: %x m_hItemList[%d]: %x",hItem, i, m_hItemList[i]);
		if(m_hItemList[i] == hItem)
		{
			//配列は0から始まっているが, ユーザーに返す値は1から始まっている
			return InternalToExternal(i);
		}
	}
	return -1;
}

int CTreeView::GetItemNum()
{
	return InternalToExternal(m_nTreeItemNo);
}

int CTreeView::GetItemIndex(int nDest)
{
	return GetItemIndex(m_hItemList[ExternalToInternal(nDest)]);
}

int CTreeView::GetSelectedItem()
{
	HTREEITEM hItem = TreeView_GetSelection(m_hTreeViewWnd);
	if(hItem == NULL) return -1;
	return GetItemIndex(hItem);
}

bool CTreeView::IsExpand(int nDest)
{
	TVITEM tvItem;
	ZeroMemory(&tvItem, sizeof(TVITEM));

	tvItem.mask = TVIF_STATE;
	tvItem.hItem = m_hItemList[ExternalToInternal(nDest)];
	TreeView_GetItem(m_hTreeViewWnd, &tvItem);
	if(tvItem.state & TVIS_EXPANDED) return true;
	return false;
}

void CTreeView::ExpandItem(int nDest, bool isExpand)
{
	if(ExternalToInternal(nDest) >= CTreeView::m_nItemListAllocateSize) return;
	TreeView_Expand(m_hTreeViewWnd, m_hItemList[ExternalToInternal(nDest)], (isExpand) ? TVE_EXPAND : TVE_COLLAPSE);
}

int CTreeView::SelectItem(int nDest)
{
	nDest = ExternalToInternal(nDest);
	if(nDest >= CTreeView::m_nItemListAllocateSize) return -1;
	TreeView_SelectItem(m_hTreeViewWnd, m_hItemList[nDest]);
	TreeView_EnsureVisible(m_hTreeViewWnd, m_hItemList[nDest]);
	return InternalToExternal(nDest);
}

TCHAR* CTreeView::GetItemName(int nDest)
{
	HWND hTree = m_hTreeViewWnd;
	static TCHAR szItemName[512];
	TVITEM tvItem;
	ZeroMemory(&tvItem, sizeof(TVITEM));

	//デスティネーションヒエラルキーを設定
	if(nDest < 0){
		//現在の選択位置から求める
		tvItem.hItem = TreeView_GetSelection(hTree);
		if(!tvItem.hItem) return "";
	}else if(nDest == 0){
		tvItem.hItem = TVI_ROOT;
	}else{
		nDest = ExternalToInternal(nDest);
		if(nDest >= CTreeView::m_nItemListAllocateSize)
			return "";
		if(m_hItemList[nDest])
			tvItem.hItem = m_hItemList[nDest];
		else return "";
	}

	tvItem.mask = TVIF_TEXT;
	tvItem.pszText = &szItemName[0];
	tvItem.cchTextMax = 512;
	TreeView_GetItem(hTree, &tvItem);

	return szItemName;
}

void CTreeView::SetItemName(const TCHAR* szNewName, int nDest)
{
	HWND hTree = m_hTreeViewWnd;
	TVITEM tvItem;
	ZeroMemory(&tvItem, sizeof(TVITEM));

	//デスティネーションヒエラルキーを設定
	if(nDest < 0){
		//現在の選択位置から求める
		tvItem.hItem = TreeView_GetSelection(hTree);
		if(!tvItem.hItem) return;
	}else if(nDest == 0){
		tvItem.hItem = TVI_ROOT;
	}else{
		nDest = ExternalToInternal(nDest);
		if(nDest >= CTreeView::m_nItemListAllocateSize)
			return;
		if(m_hItemList[nDest])
			tvItem.hItem = m_hItemList[nDest];
		else return;
	}

	tvItem.mask = TVIF_TEXT;
	tvItem.pszText = (TCHAR*)malloc(sizeof(TCHAR) * (lstrlen(szNewName)+1));
	lstrcpy(tvItem.pszText, szNewName);
	TreeView_SetItem(hTree, &tvItem);
	return;
}