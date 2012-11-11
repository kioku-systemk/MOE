#include "stdafx.h"
#include "StartupCode.h"
#include "FileOperation.h"
#include "modeler window.h"
#include "MaterialWindow.h"
#include "PrimitiveWindow.h"
#include "CloneWindow.h"

CWindowGL win;
KModelEdit mdl;
KTextureEdit ktex;
extern CTreeView vClone;
extern CListView vMaterial;
extern CListView vPrimitive;
extern char g_szModelName[1024];

void CreateCloneWindow(){
	RECT mainrect;
	GetClientRect(hMainWnd, &mainrect);
	int pos[2] = {0, 0};
	int size[2] = { 200, 240 };

	if(!FindWindowEx(hMainWnd, NULL, NULL, "Clone Object")){
		vClone.CreateTreeView(pos[0], pos[1],size[0], size[1], "Clone Object");
		vClone.SetCallbackFunctions(CloneNameEdit, ClonePopup, CloneCommand, CloneKey, CloneMouse, CloneSelectedItemNotify); 
		vClone.SetParent(hMainWnd);
		LONG oldStyle = GetWindowLong(vClone.GetTreeWnd(), GWL_STYLE);
		oldStyle |= TVS_CHECKBOXES;
		SetWindowLong(vClone.GetTreeWnd(), GWL_STYLE, oldStyle);
	//	SetWindowLong(vClone.GetHolderWnd(), GWL_EXSTYLE, (LONG)WS_EX_TOOLWINDOW);
	//	UpdateWindow(vClone.GetHolderWnd());
	}else{
		//ウィンドウサイズを保存する
		RECT rect;
		HWND myhWnd = FindWindowEx(hMainWnd, NULL, NULL, "Clone Object");
		GetWindowRect(myhWnd, &rect);
		//ScreenToClient(hMainWnd, (POINT*)pos);

		POINT minpoint = { rect.left, rect.top };
		ScreenToClient(hMainWnd, &minpoint);
		POINT maxpoint = { rect.right, rect.bottom };
		ScreenToClient(hMainWnd, &maxpoint);
		//GetWindowInfo(
		size[0] = maxpoint.x - minpoint.x;
		size[1] = maxpoint.y - minpoint.y;
		if(mainrect.right>0 && (pos[0] + size[0])>mainrect.right) //minimized problem
		{
			size[0] = mainrect.right-pos[0];
		}
		if(mainrect.bottom>0 && (pos[1] + size[1])>mainrect.bottom)
		{
			size[1] = mainrect.right-pos[1];
		}

		pos[0] = rect.right - rect.left;
		pos[1] = rect.bottom - rect.top;
	}

	SetWindowPos(vClone.GetHolderWnd(), 0, 0, 0, size[0], size[1], SWP_NOZORDER | SWP_NOMOVE);
	//MoveWindow(vClone.GetHolderWnd(), pos[0], pos[1], size[0], size[1], TRUE);
}

void CreateMaterialWindow()
{
	RECT mainrect;
	GetClientRect(hMainWnd, &mainrect);
	int pos[2] = {mainrect.right - 200, mainrect.bottom - 240};
	int size[2] = { 200, 240 };

	if(!FindWindowEx(hMainWnd, NULL, NULL, "Material")){
		vMaterial.CreateListView(pos[0], pos[1], size[0], size[1], "Material");
		vMaterial.AddColumn("Material",	320, 0);
		vMaterial.SetCallbackFunctions(MaterialNameEdit, MaterialPopup,	MaterialCommand, MaterialKey, MaterialMouse);
		vMaterial.SetParent(hMainWnd);
	}else{
		//ウィンドウサイズを保存する
		RECT rect;
		HWND myhWnd = FindWindowEx(hMainWnd, NULL, NULL, "Material");
		GetWindowRect(myhWnd, &rect);
		//ScreenToClient(hMainWnd, (POINT*)pos);

		POINT minpoint = { rect.left, rect.top };
		ScreenToClient(hMainWnd, &minpoint);
		POINT maxpoint = { rect.right, rect.bottom };
		ScreenToClient(hMainWnd, &maxpoint);
		//GetWindowInfo(
		size[0] = maxpoint.x - minpoint.x;
		size[1] = maxpoint.y - minpoint.y;
		if(mainrect.right>0 && (pos[0] + size[0])>mainrect.right) //minimized problem
		{
			size[0] = mainrect.right-pos[0];
		}
		if(mainrect.bottom>0 && (pos[1] + size[1])>mainrect.bottom)
		{
			size[1] = mainrect.right-pos[1];
		}

		pos[0] = rect.right - rect.left;
		pos[1] = rect.bottom - rect.top;
	}
	SetWindowPos(vMaterial.GetHolderWnd(), 0, 0, 0, size[0], size[1], SWP_NOZORDER | SWP_NOMOVE);
	//MoveWindow(vMaterial.GetHolderWnd(), pos[0], pos[1], size[0], size[1], TRUE);
}

