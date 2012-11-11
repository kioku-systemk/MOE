#include "stdafx.h"
#include "SceneList.h"
#include "CDemo.h"
#include "SceneObjectList.h"
#include "StartupCode.h"
#include "LayouterWindow.h"
CListView vSceneList;
extern CWindow win;
extern CWindowGL wingl;
extern CDemo demo;
extern long nCurrentMode;


static long selected_scene = -1;
static bool isUpdatingScene=false;
static int scene_oldselection = -1;
extern BOOL isPlaying;

void FreeSceneList()
{
	ClearSceneList(NULL);
	selected_scene = -1;
	isUpdatingScene = false;
	scene_oldselection = -1;
}

long GetSelectedScene(){ 
	//InvalidateRect(hMainWnd, NULL, FALSE);
	return selected_scene; }

void AddSceneList(HWND hListView, int nOrder, char* str)
{
	vSceneList.AddItem(str, nOrder);
}

void ClearSceneList(HWND hListView)
{
	vSceneList.DeleteAllItem();
}

//リフレッシュコード
void RefreshSceneList()
{
	selected_scene = -1;
	isUpdatingScene = true;
	int item = vSceneList.GetSelectedItem();

	ClearSceneList(vSceneList.GetListWnd());
	long i=0;
	char ilist[128];
	vector< CScene >::iterator it,eit=demo.scene.end();
	for(it=demo.scene.begin(); it!=eit; it++, i++){
		sprintf(ilist,"%s",it->scenename.c_str());
		AddSceneList(vSceneList.GetListWnd(), i, ilist);
	}
	isUpdatingScene = false;
	
	if(item<0) item = 0;
	if(item>=i) item = i;
	vSceneList.SelectItem(item);
	vSceneList.EnsureVisible(item);
	//RefreshSceneObjectList();
}

int SceneListNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit)
{
	if(selected_scene<0) return FALSE;
	if(lstrlen(szNewName)==0) return FALSE;
	if(!isAboutToEdit){
		SetUndo();
		demo.scene[selected_scene].scenename = szNewName;
	}
	return TRUE;
}

int SceneListKey(UCHAR key, bool isDown)
{
	//CallWindowProc((WNDPROC)GetWindowLong(hGLWnd, GWL_WNDPROC), hGLWnd, msg, wParam, lParam);
	return FALSE;
}

void SceneListSelectedItemNotify(int index, bool isByMouse)
{
	if(isUpdatingScene) return;
	//if(index<0) return; 
	selected_scene = index;
	
	//get fscene_time
	//vector<CScene>::iterator it, eit=demo.scene.end();
	//for(it=demo.scene.begin(); it!=eit; it++){
	//	if(it==demo.scene.begin()) scene_endtime.push_back(it->fscene_time);
	//	else				  scene_endtime.push_back(it->fscene_time + scene_endtime[(long)scene_endtime.size()-1]);
	//}
	long sn = GetSelectedScene();
	if(sn!=-1){
		UpdateSceneTime(demo.scene[sn].fscene_time);
	}
}

