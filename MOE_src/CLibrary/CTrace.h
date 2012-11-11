#include <windows.h>
#include <stdio.h>

void OpenTrace(const char* str);
//void OutTrace(const char* str);
//void OutTraceF(const char* format, ...);
void OutTrace(const char* format, ...);
void OutTraceLine();
void CloseTrace();