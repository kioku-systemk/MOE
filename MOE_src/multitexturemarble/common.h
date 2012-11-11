#include "../CLibrary/CWindowGL.h"
#include "../klib/kModelEdit.h"//こいつをインクルード
#include "resource.h"
#include "resrc1.h"
#include "../CLibrary/CTreeView.h"
#include "../CLibrary/CListView.h"
#include "../CLibrary/CTrace.h"
#include "../klib/ktextureedit.h"
#include <shlwapi.h>

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "Comdlg32.lib")

#define hMainWnd win.CGethWnd()
#define MBQ(str) MessageBox(NULL, str, "？", MB_YESNO|MB_ICONQUESTION|MB_SYSTEMMODAL)
#define APP_VER 0.951
#define SZ_DEFAULT_FILENAME "untitled model"