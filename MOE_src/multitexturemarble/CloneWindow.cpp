#include "stdafx.h"
#include "CloneWindow.h"
#include "StartupCode.h"
#include "modeler window.h"
#include "MaterialWindow.h"
#include "PrimitiveWindow.h"
#include "FileOperation.h"

extern KModelEdit mdl;
extern CWindowGL win;
extern CListView vMaterial;
extern char g_szModelName[1024];
CTreeView vClone;

typedef struct _tagCloneDialogParam
{
	char szCloneName[MAX_PATH];
	int nCloneMode;
	int nPrimitiveId;
	int nMaterialId;
	char szMasterName[MAX_PATH];
}CloneDialogParam;

CloneDialogParam g_Dp;

bool isDragging = false;
int	 DraggingFrom = -1;
HCURSOR hOldCursor;

//-1: KCloneTree形式への変換 -1:ファイル名を表示している分、減算
#define CTreeViewToKClone(tv) (tv-2)
#define KCloneToCTreeView(kcl) (kcl+2)
//#define CTreeViewToKClone(tv) (tv+(-1-1))
//#define KCloneToCTreeView(kcl) (kcl+(1+1))

void CloneDelete(KClone* pTarget)
{
	KClone*	kcl	= mdl.GetCloneAllocPtr();
	long allocnum = mdl.GetCloneAllocNum();
	KClone* kindex = pTarget;
	if(kindex == NULL) return;

	if(mdl.GetTree() == kindex){
		mdl.SetTree(kindex->sibling);
		for(int i=0; i<allocnum; i++)
		{
			if(&kcl[i] == kindex){
				kcl[i].sibling = NULL;
				//kcl[i].child   = NULL;
			}
		}
	}else{
		//繋ぎ替え
		for(int i=0; i<allocnum; i++)
		{
			if(&kcl[i] != kindex)
			{
				if((kcl[i].child   == kindex))
					kcl[i].child   = kindex->sibling;
				else if((kcl[i].sibling == kindex))
					kcl[i].sibling = kindex->sibling;
			}
		}

		//情報の削除. 上のループと一緒にしてしまうと, 特定の条件時に変な繋ぎ方になってしまう.
		//(繋ぎかえる先を発見する前に, 自分自身を見つけてしまった場合は自分の兄弟が消失する)
		for(int i=0; i<allocnum; i++)
		{
			if(&kcl[i]==kindex){
				kcl[i].sibling = NULL;
				//kcl[i].child   = NULL;
			}
		}
	}
}

KClone* pSaveForDelete;
KClone* pSaveForDeleteSibling;
KClone* CloneAddToEmptyPoint(KClone* pDest, KClone* pAdd) //child->Child's sibling->sibling...
{
	KClone*	kcl	= mdl.GetCloneAllocPtr();
	long allocnum = mdl.GetCloneAllocNum();
	KClone* kindex = pDest;
	KClone* tmp;
	if(kindex == NULL){//一番上の階層の子供にする
		if(mdl.GetTree() == NULL) mdl.SetTree(pAdd);
		
		if(mdl.GetTree()->sibling == NULL)	tmp = mdl.GetTree();
		else tmp = mdl.GetTree()->sibling;

		for(int i=0; i<allocnum; i++)
		{
			if(&kcl[i] != pSaveForDelete){ 
				if(&kcl[i] == tmp){
					if(kcl[i].sibling == NULL){
						(kcl[i].sibling) = pAdd;
						return NULL;
					}
					tmp = tmp->sibling;
					i=-1;
				}
			}
		}
	}else{ //ファイル名以外の場所へドロップした
		for(int i=0; i<allocnum; i++)
		{
			if(&kcl[i] == kindex && kcl[i].child==NULL)
			{
				(kcl[i].child) = pAdd;
				return NULL;
			}
		}

		if(kindex->child->sibling == NULL) tmp = kindex->child;
		else tmp =  kindex->child->sibling;
		for(int i=0; i<allocnum; i++)
		{
			if(&kcl[i] == tmp)
			{
				if(&kcl[i] != pSaveForDelete){	
					if(kcl[i].sibling == NULL)
					{
						(kcl[i].sibling) = pAdd;
						return NULL;
					}
					i=-1;
					tmp = tmp->sibling;
				}
			}
		}
	}
	//if(pDest->child==NULL){
	//	return &(pDest->child);
	//}else{
	//	KClone** tmp = &(pDest->child);
	//	while(1)
	//	{
	//		if(*tmp==NULL)
	//		{
	//			return tmp;
	//		}
	//		*tmp=(*tmp)->sibling;
	//	}
	//}
	return NULL;
}

