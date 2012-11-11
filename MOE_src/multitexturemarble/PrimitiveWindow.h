#include "common.h"

int PrimitiveNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit);
void PrimitivePopup(HWND	hWnd, WPARAM wParam, LPARAM	lParam);
void PrimitiveCommand(HWND hWnd,	WPARAM wParam, LPARAM lParam);
int PrimitiveKey(UCHAR key, bool isDown);
int PrimitiveMouse(long x, long y, UINT btn);
int PrimitiveGetSelectedPrimitive();

void AddPrimitiveList(HWND hListView, int nOrder, char* str);
void ClearPrimitiveList(HWND hListView);
void RefreshPrimitiveList();