void CreatePrimitiveWindow()
{
	RECT mainrect;
	GetClientRect(hMainWnd, &mainrect);
	int pos[2] = {mainrect.right - 200, 0};
	int size[2] = { 200, 240 };

	if(!FindWindowEx(hMainWnd, NULL, NULL, "Primitive")){
		vPrimitive.CreateListView(pos[0], pos[1], size[0], size[1], "Primitive");
		vPrimitive.AddColumn("Primitive", 320, 0);
		vPrimitive.SetCallbackFunctions(PrimitiveNameEdit, PrimitivePopup, PrimitiveCommand, PrimitiveKey, PrimitiveMouse);
		vPrimitive.SetParent(hMainWnd);
	}else{
		//ウィンドウサイズを保存する
		RECT rect;
		HWND myhWnd = FindWindowEx(hMainWnd, NULL, NULL, "Primitive");
		GetWindowRect(myhWnd, &rect);
		//ScreenToClient(hMainWnd, (POINT*)pos);

		POINT minpoint = { rect.left, rect.top };
		ScreenToClient(hMainWnd, &minpoint);
		POINT maxpoint = { rect.right, rect.bottom };
		ScreenToClient(hMainWnd, &maxpoint);
		//GetWindowInfo(
		size[0] = maxpoint.x - minpoint.x;
		size[1] = maxpoint.y - minpoint.y;
		if(mainrect.right>0 && (pos[0] + size[0])>mainrect.right) //minimized problem
		{
			size[0] = mainrect.right-pos[0];
		}
		if(mainrect.bottom>0 && (pos[1] + size[1])>mainrect.bottom)
		{
			size[1] = mainrect.right-pos[1];
		}

		pos[0] = rect.right - rect.left;
		pos[1] = rect.bottom - rect.top;
	}

	SetWindowPos(vPrimitive.GetHolderWnd(), 0, 0, 0, size[0], size[1], SWP_NOZORDER | SWP_NOMOVE);
	//MoveWindow(vPrimitive.GetHolderWnd(), pos[0], pos[1], size[0], size[1], TRUE);
}

void CreateToolWindow()
{
	CreateMaterialWindow();
	CreatePrimitiveWindow();
	CreateCloneWindow();
}

