#include "stdafx.h"
#include "SceneObjectList.h"
#include "cdemo.h"
#include "SceneList.h"
#include "CloneWindow.h"
#include "StartupCode.h"
#include "LayouterWindow.h"

CListView vSceneObjectList;
extern CWindow win;
extern CWindowGL wingl;
extern CDemo demo;

/* 内部からのアクセス用 */
int SceneObjectListNameEdit(int nDest, const TCHAR* szNewName);
int SceneObjectListKey(UCHAR key, bool isDown);

static long selected_sceneobj = -1;
static int last_selection = 0;
static int last_state		= 0;

void FreeSceneObjectList()
{
	ClearSceneObjectList(NULL);
	selected_sceneobj = -1;
	last_selection = 0;
	last_state		= 0;
}

long GetSelectedSceneObject(){ 
	//InvalidateRect(hMainWnd, NULL, FALSE);
	return selected_sceneobj;
}

void AddSceneObjectList(HWND hListView, int nOrder, const char* str)
{
	char* szTmp = (char*)GlobalAlloc(GPTR, sizeof(char) * (lstrlen(str)+1));
	lstrcpy(szTmp, str);
	PathStripPath(szTmp);
	vSceneObjectList.AddItem(szTmp, nOrder);
	GlobalFree(szTmp);
}

void ClearSceneObjectList(HWND hListView)
{
	vSceneObjectList.DeleteAllItem();
}

//リフレッシュコード
void RefreshSceneObjectList()
{
	int item = vSceneObjectList.GetSelectedItem();

	selected_sceneobj = -1;
	ClearSceneObjectList(vSceneObjectList.GetListWnd());
	long i=0;

	long index = GetSelectedScene();
	if(index<0) return;
	vector<CSceneObject>::iterator it, eit=demo.scene[index].sceneobj.end();
	for(it=demo.scene[index].sceneobj.begin(); it!=eit; it++,i++){
		//更新処理
		long m;
		for(m=0; m<256; m++){
			if(demo.obj[m]==(it->model)) break;
		}
		if(m==256){
			AddSceneObjectList(vSceneObjectList.GetListWnd(), i, "camera");
		}else{
			AddSceneObjectList(vSceneObjectList.GetListWnd(), i, demo.obj_name[m].c_str());
		}

		long sco = i;
		int is_cameratrans = demo.scene[index].sceneobj[sco].is_cameratrans;
		//if(is_cameratrans<2){
		//	vSceneObjectList.SetCheck(i, is_cameratrans==0 ? false : true);
		//}
		//if(is_cameratrans==0){
		//	vSceneObjectList.AddSubItem(i, 1, "no_cameratrans");
		//}else if(is_cameratrans==1){
		//	vSceneObjectList.AddSubItem(i, 1, "cameratrans");
		//}
		//vSceneObjectList.AddSubItem(i, 2, is_cameratrans==2 ? "NO_DOF" : "DOF");
		if(is_cameratrans==0){
			vSceneObjectList.AddSubItem(i, 1, "NO_CAM");
		}else if(is_cameratrans==1){
			vSceneObjectList.AddSubItem(i, 1, "-");
		}else if(is_cameratrans==2){
			vSceneObjectList.AddSubItem(i, 1, "NO_DOFCAM");
		}

		int interpolate = demo.scene[index].sceneobj[sco].interpolate;
		if(interpolate==0){
			vSceneObjectList.AddSubItem(i, 2, "Catmull-Rom");
		}else if(interpolate==1){
			vSceneObjectList.AddSubItem(i, 2, "Spline");
		}else if(interpolate==2){
			vSceneObjectList.AddSubItem(i, 2, "Linear");
		}
	}

	if(item<0 || demo.scene[index].sceneobj.size() <= (unsigned long)item) item = 0;
	vSceneObjectList.SelectItem(item);
	vSceneObjectList.EnsureVisible(item);


		selected_sceneobj = item;
		long m;
		KModelEdit* pMdl = demo.scene[index].sceneobj[item].model;
		for(m=0; m<256; m++){
			if(demo.obj[m]==(pMdl)) break;
		}
		if(m==256) 
			RefreshCloneTree(pMdl, "camera", 1);
		else
			RefreshCloneTree(pMdl, demo.obj_name[m].c_str(), 1);
}

