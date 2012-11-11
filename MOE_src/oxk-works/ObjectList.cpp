#include "stdafx.h"
#include "ObjectList.h"
#include "CDemo.h"
#include "LayoutFileManager.h"
#include "SceneList.h"
#include "SceneObjectList.h"
#include "StartupCode.h"
#include "CloneWindow.h"

CListView vObjectList;
extern CWindow win;
extern CWindowGL wingl;
extern CDemo demo;

//static KClone* selected_object = NULL;
//
//KClone* GetSelectedObject(){ return selected_object; }
static int selected_object_num = -1;
static int last_selection = 0;
static int last_state		= 0;
int GetSelectedObjectNum(){ return selected_object_num; }

void FreeObjectList()
{
	ClearObjectList(NULL);
	RefreshObjectList();
	selected_object_num = -1;
	last_selection = 0;
	last_state		= 0;
}

void AddObjectList(HWND hListView, int nOrder, char* str)
{
	vObjectList.AddItem(str, nOrder);
}

void ClearObjectList(HWND hListView)
{
	vObjectList.DeleteAllItem();
}

//リフレッシュコード
void RefreshObjectList()
{
	int item = vObjectList.GetSelectedItem();

	selected_object_num = -1;
	ClearObjectList(vObjectList.GetListWnd());
	long i;
	char ilist[128];

	char* tmp = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
	for(i=0; i<256; i++){
		if(demo.obj_name[i]!="")
		{
			lstrcpy(tmp, demo.obj_name[i].c_str());
			PathStripPath(tmp);
			sprintf(ilist,"%d: %s ", i, tmp);
			AddObjectList(vObjectList.GetListWnd(), i, ilist);
			//long scene = GetSelectedScene();
			//if(scene<0) continue;
			//KClone* pKcl = demo.scene[scene].sceneobj[i].model->GetCloneAllocPtr();
			//if(!pKcl) continue;
			//ListView_SetCheckState(vObjectList.GetListWnd(), i, !pKcl->clone_data.visible);
			//pKcl->clone_data.visible = 1;
		}else{
			sprintf(ilist,"%d: %s ", i, "-");
			AddObjectList(vObjectList.GetListWnd(), i, ilist);
		}
	}
	GlobalFree(tmp);

	if(item<0) item = 0;
	vObjectList.EnsureVisible(item);
	vObjectList.SelectItem(item);
}

//名称変更はだめ
int ObjectListNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit)
{
	return FALSE;
}

int ObjectListKey(UCHAR key, bool isDown)
{
	//CallWindowProc((WNDPROC)GetWindowLong(hGLWnd, GWL_WNDPROC), hGLWnd, isDown ? WM_KEYDOWN : WM_KEYUP, (WPARAM)key, 0);
	return FALSE;
}

void ObjectListSelectedItemNotify(int index, bool isByMouse)
{
	selected_object_num = index;
	//long i = GetSelectedScene();
	//if(i<0) return;
	//KClone* pKcl = demo.scene[i].sceneobj[index].model->GetCloneAllocPtr();
	//selected_object = &pKcl[index];
}

void ObjectListPopup(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LV_HITTESTINFO lvhtst;
	HWND hList = vObjectList.GetListWnd();

	lvhtst.pt.x	= LOWORD(lParam);
	lvhtst.pt.y	= HIWORD(lParam);
	ScreenToClient(hList, &lvhtst.pt);
	int nDest =	ListView_HitTest(hList,	&lvhtst);
	if(nDest>=0){
		HMENU hMenu, hSub;

		vObjectList.SelectItem(nDest);
		hMenu =	LoadMenu(GetModuleHandle(NULL), "OBJECTLIST_POPUP");//GetMenu(hWnd);
		hSub = GetSubMenu(hMenu, 0);
		TrackPopupMenu(hSub, TPM_LEFTALIGN, (int)LOWORD(lParam), (int)HIWORD(lParam), 0, hWnd,	NULL);
	}
}