KClone* CloneReConnect(int nDest) //nDestの場所に,DraggingFromのポインタを挿入する
{
	int nFrom = DraggingFrom;
	if(DraggingFrom == nDest){ //同じ場所にドロップされた場合
		return NULL;
	}
	DraggingFrom = -1;	//次回以降の処理のために初期化する

	KClone* pFrom = GetKClone(nFrom);
	if(!pFrom) return NULL;
	KClone* pDest = GetKClone(nDest);
	if(!pDest){
		if(CTreeViewToKClone(nDest)<0){ //ファイル名に追加された場合
			pDest = NULL;
		}
		//if(mdl.GetTree() == pFrom) return NULL; //一番上のやつが自分自身をコピーできないように
	}else{
		if(pDest == pFrom->child) return NULL;
		//if(pFrom  == pDest->child) return NULL;

		//チェック
		KClone** node_ptr=(KClone**)GlobalAlloc(GPTR,256*sizeof(KClone*));
		KClone*	kpt=mdl.GetTree();
		if(kpt == NULL)	return NULL;

		int node = 0;
		long child_end = 0;
		KClone* ignore = pFrom->child;
		while(node!=-1)
		{
			if(ignore==NULL) break;

			if(ignore->child == kpt || ignore->sibling == kpt) return NULL;
			if((child_end!=1)&&(kpt->child!=NULL)){
				node++;
				kpt	= kpt->child;
				ignore = kpt;
				node_ptr[node]=kpt;
				child_end=0;
			}else if(kpt->sibling!=NULL){
				kpt	= kpt->sibling;
				ignore = kpt;
				node_ptr[node]=kpt;
				child_end=0;
			}else{
				node--;
				kpt	= node_ptr[node];
				if(kpt==NULL) kpt = mdl.GetTree()->sibling;
				if(kpt==NULL) break;
				//ignore = kpt;
				child_end=1;
			}
		}
		GlobalFree(node_ptr);
	}
	SetUndo();

	pSaveForDelete = pFrom;
	pSaveForDeleteSibling = pFrom->sibling;
	CloneDelete(pFrom);
	CloneAddToEmptyPoint(pDest, pFrom);
	
	return NULL;
}

KClone* CloneInsertItem(int nDest, int nAdditional) //nDestの場所にnew itemを追加する
{
	int nFrom = DraggingFrom;
	if(DraggingFrom == nDest){ //同じ場所にドロップされた場合
		return NULL;
	}
	DraggingFrom = -1;	//次回以降の処理のために初期化する

	KClone* pKcl = NULL; //新しく生成されたクローンを格納する

	if(nFrom<0) nFrom = nDest; //ドラッグされずに, PrimitiveListがダブルクリックされた場合
	
	KClone* pFrom = GetKClone(nFrom);
	KClone* pDest = GetKClone(nDest);
	if((pFrom == pDest) || (!pFrom || !pDest)){
		//if(pFrom == pDest){ //現在選択されている場所に追加する場合
			SetUndo();
			pKcl = mdl.CreateClone(1);
			if(mdl.GetTree()!=NULL){ //一番上に誰かがいる場合
				KClone** kChild = NULL;
				if(nDest==KCloneToCTreeView(-1)){//root 1 //
					kChild = &(mdl.GetTree()->sibling);
				}else if(nDest!=KCloneToCTreeView(0)){ //
					kChild = &(GetKClone(nDest)->child);
				}else{//GetTree()を選択した場合 2
					kChild = &(mdl.GetTree()->child);
				}

				if((*kChild)==NULL)
				{
					(*kChild) = pKcl;
				}else{
					KClone* tmp = (*kChild);
					while(tmp->sibling != NULL) tmp = tmp->sibling;
					tmp->sibling = pKcl; //選択されたやつの子供になる
				}
			}else{			//誰もいなかった場合、新しく作ったのをルートにする
				mdl.SetTree(pKcl);
			}
			pKcl->clone_data.primitive_id = (nAdditional>=0) ? nAdditional : 0;//PrimitiveID クローンモードなら無視
			pKcl->clone_data.material_id = (vMaterial.GetSelectedItem()>=0) ? vMaterial.GetSelectedItem() : 0;
			pKcl->clone_data.clonemode = 0;//新機能！クローンモード1 0ならPrimitiveID参照
			char szCloneName[] = "no_name";
			pKcl->clone_data.clone_name = (char*)GlobalAlloc(GPTR, (lstrlen(szCloneName)+1) * sizeof(char));//名前
			lstrcpy(pKcl->clone_data.clone_name, szCloneName);
			pKcl->clone_data.scale.x=1.0f;//拡大縮小率は最低でもないとみえね・・・
			pKcl->clone_data.scale.y=1.0f;
			pKcl->clone_data.scale.z=1.0f;	

			vClone.SelectItem(GetCTreeViewId(pKcl));
			vClone.EnsureVisible(GetCTreeViewId(pKcl));
			//ModelerSelectClone(pKcl);
			//Render();
			return pKcl;
	}else{
		return NULL;	
	}
}

void AddCloneTree(HWND hTreeView, int nOrderInKClone, char* str)
{
	vClone.AddItem(str, (nOrderInKClone>0) ? KCloneToCTreeView(nOrderInKClone) : nOrderInKClone);
}

void ClearCloneTree(HWND hTreeView)
{
	vClone.DeleteAllItem();
}

