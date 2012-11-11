#include "common.h"

/* 外部からのアクセス用 */
void AddSceneObjectList(HWND hListView, int nOrder, const char* str);
void ClearSceneObjectList(HWND hListView);
void RefreshSceneObjectList();
long GetSelectedSceneObject();
void FreeSceneObjectList();


LRESULT CALLBACK SceneObjectListHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* 内部からのアクセス用 */
int SceneObjectListNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit);
int SceneObjectListKey(UCHAR key, bool isDown);
int SceneObjectListMouse(long x, long y, UINT msg);
void SceneObjectListPopup(HWND hWnd, WPARAM wParam, LPARAM lParam);
void SceneObjectListCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
void SceneObjectListSelectedItemNotify(int, bool);
//void SceneObjectListPopup(HWND hWnd, WPARAM wParam, LPARAM lParam):