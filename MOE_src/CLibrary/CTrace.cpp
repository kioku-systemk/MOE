#include "stdafx.h"
#include "./CTrace.h"

#ifdef _CRVHOGEHOGE
static FILE* trace_fp;

void OpenTrace(const char* str)
{
	trace_fp=fopen(str, "wt");
}

//void OutTrace(const char* format, ...)
//{
//	fprintf(trace_fp, str);
//	fprintf(trace_fp, "\n");
//}

void OutTrace(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
#if _MSC_VER >= 1400
	TCHAR* str = (TCHAR*)malloc(sizeof(TCHAR) * (_vscprintf(format, ap)+1));
#else
	TCHAR str[1024];
#endif
	vsprintf(str,format,ap);
	va_end(ap);
	fprintf(trace_fp, str);
	fprintf(trace_fp, "\n");
#if _MSC_VER >= 1400
	free(str);
#endif
}

void OutTraceLine()
{
	fprintf(trace_fp, "------------------------------\n");
}

void CloseTrace()
{
	if(trace_fp) fclose(trace_fp);
}
#else
void OpenTrace(const char* str){};
void OutTrace(const char* format, ...){};
void OutTraceLine(){};
void CloseTrace(){};
#endif