//long before_allocnum;
//KClone* prev_kcl;
//long before_selected_item;
//KClone* prev_kclone;
void SaveItemState()
{
	KClone*	kcl	= mdl.GetCloneAllocPtr();
	long before_allocnum = mdl.GetCloneAllocNum();
	if(kcl==NULL) return;

//	before_selected_item = vClone.GetSelectedItem();
	for(int i=0; i<before_allocnum; i++)
	{
		if(i==0) continue;
		//if(i==0) continue;//ルートの場合は,KCloneは存在しない
  //      //選択状態を保存する
		//KClone* kcl = GetKClone(i+1);
		//if(kcl==NULL) continue;
		//if(CTreeViewToKClone(i+1)<before_allocnum){
		//	kcl->clone_data.tree_collapse = (vClone.IsExpand(i+1)) ? 0:1;
		//	//if(kcl==prev_kcl)
		//	//	kcl->clone_data.tree_collapse = (vClone.IsExpand(i+1)) ? 0:1;
		//	//else
		//	//	kcl->clone_data.tree_collapse = 0;
		//}
		//vClone.SelectItem(i);
		kcl[i].clone_data.tree_collapse = !vClone.IsExpand(GetCTreeViewId(&kcl[i])); 
	}
}

void ApplyItemState()
{
	KClone*	kcl	= mdl.GetCloneAllocPtr();
    long allocnum = mdl.GetCloneAllocNum();
	if(kcl==NULL) return;

	for(int i=0; i<allocnum; i++)
	{
		//if(allocnum==before_allocnum){
		//	vClone.SelectItem(before_selected_item);
		//	vClone.ExpandItem(GetCTreeViewId(&kcl[i]), !kcl[i].clone_data.tree_collapse);
		//}else{
		//	vClone.ExpandItem(GetCTreeViewId(&kcl[i]), TRUE);
		//}
		//vClone.SelectItem(i);
		vClone.ExpandItem(GetCTreeViewId(&kcl[i]), !kcl[i].clone_data.tree_collapse);
	}
	//if(allocnum==before_allocnum)	
	//	vClone.SelectItem(before_selected_item);
	//else
	//	vClone.SelectItem(1);
}

void RefreshCloneTree()
{
	if(g_szModelName[0]=='\0') return;
	//prev_kclone = (KClone*)GlobalAlloc(GPTR, GlobalSize(mdl.GetTree()));
	//CopyMemory(prev_kclone, mdl.GetTree(), GlobalSize(mdl.GetTree()));

	SaveItemState();
	ClearCloneTree(vClone.GetTreeWnd());

	//OutTrace("GetClone() : mdl.GetTree() -> %p", kpt);
	char* szTitle = (char*)GlobalAlloc(GPTR, sizeof(char) * (lstrlen(g_szModelName)+1));
	lstrcpy(szTitle, g_szModelName);
	PathStripPath(szTitle);
	AddCloneTree(vClone.GetTreeWnd(), 0, szTitle);
	GlobalFree(szTitle);
	
	KClone** node_ptr=(KClone**)GlobalAlloc(GPTR,256*sizeof(KClone*));
	KClone*	kpt=mdl.GetTree();
	if(kpt == NULL){
		vClone.SelectItem(1);
		vClone.EnsureVisible(1);
		return;
	}

	int* node_id=(int*)GlobalAlloc(GPTR,256*sizeof(int));

	int node=0;
	long child_end=0;

	node_id[node] = 1;
    node_ptr[node]=NULL;
	//NULL->child

	node++;
	node_id[node] = vClone.AddItem(kpt->clone_data.clone_name, node_id[node-1]);
	node_ptr[node] = kpt;
	
	while(node!=-1)
	{
		if((child_end!=1)&&(kpt->child!=NULL)){
			node++;
			kpt	= kpt->child;
			node_ptr[node]=kpt;
			child_end=0;
			//--copy--------------------
			node_id[node] = vClone.AddItem(kpt->clone_data.clone_name, node_id[node-1]);
			//--------------------------
		}else if(kpt->sibling!=NULL){
			kpt	= kpt->sibling;
			node_ptr[node]=kpt;
			child_end=0;
			//--copy--------------------
			node_id[node] = vClone.AddItem(kpt->clone_data.clone_name, node_id[node-1]);
			//--------------------------
		}else{
			node--;
			kpt	= node_ptr[node];
			if(kpt==NULL) kpt = mdl.GetTree()->sibling;
			if(kpt==NULL) break;
			child_end=1;
		}
	}
	GlobalFree(node_ptr);
	GlobalFree(node_id);

	//GlobalFree(prev_kclone);

	vClone.SetCheck(0+1, TRUE);
	for(int i=0; i<vClone.GetItemNum(); i++)
	{
		if(i==0) continue;
		KClone* p = GetKClone(i+1);
		if(!p) continue;
		vClone.SetCheck(i+1, !(p->clone_data.visible));
	}

	//vClone.SelectItem(lastselection

	ApplyItemState();
}

