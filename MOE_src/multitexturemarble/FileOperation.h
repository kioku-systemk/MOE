#include "common.h"

int OpenModel(KModelEdit* mdl, bool isOpenDialog = true);
int SaveModel(KModelEdit* mdl, bool isOpenDialog = true);
int GetOpenFileNameSingle(HWND hOwnerWnd, const char* szExtention, char* szRecv, bool isAllowAllType = true);
int GetSaveFileNameSingle(HWND hOwnerWnd, const char* szExtention, char* szRecv, bool isAllowAllType = true);