#include "common.h"

int CloneGetSelectedItem();
void CloneSelectClone(KClone* pDest);
void CloneSelectedItemNotify(int nDest, bool isByMouse, int nOrg, bool isDrag);
int CloneGetKCloneId(KClone* pSrc);
int GetCTreeViewId(KClone* kcl);
KClone* GetKClone(int nTreeViewId);
KClone* CloneGetKCloneFromTreeViewId(int nTreeViewId);
KClone* GetMaster(KClone* pSlave);
KClone* GetSlave(KClone* pMaster);
KClone* GetKCloneFromObjName(const char* szCloneName, bool isSearchClone);

void CloneDelete(KClone* pTarget, KClone* pAdd);
KClone* CloneAddToEmptyPoint(KClone* pDest);
KClone* CloneReConnect(int nDest);
KClone* CloneInsertItem(int nDest, int nAdditional = -1);

void AddCloneTree(HWND hTreeView, int nOrderInKClone, char* str);
void ClearCloneTree(HWND hTreeView);
void RefreshCloneTree();

int CloneNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit);
void ClonePopup(HWND	hWnd, WPARAM wParam, LPARAM	lParam);
void CloneCommand(HWND hWnd,	WPARAM wParam, LPARAM lParam);
int CloneKey(UCHAR key, bool isDown);
int CloneMouse(long x, long y, UINT btn);
LRESULT CALLBACK CloneDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CloneDelete(KClone* pTarget);