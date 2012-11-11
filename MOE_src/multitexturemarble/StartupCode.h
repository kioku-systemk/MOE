#include "common.h"

void OnDraw(HDC hDC);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void Initialize();
int cmain();

void RefreshAllView();
void VerifyOnExit();
void VerifyBeforeOpenFile();
void OnDraw(HDC hDC);
void ImportKMD(const char* szImportFile);
void PrepareForFileChange(bool isDeleteTextures = false);