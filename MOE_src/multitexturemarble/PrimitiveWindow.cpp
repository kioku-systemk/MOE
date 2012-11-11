#include "stdafx.h"
#include "PrimitiveWindow.h"
#include "CloneWindow.h"
#include "FileOperation.h"
#include "Modeler Window.h"

extern KModelEdit mdl;
extern CWindowGL win;
CListView vPrimitive;

int selected_primitive = -1;

int PrimitiveGetSelectedPrimitive()
{
	return selected_primitive;
}

void AddPrimitiveList(HWND hListView, int nOrder, char* str)
{
	vPrimitive.AddItem(str, nOrder);
//	SendMessage(hListView, LB_ADDSTRING, 0, (LPARAM)str);
}

void ClearPrimitiveList(HWND hListView)
{
	vPrimitive.DeleteAllItem();
//	SendMessage(hListView,  LB_RESETCONTENT , 0, 0);
}

void RefreshPrimitiveList()
{
	int lastpos = vPrimitive.GetSelectedItem();
	ClearPrimitiveList(vPrimitive.GetListWnd());
	long i;
	char ilist[128];
	for(i=0; i<256; i++){
		KObject* obj = mdl.GetPrimitive(i);
		if(obj==NULL)	sprintf(ilist,"%d: %s ", i, "-");
		else			sprintf(ilist,"%d: %s ", i, obj->GetName());
		AddPrimitiveList(vPrimitive.GetListWnd(), i, ilist);
	}
	if(lastpos==-1) lastpos = 0;
	vPrimitive.SelectItem(lastpos);
	vPrimitive.EnsureVisible(lastpos);
}

//void RefreshCloneList()
//{
//	ClearList(IDC_CLONELIST);
//	vector<KClone>::iterator it;
//	vector<KClone>::iterator eit=clone_list.end();
//	for(it=clone_list.begin(); it!=eit; it++){
//		char cname[128];
//		char* str_visible[] = {"FALSE","TRUE "};
//		char* str_lock[] = {"","LOCK"};
//		sprintf(cname,"%3d : (%s) %s",it->primitive_id
//						,str_visible[clone_state[(long)(it-clone_list.begin())].visible]
//						,str_lock[clone_state[(long)(it-clone_list.begin())].lock]);
//		AddList(IDC_CLONELIST,cname);
//	}
//}

//マテリアルの名称が変更されていない場合に限り有効
int KObjectToListView(KObject* prm)
{
	for(int i=0; i<mdl.GetPrimitiveNum(); i++)
	{
		if(mdl.GetPrimitive(i) == prm)
		{
			return i;
		}
	}	
	return -1;
}

KObject* ListViewToKObject(int nLv)
{
	char szMat[MAX_PATH];
	lstrcpy(szMat, vPrimitive.GetItemName(nLv));
	int item = atoi(szMat);
	//OutTraceLine();
	//OutTrace("ListViewToKObject : %d %s %d", nLv, szMat, atoi(szMat));
	return mdl.GetPrimitive(item);
}

int ListViewIdToKObjectId(int nLv)
{
	char szMat[MAX_PATH];
	lstrcpy(szMat, vPrimitive.GetItemName(nLv));
	//OutTraceLine();
	//OutTrace("ListViewIdToKObjectId() : %d %s %d", nLv, szMat, atoi(szMat));
	return atoi(szMat);
}

int PrimitiveNameEdit(int nDest, const	TCHAR* szNewName, bool isAboutToEdit)
{
	return FALSE;
}

void PrimitivePopup(HWND hWnd, WPARAM wParam , LPARAM lParam)
{
	LV_HITTESTINFO lvhtst;
	HWND hList = vPrimitive.GetListWnd();

	lvhtst.pt.x	= LOWORD(lParam);
	lvhtst.pt.y	= HIWORD(lParam);
	ScreenToClient(hList, &lvhtst.pt);
	int nDest =	ListView_HitTest(hList,	&lvhtst);
	if(nDest>=0){
		HMENU hMenu, hSub;

		vPrimitive.SelectItem(nDest);
		hMenu =	LoadMenu(GetModuleHandle(NULL), "PRIMITIVEVIEW");//GetMenu(hWnd);
		hSub = GetSubMenu(hMenu, 0);
		TrackPopupMenu(hSub, TPM_LEFTALIGN, (int)LOWORD(lParam), (int)HIWORD(lParam), 0, hWnd,	NULL);
	}
}

void PrimitiveCommand(HWND hWnd, WPARAM	wParam,	LPARAM lParam)
{
	HWND hList = vPrimitive.GetListWnd();

	switch (LOWORD(wParam)){
		case ID_ITEMVIEW_LOAD:
		{
			//int item = vPrimitive.AddItem(NULL, -1);
			//OutTraceLine();
			//OutTrace("PrimitiveCommand() : Load Primitive : item -> %d", item);
			//OutTrace("PrimitiveCommand() : Load Primitive : KObjectID -> %d", ListViewIdToKObjectId(item));
			//mdl.FreePrimitive(ListViewIdToKObjectId(item));
			//mdl.PrimitiveLoadFromFile(ListViewIdToKObjectId(item), GetOpenFileNameSingle(vPrimitive.GetListWnd(), "KMB(Primitive) files(*.kmb)\0*.kmb\0All Files(*.*)\0*.*\0\0", "kmb", NULL, "Open KMB(Primitive) File"));
			
			char szPrimitiveFile[1024];
			if(GetOpenFileNameSingle(vPrimitive.GetListWnd(), "kmb", szPrimitiveFile)==FALSE) return;
			
			int item = selected_primitive;
			mdl.FreePrimitive(item);
			if(mdl.PrimitiveLoadFromFile(item, szPrimitiveFile)!=0) return;
			
			RefreshPrimitiveList();
			break;
		}
		case ID_ITEMVIEW_EDITNAME:
			break;
		case ID_ITEMVIEW_DELETE:
		{
			int item = selected_primitive;
			mdl.FreePrimitive(item);
			RefreshPrimitiveList();
			break;
		}
		case ID_ITEMVIEW_PROPERTY:
		{
			break;
			RefreshPrimitiveList();
		}
		default:
			OutTraceLine();
			OutTrace("PrimitiveCommand() : Nothing to do.");
			OutTrace("wParam -> %d", wParam);
			OutTrace("LOWORD(wParam) -> %d", LOWORD(wParam));
			OutTrace("lParam -> %d", lParam);
	}
	InvalidateRect(hMainWnd, NULL, FALSE);
}

int PrimitiveKey(UCHAR key, bool isDown)
{
	return FALSE;
}

int PrimitiveMouse(long x, long y, UINT btn)
{
	selected_primitive = vPrimitive.GetSelectedItem();
	if(btn==WM_LBUTTONDBLCLK)
	{
		if(mdl.GetPrimitive(selected_primitive)){
			CloneInsertItem(CloneGetSelectedItem(), selected_primitive);
			RefreshCloneTree();
		}
	}
	return FALSE;
}

void GetPrimitive()
{
	RefreshPrimitiveList();
	//long objnum	= mdl.GetPrimitiveNum();
	//KObject* obj;

	//for(int	i=0; i<objnum; i++)
	//{
	//	obj	= mdl.GetPrimitive(i);
	//	if(obj)
	//	{
	//		//char str[64];
	//		//wsprintf(str, "%d",	i);
	//		vPrimitive.AddItem(NULL, -1);
	//	}
	//}
}