//名称変更はだめ
int SceneObjectListNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit)
{
	return FALSE;
}

int SceneObjectListKey(UCHAR key, bool isDown)
{
	return FALSE;
}

void SceneObjectListSelectedItemNotify(int index, bool isByMouse)
{
	long i = GetSelectedScene();
	if(i<0) return;
	int item = index;
	if(item<0) return;

	if(&demo.scene[i].sceneobj[item]!=NULL)
	{
		KModelEdit* pMdl = demo.scene[i].sceneobj[item].model;
		if(pMdl){
			UpdateTimeline();//Update timeline
		}
	}
	selected_sceneobj = index;
}

void SceneObjectListPopup(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LV_HITTESTINFO lvhtst;
	HWND hList = vSceneObjectList.GetListWnd();

	lvhtst.pt.x	= LOWORD(lParam);
	lvhtst.pt.y	= HIWORD(lParam);
	ScreenToClient(hList, &lvhtst.pt);
	int nDest =	ListView_HitTest(hList,	&lvhtst);
	if(nDest>=0){
		HMENU hMenu, hSub;
		vSceneObjectList.SelectItem(nDest);
		hMenu =	LoadMenu(GetModuleHandle(NULL), "SCENEOBJECTLIST_POPUP");//GetMenu(hWnd);
		hSub = GetSubMenu(hMenu, 0);
		TrackPopupMenu(hSub, TPM_LEFTALIGN, (int)LOWORD(lParam), (int)HIWORD(lParam), 0, hWnd,	NULL);
	}
}

void ChangeInterpolateMethod(){
	long i = GetSelectedScene();
	if(i<0) return;
	int item = vSceneObjectList.GetSelectedItem();
	if(item<0) return;

	int interpolate = demo.scene[i].sceneobj[item].interpolate;
	if(interpolate==0){//catmul
		vSceneObjectList.AddSubItem(item, 2, "Spline");
		demo.scene[i].sceneobj[item].interpolate = 1;
	}else if(interpolate==1){
		vSceneObjectList.AddSubItem(item, 2, "Linear");
		demo.scene[i].sceneobj[item].interpolate = 2;
	}else if(interpolate==2){
		vSceneObjectList.AddSubItem(item, 2, "Catmull-Rom");
		demo.scene[i].sceneobj[item].interpolate = 0;
	}
	//Render();
}

void ToggleCameraTransChange(){
	long i = GetSelectedScene();
	if(i<0) return;
	int item = GetSelectedSceneObject();
	if(item<0) return;

	//SetUndo();
	int is_cameratrans = demo.scene[i].sceneobj[item].is_cameratrans;
	if(is_cameratrans==0){
		vSceneObjectList.AddSubItem(item, 1, "-");
		demo.scene[i].sceneobj[item].is_cameratrans = 1;
	}else if(is_cameratrans==1){
		vSceneObjectList.AddSubItem(item, 1, "NO_DOFCAM");
		demo.scene[i].sceneobj[item].is_cameratrans = 2;
	}else if(is_cameratrans==2){
		vSceneObjectList.AddSubItem(item, 1, "NO_CAM");
		demo.scene[i].sceneobj[item].is_cameratrans = 0;
	}
	Render();
}

