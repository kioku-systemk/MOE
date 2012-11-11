#include "common.h"

/* �O������̃A�N�Z�X�p */
void AddSceneList(HWND hListView, int nOrder, char* str);
void ClearSceneList(HWND hListView);
void RefreshSceneList();
long GetSelectedScene();
void FreeSceneList();

LRESULT CALLBACK SceneListHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


/* ��������̃A�N�Z�X�p */
int SceneListNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit);
int SceneListKey(UCHAR key, bool isDown);
int SceneListMouse(long x, long y, UINT msg);
void SceneListPopup(HWND hWnd, WPARAM wParam, LPARAM lParam);
void SceneListCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
void SceneListSelectedItemNotify(int, bool);

