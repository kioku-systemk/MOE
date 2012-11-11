#include "common.h"

//void OpenLayout(KModelEdit* mdl, bool isOpenDialog = true){};
//void SaveLayout(KModelEdit* mdl, bool isOpenDialog = true){};
int GetOpenFileNameSingle(HWND hOwnerWnd, const char* szExtention, char* szRecv, bool isAllowAllType = true);
int GetSaveFileNameSingle(HWND hOwnerWnd, const char* szExtention, char* szRecv, bool isAllowAllType = true);