//KCloneのポインタから、CTreeViewより返却されるIDに変換する
int GetCTreeViewId(KClone* kcl)
{
	KClone** node_ptr=(KClone**)GlobalAlloc(GPTR,256*sizeof(KClone*));
	KClone*	kpt=mdl.GetTree();
	if(kpt == NULL)	return 0;

	//KCloneの中のルートが選択された場合の例外処理
	if(kpt == kcl) return KCloneToCTreeView(0);

	int nAddNum = 1;	//KClone上のID
	int node=0;
	long child_end=0;

    node_ptr[node]=NULL;
	
	node++;
	node_ptr[node] = kpt;
	
	//if(nAddNum == nDest) return kpt;
	//nAddNum++;
	while(node!=-1){
		if((child_end!=1)&&(kpt->child!=NULL)){
			node++;
			kpt	= kpt->child;
			node_ptr[node]=kpt;
			child_end=0;
			if(kpt == kcl) return KCloneToCTreeView(nAddNum);
			nAddNum++;
		}else if(kpt->sibling!=NULL){
			kpt	= kpt->sibling;
			node_ptr[node]=kpt;
			child_end=0;
			if(kpt == kcl)	return KCloneToCTreeView(nAddNum);
			nAddNum++;
		}else{
			node--;
			kpt	= node_ptr[node];
			if(kpt==NULL) kpt = mdl.GetTree()->sibling;
			if(kpt==NULL) break;
			child_end=1;
		}
	}
	GlobalFree(node_ptr);
	return -1;
}

//CTreeViewから返却されるIDに該当するKCloneのポインタを返す
KClone* GetKClone(int nTreeViewId)
{
	int nDest = CTreeViewToKClone(nTreeViewId);
	KClone** node_ptr=(KClone**)GlobalAlloc(GPTR,256*sizeof(KClone*));
	KClone*	kpt=mdl.GetTree();
	if(kpt == NULL)	return NULL;

	//KCloneの中のルートが選択された場合の例外処理
	if(nDest == 0) return kpt;
	if(nDest <= -1) return NULL;

	int nAddNum = 1;	//KClone上のID
	int node=0;
	long child_end=0;

    node_ptr[node]=NULL;
	
	node++;
	node_ptr[node] = kpt;
	//if(nAddNum == nDest) return kpt;
	//nAddNum++;
	while(node!=-1){
		if((child_end!=1)&&(kpt->child!=NULL)){
			node++;
			kpt	= kpt->child;
			node_ptr[node]=kpt;
			child_end=0;
			if(nAddNum == nDest) return kpt;
			nAddNum++;
		}else if(kpt->sibling!=NULL){
			kpt	= kpt->sibling;
			node_ptr[node]=kpt;
			child_end=0;
			if(nAddNum == nDest) return kpt;
			nAddNum++;
		}else{
			node--;
			kpt	= node_ptr[node];
			if(kpt==NULL) kpt = mdl.GetTree()->sibling;
			if(kpt==NULL) break;
			child_end=1;
		}
	}
	GlobalFree(node_ptr);
	return NULL;
}

//ラベル名称変更の通知を処理する
int CloneNameEdit(int nDest, const	TCHAR* szNewName, bool isAboutToEdit)
{
	if(!szNewName) return FALSE;
	if(lstrlen(szNewName)<=0) return FALSE;
	if(!isAboutToEdit && CTreeViewToKClone(nDest)>=0){
		KClone*	kcl	= GetKClone(nDest);
		if(kcl==NULL) return FALSE;

		SetUndo();

		GlobalFree(kcl->clone_data.clone_name);

		kcl->clone_data.clone_name = (char*)GlobalAlloc(GPTR, lstrlen(szNewName)+1);
		lstrcpy(kcl->clone_data.clone_name,	szNewName);
	}else{
	}
	return TRUE;//TRUEを返したら,設定が反映される
}

void ClonePopup(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	TV_HITTESTINFO tvhtst;
	HTREEITEM hItem;
	HWND hTree = vClone.GetTreeWnd();

	tvhtst.pt.x	= LOWORD(lParam);
	tvhtst.pt.y	= HIWORD(lParam);
	ScreenToClient(hTree, &tvhtst.pt);
	hItem =	TreeView_HitTest(hTree,	&tvhtst);
	if (hItem){
		HMENU hMenu, hSub;
		TreeView_SelectItem(hTree, hItem);
		hMenu =	LoadMenu(GetModuleHandle(NULL), "CLONEVIEW");//GetMenu(hWnd);
        hSub = GetSubMenu(hMenu, 0);
		//hNew = GetSubMenu(hMenu, 0);
		//AppendMenu(hNew, 
		TrackPopupMenu(hSub, TPM_LEFTALIGN, (int)LOWORD(lParam), (int)HIWORD(lParam), 0, hWnd,	NULL);
	}
}

void CloneSelectClone(KClone* pDest)
{
	int item = GetCTreeViewId(pDest);
	vClone.SelectItem(item);
	SetFocus(hMainWnd);
	Render();
}

