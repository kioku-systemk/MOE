#include "common.h"

int MaterialNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit);
void MaterialPopup(HWND	hWnd, WPARAM wParam, LPARAM	lParam);
void MaterialCommand(HWND hWnd,	WPARAM wParam, LPARAM lParam);
int MaterialKey(UCHAR key, bool isDown);
int MaterialMouse(long x, long y, UINT btn);
LRESULT CALLBACK MaterialDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int  MaterialCreateTexture(int item ,const char* szTexture);
int  MaterialGetSelectedMaterial();//KMaterial—pid‚É•ÏŠ·‚·‚é

void AddMaterialList(HWND hListView, int nOrder, char* str);
void ClearMaterialList(HWND hListView);
void RefreshMaterialList();

void InitMaterialWindow();