#include "common.h"

/* �O������̃A�N�Z�X�p */
void AddSceneObjectList(HWND hListView, int nOrder, const char* str);
void ClearSceneObjectList(HWND hListView);
void RefreshSceneObjectList();
long GetSelectedSceneObject();
void FreeSceneObjectList();


LRESULT CALLBACK SceneObjectListHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* ��������̃A�N�Z�X�p */
int SceneObjectListNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit);
int SceneObjectListKey(UCHAR key, bool isDown);
int SceneObjectListMouse(long x, long y, UINT msg);
void SceneObjectListPopup(HWND hWnd, WPARAM wParam, LPARAM lParam);
void SceneObjectListCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
void SceneObjectListSelectedItemNotify(int, bool);
//void SceneObjectListPopup(HWND hWnd, WPARAM wParam, LPARAM lParam):