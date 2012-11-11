#include "common.h"

/* 外部からの呼び出し用 */
//KCloneのインデックスに該当する項目を選択状態にする
void CloneSelectClone(int);
//KModelEdit*を元に,ツリービューを再構築する
void RefreshCloneTree(KModelEdit*, const char* szFileName, int select);
//現在選択されている項目に該当するKCloneのインデックスを返す
int CloneGetSelectedItem();
KClone* GetSelectedClone();
void FreeCloneWindow();

LRESULT CALLBACK CloneHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* 内部で使用する関数 */
void ClearCloneTree(HWND hTreeView);
void CloneSelectedItemNotify(int nDest, bool isByMouse, int nOrg, bool isDrag);
int CloneNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit);
int CloneKey(UCHAR key, bool isDown);
int CloneMouse(long x, long y, UINT btn);