#include "common.h"

/* 外部からのアクセス用 */
void AddObjectList(HWND hListView, int nOrder, char* str);
void ClearObjectList(HWND hListView);
void RefreshObjectList();
int GetSelectedObjectNum();
void FreeObjectList();


LRESULT CALLBACK ObjectListHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* 内部からのアクセス用 */
int ObjectListNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit);
int ObjectListKey(UCHAR key, bool isDown);
int ObjectListMouse(long x, long y, UINT msg);
void ObjectListPopup(HWND hWnd, WPARAM wParam, LPARAM lParam);
void ObjectListCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
void ObjectListSelectedItemNotify(int, bool);