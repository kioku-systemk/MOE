#include "stdafx.h"
#include "LayoutFileManager.h"
#include "StartupCode.h"

char g_szLayoutName[1024];

//void OpenLayout(,bool isOpenDialog)
//{
//	if(mdl == NULL) return;
//
//	char szModelNameToOpen[1024];
//	if(isOpenDialog){
//		if(GetOpenFileNameSingle(NULL, "kmd", szModelNameToOpen)==FALSE) return;
//		if(szModelNameToOpen[0] == '\0') return;
//	}else{
//		if(g_szModelName[0] == '\0') return;
//		lstrcpy(szModelNameToOpen, g_szModelName);
//	}
//
//	mdl->Free();
//	if(mdl->LoadFromFile(szModelNameToOpen)<0)
//	{
//		MessageBox(NULL, "ファイルを開くことができません.", "読み取りエラー", 0);
//	}
//
//	lstrcpy(g_szModelName, szModelNameToOpen);
//	RefreshAllView();
//}
//
//void SaveLayout(, bool isOpenDialog)
//{
//	if(mdl == NULL) return;
//	
//	char szModelNameToOpen[1024];
//	if(isOpenDialog || (!isOpenDialog && 0==lstrcmp(SZ_DEFAULT_FILENAME, g_szModelName))){
//		if(GetSaveFileNameSingle(NULL, "kmd", szModelNameToOpen)==FALSE) return;
//		if(szModelNameToOpen[0] == '\0') return;
//		char szExt[64];
//		_splitpath(g_szModelName, NULL, NULL, NULL, szExt);
//		if(szExt[0] == '\0') lstrcat(szModelNameToOpen, ".kmd");
//	}else{
//		if(g_szModelName[0] == '\0') return;
//		lstrcpy(szModelNameToOpen, g_szModelName);
//	}
//
//	FILE* fp;
//	if(NULL == (fp = fopen(szModelNameToOpen,"wb"))) return;
//		mdl->SaveKMD(fp);
//	fclose(fp);
//
//	lstrcpy(g_szModelName, szModelNameToOpen);
//	RefreshAllView();
//}

int GetOpenFileNameSingle(HWND hOwnerWnd, const char* szExtention, char* szRecv, bool isAllowAllType)
{
	lstrcpy(szRecv, "");

	TCHAR szFilter[512];
	TCHAR szToken[2][64];
	
	wsprintf(szToken[0], "Supported files(*.%s)", szExtention);
	int i;
	for(i=0; i<lstrlen(szToken[0])+1; i++)
		szFilter[i] = szToken[0][i];
	//szFilter[i] = '\0';

	wsprintf(szToken[1], "*.%s", szExtention);
	int previous = lstrlen(szToken[0])+1;
	//int previous = lstrlen(szToken[0])+1+1;
	int dest = lstrlen(szToken[1])+1;
	int j;
	for(j=0; j<dest; j++)
		szFilter[j+previous] = szToken[1][j];

	if(isAllowAllType == false)	szFilter[j+previous] = '\0';
	else
	{
		//"All Files(*.*)"
		int k;
		previous += lstrlen(szToken[1])+1;
		//previous = lstrlen(szFilter)+1+1;
		char szSrc[64];
		lstrcpy(szSrc, "All Files(*.*)");
		for(k=0; k<lstrlen(szSrc)+1; k++)
			szFilter[k+previous] = szSrc[k];

		previous += lstrlen(szSrc) + 1;
		lstrcpy(szSrc, "*.*");
		for(k=0; k<lstrlen(szSrc)+1; k++)
			szFilter[k+previous] = szSrc[k];
		szFilter[k+previous] = '\0';
	}
	
	TCHAR szTitle[512];
	wsprintf(szTitle, "Opening .%s file", szExtention);

	TCHAR szCurrentDirectory[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurrentDirectory);

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hOwnerWnd;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = szRecv;
	ofn.nMaxFile = 1024;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = szExtention;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrFileTitle = NULL; //ファイル名
	ofn.lpstrTitle = szTitle;
//	ofn.lpstrInitialDir = szCurrentDirectory;
	if(GetOpenFileName(&ofn)) return TRUE;
	return FALSE;
}

int GetSaveFileNameSingle(HWND hOwnerWnd, const char* szExtention, char* szRecv, bool isAllowAllType)
{
	lstrcpy(szRecv, "");
	TCHAR szFilter[512];
	TCHAR szToken[2][64];
	
	wsprintf(szToken[0], "Supported files(*.%s)", szExtention);
	int i;
	for(i=0; i<lstrlen(szToken[0])+1; i++)
		szFilter[i] = szToken[0][i];
	//szFilter[i] = '\0';

	wsprintf(szToken[1], "*.%s", szExtention);
	int previous = lstrlen(szToken[0])+1;
	//int previous = lstrlen(szToken[0])+1+1;
	int dest = lstrlen(szToken[1])+1;
	int j;
	for(j=0; j<dest; j++)
		szFilter[j+previous] = szToken[1][j];

	if(isAllowAllType == false)	szFilter[j+previous] = '\0';
	else
	{
		//"All Files(*.*)"
		int k;
		previous += lstrlen(szToken[1])+1;
		//previous = lstrlen(szFilter)+1+1;
		char szSrc[64];
		lstrcpy(szSrc, "All Files(*.*)");
		for(k=0; k<lstrlen(szSrc)+1; k++)
			szFilter[k+previous] = szSrc[k];

		previous += lstrlen(szSrc) + 1;
		lstrcpy(szSrc, "*.*");
		for(k=0; k<lstrlen(szSrc)+1; k++)
			szFilter[k+previous] = szSrc[k];
		szFilter[k+previous] = '\0';
	}

	TCHAR szTitle[512];
	wsprintf(szTitle, "Opening .%s file", szExtention);

	TCHAR szCurrentDirectory[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurrentDirectory);

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hOwnerWnd;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szRecv;
	ofn.nMaxFile = 1024;
	ofn.lpstrDefExt = szExtention;
	ofn.Flags = OFN_OVERWRITEPROMPT;
//	ofn.lpstrInitialDir = szCurrentDirectory;
	if(GetSaveFileName((LPOPENFILENAME)&ofn)) return TRUE;
	return FALSE;
}