#include "stdafx.h"
#include "CloneWindow.h"
#include "SceneList.h"
#include "SceneObjectList.h"
#include "CDemo.h"
#include "StartupCode.h"
//#include "LayoutFileManager.h"
#include "LayouterWindow.h"

extern CWindow win;
extern CWindowGL wingl;
extern char g_szLayoutName[1024];
CTreeView vClone;
extern CDemo demo;

//-1: KCloneTree形式への変換 -1:ファイル名を表示している分、減算
#define CTreeViewToKClone(tv) (tv-2)
#define KCloneToCTreeView(kcl) (kcl+2)

static int last_selection = 0;
static int last_state		= 0;

void FreeCloneWindow(){
	vClone.DeleteAllItem();
	//before_allocnum = -1;
	//prev_kcl = NULL;
	last_selection = 0;
	last_state		= 0;
}

void AddCloneTree(HWND hTreeView, int nOrderInKClone, char* str)
{
	vClone.AddItem(str, (nOrderInKClone>0) ? KCloneToCTreeView(nOrderInKClone) : nOrderInKClone);
}

//KCloneのポインタから、CTreeViewより返却されるIDに変換する
int GetCTreeViewId(KClone* kcl)
{
	long scene = GetSelectedScene();
	if(scene<0) return 0;
	long scene_object = GetSelectedSceneObject();
	if(scene_object<0) return 0;

	KClone** node_ptr=(KClone**)GlobalAlloc(GPTR,256*sizeof(KClone*));
	KClone*	kpt=demo.scene[scene].sceneobj[scene_object].model->GetTree();
	if(kpt == NULL)	return NULL;

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
			if(kpt==NULL) kpt = demo.scene[scene].sceneobj[scene_object].model->GetTree()->sibling;
			if(kpt==NULL) break;
			child_end=1;
		}
	}
	GlobalFree(node_ptr);
	return -1;
}

void SaveItemState()
{
	long scene = GetSelectedScene();
	if(scene<0) return;
	long scene_object = GetSelectedSceneObject();
	if(scene_object<0) return;
	
	KModelEdit* m = demo.scene[scene].sceneobj[scene_object].model;
	long allocnum = 0;
	KClone* kcl = NULL;
	if(m){
		allocnum =	m->GetCloneAllocNum();
		kcl =		m->GetCloneAllocPtr();
	}
	if(kcl==NULL) return;

	//for(int i=0; i<vClone.GetItemNum(); i++)
	for(int i=0; i<allocnum; i++)
	{
		//if(i==0) continue;
		//if(CTreeViewToKClone(i+1)<before_allocnum){
		//	//if(kcl==prev_kcl)
		//	//	kcl[CTreeViewToKClone(i+1)].clone_data.tree_collapse = (vClone.IsExpand(i+1)) ? 0:1;
		//	//else
		//	//	kcl[CTreeViewToKClone(i+1)].clone_data.tree_collapse = 0;
		//	kcl[CTreeViewToKClone(i+1)].clone_data.tree_collapse = (vClone.IsExpand(i+1)) ? 0:1;
		//}
		kcl[i].clone_data.tree_collapse = !vClone.IsExpand(2+i/*GetCTreeViewId(&kcl[i])*/);
	}
	//prev_kcl = kcl;
}

void ApplyItemState()
{
	long scene = GetSelectedScene();
	if(scene<0) return;
	long scene_object = GetSelectedSceneObject();
	if(scene_object<0) return;

	KModelEdit* m = demo.scene[scene].sceneobj[scene_object].model;
	long allocnum = 0;
	KClone*	kcl = NULL;
	if(m){
		allocnum =	m->GetCloneAllocNum();
		kcl	=		m->GetCloneAllocPtr();
	}
	if(kcl==NULL) return;

	for(int i=0; i<allocnum; i++)
	{
		//if(allocnum==before_allocnum){
		vClone.ExpandItem(2+i/*GetCTreeViewId(&kcl[i])*/, !kcl[i].clone_data.tree_collapse);
		//vClone.ExpandItem(KCloneToCTreeView(i), !kcl[i].clone_data.tree_collapse);
		//}else{
		//	vClone.ExpandItem(KCloneToCTreeView(i), TRUE);
		//}
	}
}

void ClearCloneTree(HWND hTreeView)
{
	vClone.DeleteAllItem();
}

KClone* GetKClone(int nDest)
{
	nDest = CTreeViewToKClone(nDest);
	if(nDest<0) return NULL;
	long scene = GetSelectedScene();
	if(scene<0) return NULL;
	long scene_object = GetSelectedSceneObject();
	if(scene_object<0) return NULL;

	if(&demo.scene[scene].sceneobj[scene_object]==NULL) return NULL;
	KModelEdit* m = demo.scene[scene].sceneobj[scene_object].model; 
	if(!m) return NULL;
	KClone* pKcl = m->GetCloneAllocPtr();
	return &pKcl[nDest];
}