void CloneCommand(HWND hWnd, WPARAM	wParam,	LPARAM lParam)
{
	HWND hTree = vClone.GetTreeWnd();
	HTREEITEM hItem = NULL;
	
	switch (LOWORD(wParam)){
		case ID_ITEMVIEW_NEW:
		{
			KClone* pKcl = CloneInsertItem(vClone.GetSelectedItem());
			ModelerSelectClone(pKcl); //追加されたものをアクティブにする
			Render();
			RefreshCloneTree();
			break;
		}
		case ID_ITEMVIEW_EDITNAME: //編集を開始する
		{
			hItem =	TreeView_GetSelection(hTree);
			if(hItem) TreeView_EditLabel(hTree, hItem);
			OutTrace("EditName()");
			break;
		}
		case ID_ITEMVIEW_DELETE:
		{
			SetUndo();
			KClone* kindex = GetKClone(vClone.GetSelectedItem());
			CloneDelete(kindex);
			RefreshCloneTree();
			break;
		}
		case ID_ITEMVIEW_PROPERTY:
		{
			int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_CLONE_DIALOG), hMainWnd, (DLGPROC)CloneDlgProc);
			if(ret>=0){
				SetUndo();
				KClone* target = GetKClone(vClone.GetSelectedItem());
				if(target == NULL) return;

				//clone mode
				target->clone_data.clonemode = (int)(g_Dp.nCloneMode);

				//clone name
				GlobalFree(target->clone_data.clone_name);
				target->clone_data.clone_name = (char*)GlobalAlloc(GPTR, lstrlen(g_Dp.szCloneName)+1);
				lstrcpy(target->clone_data.clone_name, g_Dp.szCloneName);

				//primitive & material idは,clone modeのときは設定しない
				if(target->clone_data.clonemode<=0)
				{
					target->clone_data.primitive_id = (mdl.GetPrimitive(g_Dp.nPrimitiveId)!=NULL)	? g_Dp.nPrimitiveId : target->clone_data.primitive_id;
					//target->clone_data.material_id	= (mdl.GetMaterial(g_Dp.nMaterialId)!=NULL)		? g_Dp.nMaterialId	: target->clone_data.material_id;
					target->clone_data.material_id	= g_Dp.nMaterialId;
				}

				//clone modeなら, クローン生成しなおす
				if(target->clone_data.clonemode>0){
					//KMODEL
					KClone* kptr = GetKClone(vClone.GetSelectedItem());
					if(kptr == NULL) break;
					//存在しない名前が設定されていたら、クローンモードを解除する
					if(GetKCloneFromObjName(target->clone_data.clone_name, false)==NULL){
						target->clone_data.clonemode = 0;
						target->clone_data.primitive_id = 0;
						target->clone_data.material_id = 0;
						//kptr->clone_data.copyclone = mdl.GetCloneCopy(g_Dp.szCloneName);
					}else{
						kptr->clone_data.copyclone = mdl.GetCloneCopy(target->clone_data.clone_name);
					}
					//親が変更されていたら, つなぎかえる必要がある.
					//つなぎかえる場合、クローンのクローンの名前は比較対象にしてはならない.

					//つなぎかえる処理
					//新しいやつにひっつく
					KClone* pNewMaster = GetKCloneFromObjName(g_Dp.szMasterName, false);
					if(pNewMaster == NULL) break;
					if(pNewMaster->child == NULL){
						pNewMaster->child = target;
					}else if(pNewMaster->sibling == NULL){
						pNewMaster->sibling = target;
					}else{
						RefreshCloneTree();
						return; //すぐ下の処理を実行させないため
					}

					//前に引っ付いてたやつから離れる
					KClone*	kcl	= mdl.GetCloneAllocPtr();
					long allocnum = mdl.GetCloneAllocNum();
					if(kcl==NULL) return;

					for(int i=0; i<allocnum; i++)
					{
						if(&kcl[i] != target && (&kcl[i])!=pNewMaster)
						{
							if((kcl[i].child   == target)) kcl[i].child   = target->sibling;
							if((kcl[i].sibling == target)) kcl[i].sibling = target->sibling;
						}
					}
				}
			}else{
				return;
			}
			RefreshCloneTree();
			break;
		}
		case ID_FILE_IMPORT:
		{
			CallWindowProc((WNDPROC)GetWindowLong(hMainWnd, GWL_WNDPROC), hMainWnd, WM_COMMAND, wParam, lParam);
			break;
		}
		default:
			OutTraceLine();
			OutTrace("CloneCommand() : Nothing to do.");
			OutTrace("wParam -> %d", wParam);
			OutTrace("LOWORD(wParam) -> %d", LOWORD(wParam));
			OutTrace("lParam -> %d", lParam);
	}
/*
	//EDITの場合は、変更を保存する前に内部データが破壊されれてしまう.
	if(LOWORD(wParam)!=ID_ITEMVIEW_EDITNAME)
	{
		OutTraceLine();
		OutTrace("RefreshCloneTree()");
		int i=LOWORD(wParam);
		RefreshCloneTree();
	}
*/
	InvalidateRect(hMainWnd, NULL, FALSE);
}

