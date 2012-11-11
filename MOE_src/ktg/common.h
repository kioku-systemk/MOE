#include "../CLibrary/CWindowGL.h"
#define __USE_CTRACE__
#include "../CLibrary/CTrace.h"
#include "../kLib/KTexture.h"
#include "../kLib/KTextureEdit.h"
#include "resource.h"
#include <commctrl.h>
#include <richedit.h>

#define hMainWnd win.CGethWnd()
#define SAFE_FREE(ptr) { if(ptr!=NULL){ GlobalFree(ptr); } ptr=NULL; }
#define TEXT_BUFFER_SIZE (1024*64)