KClone* GetSelectedClone() //Layouterでは,データは線形上に並んでいる
{
	long index = CloneGetSelectedItem();
	if(index<0) return NULL;
	long scene = GetSelectedScene();
	if(scene<0) return NULL;
	long scene_object = GetSelectedSceneObject();
	if(scene_object<0) return NULL;
	if(&demo.scene[scene].sceneobj[scene_object]==NULL) return NULL;
	KModelEdit* m = demo.scene[scene].sceneobj[scene_object].model; 
	if(!m) return NULL;
	KClone* pKcl = m->GetCloneAllocPtr();
	return &pKcl[index];
}

//更新処理
void RefreshCloneTree(KModelEdit* mdl, const char* szModelName, int select)
{
	//static int old_sceneobject = GetSelectedSceneObject();
	//int current_sceneobject = GetSelectedSceneObject();
	//if(old_sceneobject!=current_sceneobject){
	//	old_sceneobject = current_sceneobject
	//}else{
		SaveItemState();
	//}
	//以前のデータをクリア
	ClearCloneTree(vClone.GetTreeWnd());

	//ファイル名表示部分
	if(szModelName!=NULL)
	{
		char* szTitle = (char*)GlobalAlloc(GPTR, sizeof(char) * (lstrlen(szModelName)+1));
			lstrcpy(szTitle, szModelName);
			PathStripPath(szTitle);
			vClone.AddItem(szTitle, 0);
		GlobalFree(szTitle);
	}else vClone.AddItem("(null)", 0);
	
	KClone** node_ptr=(KClone**)GlobalAlloc(GPTR,256*sizeof(KClone*));
	KClone*	kpt=mdl->GetTree();
	if(kpt == NULL)	return;
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
			if(kpt==NULL) kpt = mdl->GetTree()->sibling;
			if(kpt==NULL) break;
			child_end=1;
		}
	}
	GlobalFree(node_ptr);
	GlobalFree(node_id);

	long scene = GetSelectedScene();
	if(scene<0) return;
	long scene_object = GetSelectedSceneObject();
	if(scene_object<0) return;

	KModelEdit* m = demo.scene[scene].sceneobj[scene_object].model;
	long allocnum = 0;
	KClone*	kcl = NULL;
	if(m){
		allocnum =	m->GetCloneAllocNum();
		kcl	=		m->GetCloneAllocPtr();
	}
	if(kcl==NULL) return;
	for(int i=0; i<allocnum; i++){
		vClone.SetCheck(2+i, !(kcl[i].clone_data.visible));
	}
		//for(int i=0; i<vClone.GetItemNum(); i++)
		//{
		//	//if(i==0) continue;
		//	KClone* p = GetKClone(i+1);
		//	if(!p){
		//		continue;
		//	}
		//	vClone.SetCheck(i+1, !(p->clone_data.visible));
		//}
//	}
	ApplyItemState();

	if(select>=0){
		vClone.SelectItem(select+1);
	}
}

int CloneKey(UCHAR key, bool isDown)
{
	return FALSE;
}

//名称の変更は受け付けない
int CloneNameEdit(int nDest, const	TCHAR* szNewName, bool isAboutToEdit){ return FALSE; }

int CloneMouse(long x, long y, UINT btn)
{
	int nDest = vClone.GetItemFromPos(x,y);
	if(nDest<0) return FALSE;
//        vClone.SelectItem(nDest);
	
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
		if(CTreeViewToKClone(nDest)<0) return 0;
		int vstate = vClone.IsChecked(nDest);
		KClone* p = GetKClone(nDest);
		if(!p) return FALSE;
		p->clone_data.visible = !vstate;
		if(last_selection == nDest && vstate != last_state) vClone.SelectItem(last_selection);
		//win.CSetWindowText(hMainWnd, "pos : %d , checked : %d last_selection:%d %d %d", CTreeViewToKClone(nDest), vstate, last_selection, last_state, p->clone_data.visible);
		last_selection = nDest;
		last_state = vstate;
	}
	Render();
	return FALSE;
}

//指定されたKCloneインデックスに該当する項目を選択状態にする
void CloneSelectClone(int clone_index)
{
	int item = KCloneToCTreeView(clone_index);
	vClone.SelectItem(item);
	SetFocus(hMainWnd);
}

//現在選択されている項目に該当するKCloneのインデックスを返す
int CloneGetSelectedItem()
{
	int id = vClone.GetSelectedItem();
	if( id != -1 )
		return CTreeViewToKClone(id);
	else
		return -1;
}

//レイアウターに選択項目を通知する
void CloneSelectedItemNotify(int nDest, bool isByMouse, int nOrg, bool isDrag)
{
	if(CTreeViewToKClone(nDest)<0) return;

//	if(isByMouse)
	{
		//LayouterSelectClone(CTreeViewToKClone(nDest));
		Render();
		//レイアウターウィンドウに、現在選択されている項目の番号をKCloneのインデックスに変換して渡す
	}
	return;
}

LRESULT CALLBACK CloneHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CLOSE:
	case WM_DESTROY:
		VerifyOnExit();
		return FALSE;
	case WM_KEYDOWN:
	case WM_KEYUP:
		CallWindowProc((WNDPROC)GetWindowLong(hGLWnd, GWL_WNDPROC), hGLWnd, msg, wParam, lParam);
		return FALSE;
	}
	return TRUE;
}