void Initialize()
{
	HMENU hMenu = GetMenu(hMainWnd);
	
	HMENU hFileMenu = GetSubMenu(hMenu, 0);
	EnableMenuItem(hFileMenu, 7, MF_BYPOSITION|MF_GRAYED);
	
	//HMENU hEditMenu = GetSubMenu(hMenu, 1);
	//EnableMenuItem(hMenu, 1, MF_BYPOSITION|MF_GRAYED);
	//EnableMenuItem(hEditMenu, 0, MF_BYPOSITION|MF_GRAYED);
	//EnableMenuItem(hEditMenu, 1, MF_BYPOSITION|MF_GRAYED);

	HMENU hViewMenu = GetSubMenu(hMenu, 2);
	////EnableMenuItem(hMenu, 2, MF_BYPOSITION|MF_GRAYED);
	CheckMenuItem(hViewMenu, 0, (FindWindowEx(hMainWnd, NULL, NULL, "Clone Object")!=NULL) ? MF_BYPOSITION|MF_CHECKED : MF_BYPOSITION|MF_UNCHECKED);
	CheckMenuItem(hViewMenu, 1, (FindWindowEx(hMainWnd, NULL, NULL, "Primitive")!=NULL) ? MF_BYPOSITION|MF_CHECKED : MF_BYPOSITION|MF_UNCHECKED );
	CheckMenuItem(hViewMenu, 2, (FindWindowEx(hMainWnd, NULL, NULL, "Material")!=NULL) ? MF_BYPOSITION|MF_CHECKED : MF_BYPOSITION|MF_UNCHECKED );
	//EnableMenuItem(hViewMenu, 0, MF_BYPOSITION|MF_GRAYED);
	//EnableMenuItem(hViewMenu, 1, MF_BYPOSITION|MF_GRAYED);
	//EnableMenuItem(hViewMenu, 2, MF_BYPOSITION|MF_GRAYED);

	//メインウィンドウをアクティブにする
	SetFocus(win.CGethWnd());
}

void ImportKMD(const char* szImportFile){
    KClone* cln = CloneInsertItem(CloneGetSelectedItem());
	
	if(cln==NULL){//選択されてない
		MessageBox(NULL, "選択されてない", 0, MB_ICONEXCLAMATION);	
	}else{
		KModelEdit kImp;
		kImp.LoadFromFile(szImportFile);
		
		KClone* master;
		if(cln!=mdl.GetTree()){ //既存の誰かにぶら下がる
			master = GetMaster(cln);	
		}else{//ファイル名の部分に追加した(cln==GetTree())
			master = cln;
		}
		unsigned long master_num = (unsigned long)(master - mdl.GetCloneAllocPtr());
		unsigned long dummy_num  = (unsigned long)(cln - mdl.GetCloneAllocPtr());
		mdl.Import(kImp,master_num,dummy_num);
		
		CloneDelete(cln); //for safety.
	}
	RefreshAllView();
}

void OnDraw(HDC hDC)
{
	Render();
}

void RebootMe(){
	int nArgc;
	int i;
	WCHAR** wszCmdLine = CommandLineToArgvW(GetCommandLineW(), &nArgc);
	int length;
	char** szArgv = (char**)GlobalAlloc(GPTR, sizeof(char*) * nArgc);
	for(i=0; i<nArgc; i++)
	{
		if((length = ::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, NULL, 0, NULL, NULL))==0) continue;
		szArgv[i] = (char*)GlobalAlloc(GPTR, sizeof(char) * (length+1));
		::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, szArgv[i], length, NULL, NULL);
	}

	WinExec(szArgv[0], SW_SHOWNORMAL);

	for(i=0; i<nArgc; i++){
		GlobalFree(szArgv[i]);
	}
	GlobalFree(szArgv);
}