KClone* GetKCloneFromObjName(const char* szCloneName, bool isSearchClone)
{
	KClone*	kcl	= mdl.GetCloneAllocPtr();
	long allocnum = mdl.GetCloneAllocNum();
	if(kcl==NULL) return NULL;

	for(int i=0; i<allocnum; i++)
	{
		if(!isSearchClone){
			//クローンモードは排除
			if(kcl[i].clone_data.clonemode != 1)
			{
				if(lstrcmp(kcl[i].clone_data.clone_name, szCloneName)==0)
				{
					return &kcl[i];
				}
			}
		}
		else{
			//クローンモードだけ検索
			if(kcl[i].clone_data.clonemode != 0)
			{
				if(lstrcmp(kcl[i].clone_data.clone_name, szCloneName)==0)
				{
					return &kcl[i];
				}
			}
		}
	}
	return NULL;
}





WNDPROC CloneOldProc[4];
HWND g_hCloneDlg;

static int val = 0;
void CloneOnMouseMove(int nCtrl, bool& isDragging, int llim = 0, int ulim = 255)
{
	if(isDragging){
		POINT xy;
		GetCursorPos(&xy);
		ScreenToClient(GetDlgItem(g_hCloneDlg, nCtrl), &xy);
		static POINT last = {xy.x, xy.y};
		POINT now = {xy.x, xy.y};
		if(now.x < last.x)
		{
			val-=abs(last.x - now.x);
		}
		if(now.x > last.x)
		{
			val+=abs(now.x - last.x);
		}
		if(val<llim) val = llim;
		else if(val>ulim) val = ulim;
		SetDlgItemInt(g_hCloneDlg, nCtrl, val, FALSE);
		last = now;
	}
}

void CloneOnLButton(int nCtrl, bool isDown, bool& isDragging, bool& isFocused)
{
	if(isDown){
		if(!isFocused)
		{
			POINT pt;
			GetCursorPos(&pt);
			//ClientToScreen(GetDlgItem(g_hCloneDlg, nCtrl), &pt);
			if(WindowFromPoint(pt)==GetDlgItem(g_hCloneDlg, nCtrl))
			{
				SetFocus(GetDlgItem(g_hCloneDlg, nCtrl));
				isFocused = true;
				isDragging = true;
				char szBuf[64];
				GetDlgItemText(g_hCloneDlg, nCtrl, szBuf, 64);
				val = StrToInt(szBuf);

				SetCapture(GetDlgItem(g_hCloneDlg, nCtrl));
			}else{
				isFocused = false;
			}
		}
	}else{
		isFocused = false;
		isDragging = false;
		ReleaseCapture();
		val = 0;	
	}
}

void CloneOnWheel(int nCtrl, bool isDown, bool& isDragging, bool& isFocused, int llim = 0, int ulim = 255){
	POINT pt;
	::GetCursorPos(&pt);
	if(WindowFromPoint(pt)==GetDlgItem(g_hCloneDlg, nCtrl))
	{
		isFocused = true;
		char szBuf[64];
		GetDlgItemText(g_hCloneDlg, nCtrl, szBuf, 64);
		val = StrToInt(szBuf);
	}else{
		isFocused = false;
	}
	
	if(isFocused)
	{
		if(isDown)
			val++;
		else
			val--;
		if(val<llim) val = llim;
		else if(val>ulim) val = ulim;
		SetDlgItemInt(g_hCloneDlg, nCtrl, val, FALSE);
	}
}