void SceneListPopup(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if(isPlaying) return;
	//if(nCurrentMode==LAYOUTER){
		LV_HITTESTINFO lvhtst;
		HWND hList = vSceneList.GetListWnd();

		lvhtst.pt.x	= LOWORD(lParam);
		lvhtst.pt.y	= HIWORD(lParam);
		ScreenToClient(hList, &lvhtst.pt);
		int nDest =	ListView_HitTest(hList,	&lvhtst);
		HMENU hMenu, hSub;
		vSceneList.SelectItem(nDest);
		hMenu =	LoadMenu(GetModuleHandle(NULL), nCurrentMode == LAYOUTER ? "SCENELIST_POPUP" : "SCENELIST_TIMELINER_POPUP");//GetMenu(hWnd);
		hSub = GetSubMenu(hMenu, 0);
		if(nDest<0){
			int nLim = GetMenuItemCount(hSub);
			for(int i=1; i<nLim; i++){
				//disable entire menu commands except for 'add'
				EnableMenuItem(hSub, i, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
			}
		}else{
			int nItem = vSceneList.GetItemCount();
			if(nItem==1){
				//disable menu command 'move up' && 'move down'
				EnableMenuItem(hSub, 4, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
				EnableMenuItem(hSub, 5, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
			}
		}
		TrackPopupMenu(hSub, TPM_LEFTALIGN, (int)LOWORD(lParam), (int)HIWORD(lParam), 0, hWnd,	NULL);
	//}
}

int SceneListMouse(long x, long y, UINT msg)
{
	if(isPlaying) return FALSE;
    switch(msg){
		//case WM_RBUTTONDOWN:
		//{
		//	POINT pt = {x,y};
		//	ClientToScreen(vSceneList.GetHolderWnd(), &pt);
		//	SceneListPopup(vSceneList.GetHolderWnd(), (WPARAM)0, (LPARAM)MAKELPARAM(pt.x, pt.y));
		//	break;
		//}
		case WM_LBUTTONDOWN:
		{
			selected_scene = vSceneList.GetItemFromPos(x,y);
			if(scene_oldselection!=selected_scene){
				RefreshSceneObjectList(); //リフレッシュコードに引っかからないように
				Render();
				scene_oldselection = selected_scene;
			}
			break;
		}
	}
	return FALSE;
}

void SceneListCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if(isPlaying) return;
	switch(LOWORD(wParam))
	{
		case ID_SCENELIST_ADD:
		{
			SetUndo();

			CScene scn(demo.anim_buffer);
			CSceneObject sco;
			sco.model_num = -1;
			sco.model = demo.GetCameraPtr();
			sco.anim.anim_time.push_back(0.0f);
			sco.anim.anim_time.push_back(1.0f);
			KCloneData kd;
			vector<KCloneData> kcd;
			kcd.push_back(kd);//0.0f
			kcd.push_back(kd);//1.0f
			long i,pm = demo.GetCameraPtr()->GetCloneAllocNum();
			for(i=0; i<pm; i++){
				sco.anim.anim.push_back(kcd);
			}
			scn.sceneobj.push_back(sco);
			demo.scene.push_back(scn);
			RefreshSceneList();
			//RefreshAllView();
            break;
		}
		case ID_SCENELIST_DELETE:
		{
			int item = vSceneList.GetSelectedItem();
			if(item<0) return;
			SetUndo();

			vector<CScene>::iterator it=demo.scene.begin() + item;
			demo.scene.erase (it);
			RefreshSceneList();
			//RefreshAllView();
			break;
		}
		case ID_SCENELIST_MOVEUP:
		{
			int item = vSceneList.GetSelectedItem();
			if(item<=0) return;//0番目はUPできない
			SetUndo();

			CScene tmp = demo.scene[item];
			demo.scene[item] = demo.scene[item-1];
			demo.scene[item-1] = tmp;
			vSceneList.SelectItem(item-1);
			RefreshSceneList();
			break;
		}
		case ID_SCENELIST_MOVEDOWN:
		{
			int item = vSceneList.GetSelectedItem();
			if((unsigned long)item>=demo.scene.size()-1) return;//最後のはDOWNできない
			SetUndo();

			CScene tmp = demo.scene[item];
			demo.scene[item] = demo.scene[item+1];
			demo.scene[item+1] = tmp;
			vSceneList.SelectItem(item+1);
			RefreshSceneList();
			break;
		}
		case ID_SCENELIST_COPY:
		{
			long sc = GetSelectedScene();
			int item = vSceneList.GetSelectedItem();
			if(item!=-1){
				SetUndo();
				demo.scene.push_back(demo.scene[item]);
				vSceneList.SelectItem(demo.scene.size()-1);
				RefreshSceneList();
			}
			break;
		}
	}
}

LRESULT CALLBACK SceneListHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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