int	cmain()
{
#ifndef _DEBUG
	__try
#endif
	{
		win.CCreateWindow(1024, 720, "", "MAINVIEW");
		win.CSetWindowText(hMainWnd, "Marble Modeler, ver.%4.3f (Build Date: %s %s), coded by kioku and c.r.v.", APP_VER, __DATE__, __TIME__);
		ShowCursor(TRUE);
		win.CSetCallbackFunctions(KeyEvent,	MouseEvent, OnDraw);
		win.CSetHookProcedure(WndProc);
		win.LoadAccelerators("MAINACCEL");
		HANDLE hIcon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
		SetClassLong(hMainWnd, GCL_HICON, (LONG)hIcon);

		CreateToolWindow();
		Initialize();
		InitModelerWindow();
		InitMaterialWindow();

		DragAcceptFiles(hMainWnd, TRUE);

		int nArgc;
		int i;
		WCHAR** wszCmdLine = CommandLineToArgvW(GetCommandLineW(), &nArgc);
		int length;
		char** szArgv = (char**)GlobalAlloc(GPTR, sizeof(char*) * nArgc);
		for(i=0; i<nArgc; i++)
		{
			if((length = ::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, NULL, 0, NULL, NULL))==0) continue;
			szArgv[i] = (char*)GlobalAlloc(GPTR, sizeof(char) * length);
			::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, szArgv[i], length, NULL, NULL);
		}

		if(nArgc>1){
			const char* kmd_szHeader = "KMD";
			char* header = (char*)GlobalAlloc(GPTR, sizeof(char) * (lstrlen(kmd_szHeader)+1));
				FILE* fp = fopen(szArgv[1] ,"rb");
					for(i=0; i<lstrlen(kmd_szHeader); i++) header[i] = fgetc(fp);
				fclose(fp);

				if(lstrcmp(header, kmd_szHeader)==0){
					if(mdl.LoadFromFile(szArgv[1])<0)
					{
						MessageBox(NULL, "ファイルを開くことができません.", "読み取りエラー", MB_SYSTEMMODAL);
						return FALSE;
					}
					lstrcpy(g_szModelName, szArgv[1]);
					RefreshAllView();
				}else{
					MessageBox(NULL, "This is not a KMD file!", 0, MB_SYSTEMMODAL);
					return FALSE;
				}
			GlobalFree(header);
		}else{//引数がなければ
			//起動時に生成
			mdl.Create();
			lstrcpy(g_szModelName, SZ_DEFAULT_FILENAME);
		}
		RefreshAllView();

		win.CMessageLoop();
		CloseTrace();
	}
#ifndef _DEBUG
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		__try
		{
			//MessageBox(NULL, "すみません。例外処理が発生しました。現在の情報をできるだけ吐き出します。", "c.r.v.", MB_SYSTEMMODAL);
			char szPath[256] = {'\0'};
			GetCurrentDirectory(sizeof(szPath), szPath);
			//GetTempPath(sizeof(szPath), szPath);
			PathRemoveBackslash(szPath);
			lstrcat(szPath, "\\dump.kmd");
			FILE* fp=fopen(szPath, "wb");
			if(fp==NULL){
				MessageBox(NULL, "ファイルのオープンに失敗しました。\nこの後すぐ，例外処理を恣意的に発生させます。", "c.r.v.", MB_OK);
				throw;
			}
			mdl.SaveKMD(fp);
			fclose(fp);

			char szMes[1024];
			wsprintf(szMes, "最新の情報を保存している最中に例外は発生しませんでした。\nただし，保存されたデータが破損している可能性がありますので，使用する際は十分に気をつけてください。\nデータは下記の場所に保存されました。\n%s\n\nこのメッセージボックスを閉じると，アプリケーションを再起動します。",szPath);
			MessageBox(NULL, szMes, "c.r.v.", MB_SYSTEMMODAL);
			RebootMe();
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			MessageBox(NULL, "申し訳ございません。最新の情報を保存している際に例外が発生しました。すべての作業データは失われます。", "c.r.v.", MB_SYSTEMMODAL); 
		}
	}
#endif
	return 0;
}

void RefreshAllView()
{
	RefreshCloneTree();
	RefreshMaterialList();
	RefreshPrimitiveList();
	//vClone.DeleteAllItem();
	//vPrimitive.DeleteAllItem();
	//vMaterial.DeleteAllItem();
}

void VerifyOnExit()
{
	if(MessageBox(hMainWnd, "Exit?", "Exit", MB_YESNO|MB_ICONQUESTION|MB_SYSTEMMODAL)==IDYES)
		PostQuitMessage(0);
}

