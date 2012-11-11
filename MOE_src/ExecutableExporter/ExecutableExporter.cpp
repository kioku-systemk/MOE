#include "stdafx.h"

#include "../kLib/kResourceDemoDat.h"
#include "../IntegratedDemoSystem/resource.h" //IDR_SYSTEMK_DEMODAT
#include "../CLibrary/Util.h"


#ifdef NOT64K
#include "DemoSkelton_n64.h"
#else
#include "DemoSkelton.h"
#endif

#include "resource.h"

using namespace std;
using namespace File::Win32;

const unsigned char KDB_HEADER[] = {'K','D','B'};

unsigned char *kdb[KDB_NUM];
DWORD dwKDBSize[KDB_NUM];
int nYear = 2007;
string strDemoTitle, strExeName;

BOOL ExportExe(){
	//export exe
	HANDLE hFile = CreateFile(strExeName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if( hFile==INVALID_HANDLE_VALUE ){
		String::Output::Win32::MessageBox("Unable to open %s", strExeName.c_str());
		return FALSE;
	}
	DWORD dwWritten;
	WriteFile(hFile, DemoSkelton, sizeof(DemoSkelton), &dwWritten, NULL);
	if( dwWritten!=sizeof(DemoSkelton) ){
		String::Output::Win32::MessageBox("can't write to media!");
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);

	//ow resource
	HANDLE hRes = BeginUpdateResource(strExeName.c_str(), FALSE);
	if( hRes==NULL ){
		String::Output::Win32::MessageBox("can't update resource!");
		return FALSE;
	}

	KResourceDemoDat resDemo;
	resDemo.nYear = nYear;
	resDemo.dwLength           = strDemoTitle.length()+sizeof('\0');
	resDemo.szRc_DemoTitle     = const_cast<char*>(strDemoTitle.c_str());
	resDemo.dwSize[KDB_LOADER] = dwKDBSize[KDB_LOADER];
	resDemo.dwSize[KDB_MAIN]   = dwKDBSize[KDB_MAIN];
	resDemo.kdb[KDB_LOADER]    = kdb[KDB_LOADER];
	resDemo.kdb[KDB_MAIN]      = kdb[KDB_MAIN];

	unsigned char* buf = (unsigned char*)GlobalAlloc(GPTR, GetKResourceDemoDatSize(&resDemo));
	unsigned int bufptr = 0;
	CopyMemory(buf+bufptr, &resDemo.nYear, sizeof(int));		bufptr += sizeof(int);
	CopyMemory(buf+bufptr, &resDemo.dwLength, sizeof(DWORD));	bufptr += sizeof(DWORD);
	CopyMemory(buf+bufptr, resDemo.szRc_DemoTitle, resDemo.dwLength);	bufptr += (resDemo.dwLength);
	CopyMemory(buf+bufptr, &resDemo.dwSize[KDB_LOADER], sizeof(DWORD));	bufptr += sizeof(DWORD);
	CopyMemory(buf+bufptr, &resDemo.dwSize[KDB_MAIN],   sizeof(DWORD));	bufptr += sizeof(DWORD);
	CopyMemory(buf+bufptr, resDemo.kdb[KDB_LOADER],     resDemo.dwSize[KDB_LOADER]);	bufptr += resDemo.dwSize[KDB_LOADER];
	CopyMemory(buf+bufptr, resDemo.kdb[KDB_MAIN],       resDemo.dwSize[KDB_MAIN]);		bufptr += resDemo.dwSize[KDB_MAIN];

	UpdateResource(hRes, "SYSTEM_K_DEMODAT", MAKEINTRESOURCE(IDR_SYSTEM_K_DEMODAT), MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT), buf, bufptr);
	EndUpdateResource(hRes, FALSE);

	SAFE_GLOBALFREE(buf);

	return TRUE;
}

LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	BEGIN_MESSAGE_DISPATCHING(msg)

		BEGIN_MESSAGE_HANDLING(WM_INITDIALOG)
			SetDlgItemInt(hWnd, IDC_YEAR, nYear, TRUE);
			CheckDlgButton(hWnd, IDC_LOAD_CHECK, FALSE);
			CheckDlgButton(hWnd, IDC_MAIN_CHECK, FALSE);
		END_MESSAGE_HANDLING

		BEGIN_MESSAGE_HANDLING(WM_COMMAND)
			BEGIN_DIALOG_CONTROL_DISPATCHING(wParam)

				BEGIN_DIALOG_CONTROL_HANDLING(IDOK)
					//check if kdb is loaded
					if( !IsDlgButtonChecked(hWnd, IDC_LOAD_CHECK) || !IsDlgButtonChecked(hWnd, IDC_MAIN_CHECK) ){
						String::Output::Win32::MessageBox("Loader or Main is not loaded!");
						break;
					}

					Dialog::DialogFileTypeParam* pDp = NULL;
					Dialog::MakeDialogFileTypeParam(&pDp, "Executable format", "*.exe");
					Dialog::MakeDialogFileTypeParam(&pDp, "All file", "*.*");
					if( Dialog::Save(hWnd, pDp, &strExeName) ){
						nYear = GetDlgItemInt(hWnd, IDC_YEAR, NULL, TRUE);

						HWND hDemoTitle = GetDlgItem(hWnd, IDC_DEMOTITLE);
						int nTextLength = GetWindowTextLength(hDemoTitle) + sizeof('\0');
						char* szDemoTitle = reinterpret_cast<char*>( GlobalAlloc(GPTR, sizeof(char)*nTextLength) ); 
							GetWindowText(hDemoTitle, szDemoTitle, nTextLength);
							strDemoTitle = szDemoTitle;
						GlobalFree(szDemoTitle);

						if( !ExportExe() ){
							
						}else{
							if( MessageBox(	hWnd, "Exporting process went OK. Exit now?", "Say something!", MB_YESNO|MB_TOPMOST|MB_ICONQUESTION)==IDYES ){
								EndDialog(hWnd, TRUE);
							}
						}

					}
					Dialog::FreeDialogFileTypeParam(pDp);
				END_DIALOG_CONTROL_HANDLING

				BEGIN_DIALOG_CONTROL_HANDLING(IDCANCEL)
					EndDialog(hWnd, FALSE);  
				END_DIALOG_CONTROL_HANDLING

				BEGIN_DIALOG_CONTROL_HANDLE(IDC_LOAD_MAIN)
				BEGIN_DIALOG_CONTROL_HANDLE(IDC_LOAD_LOADER)
				{
					const int offset = LOWORD(wParam) - IDC_LOAD_LOADER;
					string strKDBName;

					Dialog::DialogFileTypeParam* pDp = NULL;
					Dialog::MakeDialogFileTypeParam(&pDp, "KDB format", "*.kdb");
					Dialog::MakeDialogFileTypeParam(&pDp, "All file", "*.*");
					if( Dialog::Open(hWnd, pDp, &strKDBName) ){
						HANDLE hFile = CreateFile(strKDBName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if( hFile==INVALID_HANDLE_VALUE ){
							String::Output::Win32::MessageBox("can't open %s", strKDBName.c_str());
							break;
						}

						//clean & alloc
						dwKDBSize[offset] = GetFileSize(hFile, NULL);
						SAFE_GLOBALFREE(kdb[offset]);
						kdb[offset] = reinterpret_cast<unsigned char*>( GlobalAlloc(GPTR, dwKDBSize[offset]) );

						//read
						DWORD dwRead;
						ReadFile(hFile, kdb[offset], dwKDBSize[offset], &dwRead, NULL);
						if( dwRead!=dwKDBSize[offset] ){
							String::Output::Win32::MessageBox("can't read from media!");
							CloseHandle(hFile);
							break;
						}

						if( memcmp(kdb[offset], KDB_HEADER, sizeof(KDB_HEADER))!=0 ){
							String::Output::Win32::MessageBox("this is not a KDB file: %s", strKDBName.c_str());
							CloseHandle(hFile);
							break;
						}

						//went ok.
						CloseHandle(hFile);
						CheckDlgButton(hWnd, IDC_LOAD_CHECK+offset, TRUE);
					}
					Dialog::FreeDialogFileTypeParam(pDp);
				}
				END_DIALOG_CONTROL_HANDLE
				END_DIALOG_CONTROL_DISPATCHING

				default:
					return FALSE;
		END_MESSAGE_HANDLING

	END_MESSAGE_DISPATCHING
	return TRUE;
}

BOOL InitApp(){
	//verify that this program is running on WindowsNT.
	OSVERSIONINFO osVer;
	osVer.dwOSVersionInfoSize = sizeof(osVer);
	GetVersionEx(&osVer);
	if( osVer.dwPlatformId!=VER_PLATFORM_WIN32_NT ){
		String::Output::Win32::MessageBox("You have to run this program with WindowsNT 3.51 or later version of WindowsNT OS.");
		return FALSE;
	}
	return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR szCmd, int nCmdShow){
	if( !InitApp() ){
		return FALSE;
	}

	if( DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_EE_MAIN), NULL, (DLGPROC)DlgProc) ){
		
	}

	//clean up
	for(int i=0; i<KDB_NUM; ++i){
		SAFE_GLOBALFREE(kdb[i]);
	}
	return TRUE;
}