int SceneObjectListMouse(long x, long y, UINT msg)
{
	//POINT point = {x, y};
	//ClientToScreen(vSceneObjectList.GetHolderWnd(), &point);
	//x = point.x;
	//y = point.y;
	//POINT point = {x, y};
	//ScreenToClient(vSceneObjectList.GetHolderWnd(), &point);
	//x = point.x;
	//y = point.y;
	int item = vSceneObjectList.GetItemFromPos(x, y);	if(item<0)	return 0;
	long i = GetSelectedScene();						if(i<0)		return 0;

	//if(msg==WM_RBUTTONDOWN)
	//{
	//}
	if(msg==WM_LBUTTONDBLCLK){
		selected_sceneobj = item;
		if(&demo.scene[i].sceneobj[item]!=NULL){
		//	KModelEdit* pMdl = demo.scene[i].sceneobj[item].model;
		//	if(pMdl){
				ToggleCameraTransChange();
		//	}
		}
		//int is_cameratrans = demo.scene[i].sceneobj[item].is_cameratrans;
		//if(is_cameratrans==2){//NO DOFモードである場合は表示をFALSEにして,そのときのチェックボックスの状態を反映する
		//	vSceneObjectList.AddSubItem(item, 1, "DOF");
		//	demo.scene[i].sceneobj[item].is_cameratrans = vSceneObjectList.IsChecked(item);
		//}else{//DOFを受けるモードならNO_DOFモードにする
		//	vSceneObjectList.AddSubItem(item, 1, "NO_DOF");
		//	demo.scene[i].sceneobj[item].is_cameratrans = 2;
		//}
		//Render();//編集スクリーンに反映するために再描画
		return 0;
	}else if(msg==WM_LBUTTONDOWN){//選択されたオブジェクトの階層構造をCloneWindowに表示する
		selected_sceneobj = item;
		if(&demo.scene[i].sceneobj[item]!=NULL){
			KModelEdit* pMdl = demo.scene[i].sceneobj[item].model;
			if(pMdl){
				long m;
				for(m=0; m<256; m++){
					if(demo.obj[m]==(pMdl)) break;
				}
				if(m==256) RefreshCloneTree(pMdl, "camera", 1);
				else RefreshCloneTree(pMdl, demo.obj_name[m].c_str(), 1);
			}
		}
		//Render();
	}
	//if(msg==WM_LBUTTONUP){
		//int is_cameratrans = demo.scene[i].sceneobj[item].is_cameratrans;
		//if(is_cameratrans != 2){//NO_DOFモードの場合はチェックボックスの値を無視する
		//	int vstate = vSceneObjectList.IsChecked(item);//チェックボックスがON == カメラの座標変換を受ける(is_cameratrans = 1);
		//	if(is_cameratrans != vstate){
		//		demo.scene[i].sceneobj[item].is_cameratrans = vstate;
		//		vSceneObjectList.AddSubItem(item, 1, (vstate==0) ? "DOF" : "NO_DOF");
		//		Render();//編集スクリーンに反映するために再描画
		//	}
		//}/*else{
		//	vSceneObjectList.SetCheck(item, old_check);
		//}*/
	//win.CSetWindowText(win.CGethWnd(), "item = %d, is_cameratrans = %d", item, demo.scene[i].sceneobj[item].is_cameratrans);
	//}

	//last_selection = nDest;
	//last_state = vstate;

	//if(msg==WM_RBUTTONDOWN)
	//{
	//	HMENU hMenu, hSub;
	//	hMenu =	LoadMenu(GetModuleHandle(NULL), "SCENEOBJECTLIST_POPUP");//GetMenu(hWnd);
	//	hSub = GetSubMenu(hMenu, 0);
	//	TrackPopupMenu(hSub, TPM_LEFTALIGN, (int)x, (int)y, 0, vSceneObjectList.GetHolderWnd(), NULL);
	//}
	//Render();
//	RefreshSceneObjectList();
	return FALSE;
}

void SceneObjectListCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam))
	{
		case ID_SCENEOBJ_DELETE:
		{
			long sc = GetSelectedScene();
			if(sc<0) return;
			int item = vSceneObjectList.GetSelectedItem();
			if(item<0 || item==0) return; //カメラは消せない
			if(&demo.scene[sc].sceneobj[item]==NULL) return;
			if(demo.scene[sc].sceneobj[item].model==NULL) return;

			SetUndo();
			vector<CSceneObject>::iterator it=demo.scene[sc].sceneobj.begin() + item;
			demo.scene[sc].sceneobj.erase (it);
			RefreshSceneObjectList();

			break;
		}
		case ID_SCENEOBJ_MOVEUP:
		{
			long sc = GetSelectedScene();
			if(sc<0) return;
			int item = vSceneObjectList.GetSelectedItem();
			if(item<=1) return;//カメラよりUPできない
			if(&demo.scene[sc].sceneobj[item]==NULL) return;
			if(demo.scene[sc].sceneobj[item].model==NULL) return;

			SetUndo();
			CSceneObject tmp = demo.scene[sc].sceneobj[item];
			demo.scene[sc].sceneobj[item] = demo.scene[sc].sceneobj[item-1];
			demo.scene[sc].sceneobj[item-1] = tmp;
			vSceneObjectList.SelectItem(item-1);
			RefreshSceneObjectList();
			break;
		}
		case ID_SCENEOBJ_MOVEDOWN:
		{
			long sc = GetSelectedScene();
			if(sc<0) return;
			int item = vSceneObjectList.GetSelectedItem();
			if((unsigned long)item>=demo.scene[sc].sceneobj.size()-1) return;//最後のはDOWNできない
			if(item==0) return;//カメラはDOWNできない
			if(&demo.scene[sc].sceneobj[item]==NULL) return;
			if(demo.scene[sc].sceneobj[item].model==NULL) return;

			SetUndo();
			CSceneObject tmp = demo.scene[sc].sceneobj[item];
			demo.scene[sc].sceneobj[item] = demo.scene[sc].sceneobj[item+1];
			demo.scene[sc].sceneobj[item+1] = tmp;
			vSceneObjectList.SelectItem(item+1);
			RefreshSceneObjectList();
			break;
		}
		case ID_SCENEOBJ_COPY:
		{
			long sc = GetSelectedScene();
			if(sc<0) return;
			int item = vSceneObjectList.GetSelectedItem();
			if(item<1) return;//カメラはコピーできない
			if(&demo.scene[sc].sceneobj[item]==NULL) return;
			if(demo.scene[sc].sceneobj[item].model==NULL) return;

			SetUndo();
			demo.scene[sc].sceneobj.push_back(demo.scene[sc].sceneobj[item]);
			vSceneObjectList.SelectItem(demo.scene[sc].sceneobj.size()-1);
			RefreshSceneObjectList();
			break;
		}
		case ID_SCENEOBJ_RELOAD:
		{
/*			int item = selected_sceneobj;
			long i = GetSelectedScene();
			if(i<0) return;
			long m;
			KModelEdit* pMdl = demo.scene[i].sceneobj[item].model;
			for(m=0; m<256; m++){
				if(demo.obj[m]==(pMdl)) break;
			}
			if(m==256) RefreshCloneTree(pMdl, "camera", 1);
			else RefreshCloneTree(pMdl, demo.obj_name[m].c_str(), 1);*/			
		break;
		}
		//case ID_FILE_TOGGLEDOF:
		//{
		//	long i = GetSelectedScene();
		//	if(i<0) return;
		//	int item = vSceneObjectList.GetSelectedItem();
		//	if(item<0) return;

		//	int is_cameratrans = demo.scene[i].sceneobj[item].is_cameratrans;
		//	if(is_cameratrans==2){//NO DOFモードである場合は表示をFALSEにして,そのときのチェックボックスの状態を反映する
		//		vSceneObjectList.AddSubItem(item, 1, "DOF");
		//		//demo.scene[i].sceneobj[item].is_cameratrans = 
		//	}else{//DOFを受けるモードならNO_DOFモードにする
		//		vSceneObjectList.AddSubItem(item, 1, "NO_DOF");
		//		demo.scene[i].sceneobj[item].is_cameratrans = 2;
		//	}
		//	Render();//編集スクリーンに反映するために再描画
		//	break;
		//}
		case ID_FILE_TOGGLECAMERATRANS:{
			ToggleCameraTransChange();
			break;
		}
		case ID_SCENEOBJ_INTERPOLATE:
		{
			ChangeInterpolateMethod();
			break;							 
		}
	}
}

LRESULT CALLBACK SceneObjectListHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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