//void PrepareForFileChange(bool isDeleteTextures = false)
//{
//	int i;
//	for(i=0; i<mdl.GetMaterialNum(); i++)
//	{
//		KMaterial* mat = mdl.GetMaterial(i);
//		if(mat!=NULL){
//			//if(isDeleteTextures){
//			//	if(mat->gltexure_num!=NULL){
//			//		glDeleteTextures(mat->texcnt, mat->gltexure_num);
//			//		mat->gltexure_num = NULL;
//			//	}
//			//}
//			if(mat->texture!=NULL){
//				for(i=0; i<mat->number_of_texture; i++){
//					GlobalFree(mat->texture[i]);
//				}
//				GlobalFree(mat->texture);
//
//				mat->texture = (char**)GlobalAlloc(GPTR,sizeof(char*) * 1);
//				mat->texture[0] = (char*)GlobalAlloc(GPTR,sizeof(char) * 1);
//			}
//		}
//	}
//}

//extern 
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_FILE_NEW_PROJECT:
				//SetUndo();
				//if(((long)undo_mdl.size()-1 > 0) || ((long)redo_mdl.size()-1 > 0)){
				//	if(MBQ("現在編集中のファイルを破棄し、新規ファイルを作成しますか？")==IDNO) return TRUE;
				//}
				//PrepareForFileChange();
				mdl.Free();
				mdl.Create();

				//SaveModel(&mdl);
				//mdl.LoadFromFile(g_szModelName);
				lstrcpy(g_szModelName, SZ_DEFAULT_FILENAME);
				RefreshAllView();
				break;
			case ID_FILE_OPEN_PROJECT:
				//PrepareForFileChange();
				//if(MBQ("現在編集中のファイルを破棄し、違うファイルをロードしますか？")==IDNO) return TRUE;
				(OpenModel(&mdl));
				break;
			case ID_FILE_SAVE_OW:
				//PrepareForFileChange();
				SaveModel(&mdl,false);
				break;
			case ID_FILE_SAVE_AS:
				//PrepareForFileChange();
				SaveModel(&mdl);
				//OpenModel(&mdl, false);
				break;
			case ID_FILE_IMPORT:{
				char szImportFile[1024];
				if(GetOpenFileNameSingle(hMainWnd, "kmd", szImportFile, TRUE)){
					ImportKMD(szImportFile);
				}
				break;
			}
			case ID_FILE_EXIT:
				VerifyOnExit();
				break;
			case ID_EDIT_REDO:
				Redo();
				break;
			case ID_EDIT_UNDO:
				Undo();
				break;
			case ID_VIEW_CLONEFRAME:
				RefreshCloneTree();
				break;
			case ID_VIEW_MATERIALFRAME:
				RefreshMaterialList();
				break;
			case ID_VIEW_PRIMITIVEFRAME:
				RefreshPrimitiveList();
				//CreateToolWindow();
				break;
			}
		InvalidateRect(hMainWnd, NULL, FALSE);
		break;
		}
		case WM_SIZE:
		{
			if(wParam != SIZE_MINIMIZED){
				CreateToolWindow();
				RefreshCloneTree();
				RefreshMaterialList();
				RefreshPrimitiveList();
			}
			break;
		}
		case WM_KEYDOWN:
		{
			if((UCHAR)wParam == VK_ESCAPE)
			{
				VerifyOnExit();
				return FALSE;
			}
		break;
		}
		case WM_DROPFILES:
		{
			HDROP hDrop;
			char szFilename[1024] = {0};
			hDrop = (HDROP)wParam;
			DragQueryFile(hDrop, 0, szFilename, 1024);
			DragFinish(hDrop);

			if(MessageBox(hMainWnd, "Open a dropped file?", szFilename, MB_YESNO|MB_ICONQUESTION|MB_SYSTEMMODAL)==IDYES){
				//PrepareForFileChange();
				mdl.Free();
				if(mdl.LoadFromFile(szFilename)<0)
				{
					MessageBox(NULL, "ファイルを開くことができません.", "読み取りエラー", 0);
					return FALSE;
				}
				lstrcpy(g_szModelName, szFilename);
				RefreshAllView();
			}
			break;
		}
		case WM_CLOSE:
		case WM_QUIT:
		case WM_DESTROY:
			VerifyOnExit();
			return FALSE;
	}
	return TRUE;
}