LRESULT CALLBACK CloneModeProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool isFocused = false;
	static bool isDragging = false;
	
	switch(msg)
	{
		case WM_MOUSEWHEEL:
		{
			CloneOnWheel(IDC_CLONE_MODE, ((int)wParam>0), isDragging, isFocused, 0, 1);
		}
		case WM_MOUSEMOVE:
		{
			CloneOnMouseMove(IDC_CLONE_MODE, isDragging, 0, 1);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			CloneOnLButton(IDC_CLONE_MODE, TRUE, isDragging, isFocused);
			break;
		}
		case WM_LBUTTONUP:
		{
			CloneOnLButton(IDC_CLONE_MODE, FALSE, isDragging, isFocused);
			break;
		}
	}
	return CallWindowProc(CloneOldProc[2], hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK ApplyMaterialProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool isFocused = false;
	static bool isDragging = false;
	
	switch(msg)
	{
		case WM_MOUSEWHEEL:
		{
			CloneOnWheel(IDC_CLONE_MATERIAL_ID, ((int)wParam>0), isDragging, isFocused);
		}
		case WM_MOUSEMOVE:
		{
			CloneOnMouseMove(IDC_CLONE_MATERIAL_ID, isDragging);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			CloneOnLButton(IDC_CLONE_MATERIAL_ID, TRUE, isDragging, isFocused);
			break;
		}
		case WM_LBUTTONUP:
		{
			CloneOnLButton(IDC_MATERIAL_COLOR_R, FALSE, isDragging, isFocused);
			break;
		}
	}
	return CallWindowProc(CloneOldProc[1], hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK ApplyPrimitiveProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool isFocused = false;
	static bool isDragging = false;
	
	switch(msg)
	{
		case WM_MOUSEWHEEL:
		{
			CloneOnWheel(IDC_CLONE_PRIMITIVE_ID, ((int)wParam>0), isDragging, isFocused);
		}
		case WM_MOUSEMOVE:
		{
			CloneOnMouseMove(IDC_CLONE_PRIMITIVE_ID, isDragging);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			CloneOnLButton(IDC_CLONE_PRIMITIVE_ID, TRUE, isDragging, isFocused);
			break;
		}
		case WM_LBUTTONUP:
		{
			CloneOnLButton(IDC_CLONE_PRIMITIVE_ID, FALSE, isDragging, isFocused);
			break;
		}
	}
	return CallWindowProc(CloneOldProc[0], hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK CloneDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	g_hCloneDlg = hWnd;

	switch(msg)
	{
	case WM_INITDIALOG:
	{
		KClone* target = GetKClone(vClone.GetSelectedItem());
		if(target == NULL)
		{
			EndDialog(hWnd, -1);
			return TRUE;
		}

		//現在の状態を表示
		SetDlgItemInt(hWnd, IDC_CLONE_PRIMITIVE_ID, target->clone_data.primitive_id, FALSE);
		SetDlgItemInt(hWnd, IDC_CLONE_MATERIAL_ID, target->clone_data.material_id, FALSE);
		SetDlgItemText(hWnd, IDC_CLONE_NAME, vClone.GetItemName(vClone.GetSelectedItem()));
		SetDlgItemInt(hWnd, IDC_CLONE_MODE, target->clone_data.clonemode, FALSE);
		if(CTreeViewToKClone(GetCTreeViewId(target))==0){ //KCloneの中のルートの場合,自分の上にいるのはダミーだから処理しない
			SetDlgItemText(hWnd, IDC_CLONE_MASTER_NAME, "KClone == GetTree()");
			EnableWindow(GetDlgItem(hWnd, IDC_CLONE_MASTER_NAME), FALSE);
		}else{
			KClone* pMaster = GetMaster(target);
			char* str = (pMaster==NULL) ? "" : vClone.GetItemName(GetCTreeViewId(pMaster));
			SetDlgItemText(hWnd, IDC_CLONE_MASTER_NAME, str);
		}

		//クローンのクローンなら,マテリアルとプリミティブは変更不能
		if(target->clone_data.clonemode>0)
		{
			EnableWindow(GetDlgItem(hWnd, IDC_CLONE_PRIMITIVE_ID), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_CLONE_MATERIAL_ID), FALSE);
			////ついでに、従属先の名前も.
			//EnableWindow(GetDlgItem(hWnd, IDC_CLONE_MASTER_NAME), FALSE);
		}


		//サブクラス化
		CloneOldProc[0] = (WNDPROC)GetWindowLong(GetDlgItem(hWnd, IDC_CLONE_PRIMITIVE_ID), GWL_WNDPROC);
		CloneOldProc[1] = (WNDPROC)GetWindowLong(GetDlgItem(hWnd, IDC_CLONE_MATERIAL_ID), GWL_WNDPROC);
		CloneOldProc[2] = (WNDPROC)GetWindowLong(GetDlgItem(hWnd, IDC_CLONE_MODE), GWL_WNDPROC);
		SetWindowLong(GetDlgItem(hWnd, IDC_CLONE_PRIMITIVE_ID), GWL_WNDPROC, (LONG)ApplyPrimitiveProc);
		SetWindowLong(GetDlgItem(hWnd, IDC_CLONE_MATERIAL_ID), GWL_WNDPROC, (LONG)ApplyMaterialProc);
		SetWindowLong(GetDlgItem(hWnd, IDC_CLONE_MODE), GWL_WNDPROC, (LONG)CloneModeProc);
		return TRUE;
	}
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case IDOK:
			{
				//Clone name
				char szStr[MAX_PATH];
				GetDlgItemText(hWnd, IDC_CLONE_NAME, g_Dp.szCloneName, MAX_PATH);
		
				//Clone Mode
				GetDlgItemText(hWnd, IDC_CLONE_MODE, szStr, MAX_PATH);
				g_Dp.nCloneMode = atoi(szStr);

				//Primitive ID
				GetDlgItemText(hWnd, IDC_CLONE_PRIMITIVE_ID, szStr, MAX_PATH);
				g_Dp.nPrimitiveId = atoi(szStr);
				
				//Material ID
				GetDlgItemText(hWnd, IDC_CLONE_MATERIAL_ID, szStr, MAX_PATH);
				g_Dp.nMaterialId = atoi(szStr);

				//Master Name
				GetDlgItemText(hWnd, IDC_CLONE_MASTER_NAME, g_Dp.szMasterName, MAX_PATH);

				//Exit Dialog
				EndDialog(hWnd, 0);
				return TRUE;
			}
			case IDCANCEL:
			{
				EndDialog(hWnd, -1);
				return TRUE;
			}
		break;
		}
	case WM_DESTROY:
	case WM_NCDESTROY:
		return TRUE;
	}
	return FALSE;
}

