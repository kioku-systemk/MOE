#include "../CLibrary/CWindowGL.h"
#include "../CLibrary/CTreeView.h"
#include "../CLibrary/CListView.h"
#include "../CLibrary/CTrace.h"
#include "../klib/kModelEdit.h"
#include "../CLibrary/csound.h"

#include "resource.h"
#include "resrc1.h"
#include <shlwapi.h>

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "Comdlg32.lib")
#pragma warning(disable:4244)//å^ïœä∑ÇÃåxçêîrèú


#define hMainWnd win.CGethWnd()
#define hGLWnd	 wingl.CGethWnd()
#define MBQ(str) (MessageBox(NULL, str, "ÅH", MB_YESNO|MB_ICONQUESTION)==IDYES)
#define APP_VER 0.9
//#define SZ_APP_TTL "Layouter, ver.%4.2f, Build : %s, coded by kioku and c.r.v." 
#define SZ_DEFAULT_FILENAME "untitled layout"

#define LAYOUTER 0
#define TIMELINER 1