int ObjectListMouse(long x, long y, UINT msg)
{
	int nDest = vObjectList.GetItemFromPos(x,y);
	if(nDest<0) return FALSE;

	int vstate = vObjectList.IsChecked(nDest);
	//win.CSetWindowText(hMainWnd, "pos : %d , checked : %d last_selection:%d %d", nDest, vstate, last_selection, last_state);
	//long i = GetSelectedScene();
	//if(i<0) return 0;
	//KModelEdit* mdl = demo.obj[nDest];
	//if(!mdl) return 0;
	//KClone* pKcl = mdl->GetCloneAllocPtr();
	//pKcl[nDest].clone_data.visible = !vstate;
	//if(last_selection == nDest && vstate != last_state) vObjectList.SelectItem(last_selection);
	//last_selection = nDest;
	//last_state = vstate;

	switch(msg)
	{
		case WM_LBUTTONDBLCLK:
		{
			int item = vObjectList.GetSelectedItem();
			int sc = GetSelectedScene();
			if(item<0 || sc<0) break;
			//------------------------------------------------
			if(demo.obj[item]!=NULL){
				CSceneObject sco;
				sco.model_num = item;
				sco.model = demo.obj[item];
				sco.anim.anim_time.push_back(0.0f);
				sco.anim.anim_time.push_back(1.0f);
				KCloneData kd;
				vector<KCloneData> kcd;
				kcd.push_back(kd);//0.0f
				kcd.push_back(kd);//1.0f
				long i,pm = demo.obj[item]->GetCloneAllocNum();
				for(i=0; i<pm; i++){
					sco.anim.anim.push_back(kcd);
				}
				demo.scene[sc].sceneobj.push_back(sco);
				RefreshSceneObjectList();
			}
			//---------------------------------------------------
			break;
		}
	}
	//HMENU hMenu, hSub;
	//hMenu =	LoadMenu(GetModuleHandle(NULL), "OBJECTLIST_POPUP");//GetMenu(hWnd);
	//hSub = GetSubMenu(hMenu, 0);
	//TrackPopupMenu(hSub, TPM_LEFTALIGN, (int)x, (int)y, 0, vObjectList.GetHolderWnd(), NULL);
	return FALSE;
}

void ObjectListCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
		case ID_OBJLIST_RELOAD:
		{
			int item = vObjectList.GetSelectedItem();
			int sindex = GetSelectedScene();
			int soindex = GetSelectedSceneObject();
			if(sindex<0 || soindex<0) return;

			char* szFileName = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
			lstrcpy(szFileName,	demo.obj_name[item].c_str());
			demo.FreePrimitive(item);
			demo.LoadObject(item, szFileName);

			int sc = GetSelectedScene();
			if(item<0 || sc<0) break;

			KModelEdit* pMdl = demo.scene[sindex].sceneobj[soindex].model;
			RefreshCloneTree(pMdl, demo.obj_name[item].c_str(), 1);

			GlobalFree(szFileName);
			RefreshObjectList();
		}
		break;
		case ID_OBJLIST_ADD:
		{
			char* szFileName = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
			if(GetOpenFileNameSingle(hWnd, "kmd", szFileName, FALSE))
			{
				int item = vObjectList.GetSelectedItem();
				if(demo.obj[item]!=NULL){
                    demo.FreePrimitive(item);
				}
				demo.LoadObject(item, szFileName);
				RefreshObjectList();
			}
			GlobalFree(szFileName);
			break;
		}
		case ID_OBJLIST_DELETE:
		{
			int item = vObjectList.GetSelectedItem();
			demo.FreePrimitive(item);
			RefreshObjectList();
			break;
		}
	}
}

LRESULT CALLBACK ObjectListHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//if(msg==WM_KEYDOWN || msg==WM_KEYUP)
	//	CallWindowProc((WNDPROC)GetWindowLong(hGLWnd, GWL_WNDPROC), hGLWnd, msg, wParam, lParam);
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