//線形検索で探す
int CloneGetKCloneId(KClone* pSrc)
{
	KClone*	kcl	= mdl.GetCloneAllocPtr();	//GetTreeでは線形検索は不可能
	long allocnum = mdl.GetCloneAllocNum();
	if(kcl==NULL) return -1;

	for(int i=0; i<allocnum; i++)
	{
		if(&kcl[i] == pSrc) return i;
	}
	return -1;
}

//従属先を探す
KClone* GetMaster(KClone* pSlave)
{
	KClone*	kcl	= mdl.GetCloneAllocPtr();	//GetTreeでは線形検索は不可能
	long allocnum = mdl.GetCloneAllocNum();
	if(kcl==NULL) return NULL;

	//if(pSlave == mdl.GetTree()) return pSlave;

	for(int i=0; i<allocnum; i++)
	{
		if(kcl[i].child == pSlave) return &kcl[i];
		else if(kcl[i].sibling == pSlave) return &kcl[i];
	}
	return NULL;
}

//従属しているヤツを探す
KClone* GetSlave(KClone* pMaster)
{
	KClone*	kcl	= mdl.GetCloneAllocPtr();	//GetTreeでは線形検索は不可能
	long allocnum = mdl.GetCloneAllocNum();
	if(kcl==NULL) return NULL;
	
	for(int i=0; i<allocnum; i++)
	{
		if(pMaster->child == &kcl[i]) return &kcl[i];
		else if(pMaster->sibling == &kcl[i]) return &kcl[i];
	}
	return NULL;
}

int CloneKey(UCHAR key, bool isDown)
{
	switch(key)
	{
	case VK_DELETE:
		CloneCommand(vClone.GetHolderWnd(), (WPARAM)ID_ITEMVIEW_DELETE, 0);
		break;
	case VK_RETURN:
		ModelerSelectClone(GetKClone(vClone.GetSelectedItem()));
		Render();
//		SetFocus(hMainWnd);
		break;
	}
	return FALSE;
}

int last_selection = 0;
int last_state		= 0;
int CloneMouse(long x, long y, UINT btn)
{
	if(isDragging)
	{
		int nDest = vClone.GetItemFromPos(x,y);
		if(nDest<0) return FALSE;
		vClone.SelectItem(nDest);
		if(btn == WM_LBUTTONUP)
		{
			CloneSelectedItemNotify(nDest, 0, 0, FALSE);
			isDragging = FALSE;
			SetClassLong(vClone.GetTreeWnd(),GCL_HCURSOR,(LONG)hOldCursor);
			SetCursor(hOldCursor);

			CloneReConnect(nDest);
			RefreshCloneTree();
			//CloneInsertItem(nDest);
		}
		//Render();
	}
	if(!isDragging)
	{
		int nDest = vClone.GetItemFromPos(x,y);
		if(nDest<0) return FALSE;
//        vClone.SelectItem(nDest);
		int vstate = vClone.IsChecked(nDest);
		
		//win.CSetWindowText(hMainWnd, "pos : %d , checked : %d last_selection:%d %d", nDest, vstate, last_selection, last_state);
		//if(CTreeViewToKClone(nDest)==-1){ //ファイル名
		//	for(int i=0; i<vClone.GetItemNum(); i++)
		//	{
		//		if(i==0) continue;
		//		KClone* p = GetKClone(i+1);
		//		if(!p) continue;
		//		
		//		if(last_selection == nDest && vstate != last_state){
		//			vClone.SetCheck(i+1, vstate);
		//			p->clone_data.visible = !vstate;
		//		}
		//	}
		//}else
		{
			KClone* p = GetKClone(nDest);
			if(!p) return FALSE;
			p->clone_data.visible = !vstate;
			if(last_selection == nDest && vstate != last_state) vClone.SelectItem(last_selection);
		}
		last_selection = nDest;
		last_state = vstate;
	}
	return FALSE;
}

void CloneSelectedItemNotify(int nDest, bool isByMouse, int nOrg, bool isByDrag)
{
	if(isByMouse)
	{
	}
//	SetFocus(vClone.GetTreeWnd());
//	if(isByMouse)//マウスで選択された
//	{
	//if(KCloneToCTreeView(nDest)!=0)
	if(nDest!=1)
	{
		//この関数を抜けるまで、選択状態は更新されないのでvClone.GetSelectedItem()は使用不可能.
		ModelerSelectClone(GetKClone(nDest)); //モデラに通知
		Render();
	}
//	}
	//if(isDragging)
	//{
	//	vClone.SelectItem(nDest);
	//}
	if(isByDrag){
		HCURSOR hCursor = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_DRAG));
		SetClassLong(vClone.GetTreeWnd(),GCL_HCURSOR,(LONG)hCursor);
		hOldCursor = (HCURSOR)SetCursor(hCursor);
		
		DraggingFrom = nDest;
		isDragging = TRUE;
	}
	//win.CSetWindowText(hMainWnd, "State:%s  Selecting:%d Original:%d MaxCount:%d", (isDragging) ? "Dragging":"Not Dragging", nDest, DraggingFrom, vClone.GetAmount());
	return;
}

void GetClone()
{
	RefreshCloneTree();
}

int CloneGetSelectedItem()
{
	return vClone.GetSelectedItem();
}