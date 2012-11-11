#ifndef _UTIL_H_FE2915E7_4CEA_40EC_B75D_C221A34BB901_
#define _UTIL_H_FE2915E7_4CEA_40EC_B75D_C221A34BB901_

#include <vector>
#include <string>
#include <assert.h>

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <commctrl.h>

//#ifdef USE_RENDERER_GL
#include <gl/gl.h>
#include <gl/glu.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
//#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(__ptr__) do{ if(__ptr__) delete __ptr__; __ptr__=NULL; }while(0)
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(__aptr__) do{ if(__aptr__) delete[] __aptr__; __aptr__=NULL; }while(0)
#endif
#ifndef COUNTOF_ARRAY
#define COUNTOF_ARRAY(__array__) sizeof(__array__)/sizeof(__array__##[0])
#endif 

#ifndef SAFE_GLOBALFREE
#define SAFE_GLOBALFREE(__ptr__) do{ if(((__ptr__) != NULL)) GlobalFree(__ptr__); __ptr__=NULL; }while(0)
#endif

#ifndef SAFE_COPYMEMORY
#define SAFE_COPYMEMORY(__destptr__,__srcptr__,__sizetocopy__) if((__destptr__ != NULL) && (__srcptr__ != NULL) && ((__sizetocopy__) > 0)) CopyMemory(__destptr__,__srcptr__,__sizetocopy__)
#endif

#define GET_ARGUMENTS()\
	int argc;\
	int i;\
	WCHAR** wszCmdLine = CommandLineToArgvW(GetCommandLineW(), &argc);\
	int length;\
	char** argv = (char**)GlobalAlloc(GPTR, sizeof(char*) * argc);\
	for(i=0; i<argc; i++){\
		if((length = ::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, NULL, 0, NULL, NULL))==0) continue;\
		argv[i] = (char*)GlobalAlloc(GPTR, sizeof(char) * (length+1));\
		::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, argv[i], length, NULL, NULL);\
	}
#define RELEASE_ARGUMENTS()\
	for(i=0; i<argc; i++){\
		GlobalFree(argv[i]);\
	}\
	GlobalFree(argv);

#ifdef UNICODE
#ifndef SetWindowLongPtr
#define SetWindowLongPtr  SetWindowLongPtrW
#endif
#else
#ifndef SetWindowLongPtr
#define SetWindowLongPtr  SetWindowLongPtrA
#endif
#endif // !UNICODE

#define DECLARE_VECTOR_ITERATOR(type__)\
	std::vector<##type__##>::iterator

#define BEGIN_VECTOR_LOOP(type__,name__)\
	{\
	std::vector<##type__##>::iterator it,eit=##name__##.end();\
	for(it=##name__##.begin(); it!=eit; ++it){

#define END_VECTOR_LOOP\
	}\
	}

#define BEGIN_MESSAGE_DISPATCHING(message__)\
	switch(##message__##)\
	{
#define END_MESSAGE_DISPATCHING\
	}

#define BEGIN_MESSAGE_HANDLE(message__)\
	case message__##:
#define END_MESSAGE_HANDLE\
	break;

#define BEGIN_MESSAGE_HANDLING(message__)\
	case message__##:\
	{
#define END_MESSAGE_HANDLING\
	}break;

#define BEGIN_DIALOG_CONTROL_DISPATCHING(wParam__)\
	switch(LOWORD(##wParam__##))\
	{
#define END_DIALOG_CONTROL_DISPATCHING\
	}break;

#define BEGIN_DIALOG_CONTROL_HANDLING(control__)\
	case control__##:\
	{
#define END_DIALOG_CONTROL_HANDLING\
	}break;
#define BEGIN_DIALOG_CONTROL_HANDLE(control__)\
	case control__##:
#define END_DIALOG_CONTROL_HANDLE\
	break;

namespace Renderer{
	//typedef struct _tagRendererColor{
	//	float r; float g; float b;
	//	float a;
	//}RendererColor;

	//typedef struct _tagRendererVector3{
	//	float x; float y; float z;
	//}RendererVector3;

	class RendererColor{
		public:
			float r; float g; float b;
			float a;
			RendererColor(float nr, float ng, float nb, float na){ r=nr; g=ng; b=nb; a=na; }
	};

	class RendererVector2{
		public:
			float x; float y;
			RendererVector2(){};
			RendererVector2(float nx, float ny){ x=nx; y=ny; }
	};

	class RendererVector3 : public RendererVector2{
		public:
			float z;
			RendererVector3(){};
			RendererVector3(float nx, float ny, float nz){ x=nx; y=ny; z=nz; }
	};

	class RendererVector4 : public RendererVector3{
		public:
			float w;
			RendererVector4(){};
			RendererVector4(float nx, float ny, float nz, float nw){ x=nx; y=ny; z=nz; w=nw; }
	};

	namespace GL{
		//#ifdef USE_RENDERER_GL
		//#endif

		enum eRenderingMode{ VIEW_ORTHO=0, VIEW_PERSPECTIVE };
		enum eMatrixMode{ PROJECTION=GL_PROJECTION, MODELVIEW=GL_MODELVIEW, TEXTURE=GL_TEXTURE };

		bool InitOpenGL(HWND hWnd, HGLRC* phRC, int width, int height);
		void ResizeWindow(int iWidth, int iHeight);
		void ViewOrtho(float fWorldWidth, float fWorldHeight, float fNear=0.00001f, float fFar=1000.0f);
		void ViewPerspective(float fFovy, float fWorldWidth, float fWorldHeight, float fNear=1.0f, float fFar=1000.0f);
		void PushViewMatrix();
		void PopViewMatrix();
		bool UnInitOpenGL(HGLRC& hRC);
		void ActivateScreen(HWND hWnd, HGLRC& hRC);
		void ClearScreen(float r, float g, float b, float a);
		void RedrawScreen(HWND hWnd/*, HGLRC& hRC*/);
		void FillScreen(float r, float g, float b, float a);
		void FillScreen(RendererColor& color);

		void DrawRect(float width, float height);
		void DrawRect(float x, float y, float width, float height);

		void DrawCenteredRect(float width, float height);
		void DrawCenteredRect(float xoffset, float yoffset, float width, float height);
		void DrawCenteredRect(float xoffset, float yoffset, float width, float height, float screenwidth, float screeneheight);

		void LineWidth(float fWidth);
		void DrawLine(float frmx, float frmy, float tox, float toy);

		void DrawColor(float r, float g, float b, float a);
		void DrawColor(RendererColor& color);
		
		RendererVector2 GetCenteredPos(float width, float height);
		RendererVector2 GetCenteredPos(float width, float height, float screenwidth, float screeneheight);

		void Translate(float x, float y, float z);
		void Rotate(float x, float y, float z);
		void Scale(float x, float y, float z);
		void LoadIdentity();
		void PushMatrix();
		void PopMatrix();
		void PushViewMatrix();
		void PopViewMatrix();
		void PostRedisplay();
		void PushStates();
		void PopStates();

		void MatrixMode(Renderer::GL::eMatrixMode matrix_mode);
		void DepthTestState(bool new_state);
		void LineSmoothState(bool new_state);
		void LightingState(bool new_state);
		void LightState(unsigned int light, bool new_state);

	}
}

namespace String{

	namespace Output{
		namespace GL{
			void __DeleteFontList();
			void __CreateFontList();
			GLuint __QueryFontChache(unsigned short w);
			void DrawString(const char* format, ...);
		}
		namespace Win32{
			void OutputDebugStringFormatted(const char* format, ...);
			void MessageBox(const char* format, ...);
			void SetWindowText(HWND hWnd, const char* format, ...);
		}
	}

	void FormatString(char* szRecv, unsigned long nSize, const char* format, ...);

}

namespace File{
	namespace Win32{
		namespace Dialog{
			//typedef struct _tagDialogFileTypeParam{
			//	char* szDescription; //Executable File
			//	char* szExt;		 //exe
			//	_tagDialogFileTypeParam* next;
			//}DialogFileTypeParam;
			class DialogFileTypeParam{
				public:
					char* szDescription; //Executable File
					char* szExt;		 //exe
					DialogFileTypeParam* next;
					DialogFileTypeParam():szDescription(NULL),szExt(NULL),next(NULL){}
			};
	
			const int MAX_SELECTION_OF_FILE = 1024;
			const int SIZE_FILENAME_BUFFER = MAX_PATH * MAX_SELECTION_OF_FILE;

			void __CreateDialogFileTypeParam(DialogFileTypeParam** ppDp, const char* szDescription, const char* szExt);
			const char* __MakeFilterString(const DialogFileTypeParam& dp);
			void __SplitMultipleFilePath(const char* szMultipleFileBuffer, char** szPathName/*[MAX_PATH]*/, int* nNum);
			const OPENFILENAME& __GetOpenOFN(HWND hWnd, const char* szFilter, char* szFileNameBuffer, const int nSizeBuffer, BOOL isAllowMultipleSelection);
			const OPENFILENAME& __GetSaveOFN(HWND hWnd, const char* szFilter, char* szFileNameBuffer, const int nSizeBuffer, BOOL isAllowMultipleSelection);

			//std::string& __MakeFilterString(const DialogFileTypeParam& dp);
			bool MakeDialogFileTypeParam(DialogFileTypeParam** ppDpHead, const char* szDescription, const char* szExt);//szExt = "*.map"(single) or "*.map;*.ply;*itm"(multiple)
			bool FreeDialogFileTypeParam(const DialogFileTypeParam* pDpHead);

			bool Open(HWND hWnd, const DialogFileTypeParam* dp, char** szPathName/*[MAX_PATH]*/, int* nSelection = NULL);
			bool Save(HWND hWnd, const DialogFileTypeParam* dp, char** szPathName/*[MAX_PATH]*/, int* nSelection = NULL);
			bool Open(HWND hWnd, const DialogFileTypeParam* dp, std::string* strPathName, int* nSelection = NULL);
			bool Save(HWND hWnd, const DialogFileTypeParam* dp, std::string* strPathName, int* nSelection = NULL);

		}
	}
	bool IsEof(FILE* fp);
}

namespace System{
	namespace Win32{
		void SetForegroundWindowEx(HWND hTargetWnd, DWORD dwForegroundThreadID = 0);
	}
}

namespace Control{
	namespace Win32{
		namespace ListBox{
			typedef int HLISTITEM;
			const HLISTITEM HLISTITEM_ROOT = ((int)0);
			//HLISTITEM GetHandle(int 

			void Set_CurrentSelection(HWND hWnd, HLISTITEM hNewSelection);
			HLISTITEM Get_CurrentSelection(HWND hWnd);

			HLISTITEM Add(HWND hWnd, const char* szString);
			HLISTITEM Insert(HWND hWnd, HLISTITEM hWhere, const char* szString);
			void Delete(HWND hWnd, HLISTITEM hDelete);
			void DeleteAll(HWND hWnd);

			bool IsValidValue(HLISTITEM hCheckFor);

			int Get_Index(HLISTITEM hConvert);
			int Get_PrevIndex(HLISTITEM hNext);
			int Get_NextIndex(HLISTITEM hPrevious);
		}
		namespace TreeView{
			const HTREEITEM HTREEITEM_ROOT = TVI_ROOT;
			const HTREEITEM HTREEITEM_LAST = TVI_LAST;

			HTREEITEM Get_Root(HWND hWnd);
			HTREEITEM Get_Parent(HWND hWnd, HTREEITEM hChild);
			HTREEITEM Get_Child(HWND hWnd, HTREEITEM hParent);
			HTREEITEM Get_PrevSibling(HWND hWnd, HTREEITEM hSomeWhere);
			HTREEITEM Get_NextSibling(HWND hWnd, HTREEITEM hSomeWhere);
			HTREEITEM Get_FirstVisible(HWND hWnd);
			HTREEITEM Get_NextVisible(HWND hWnd, HTREEITEM hSomeWhere);
			HTREEITEM Get_PrevVisible(HWND hWnd, HTREEITEM hSomeWhere);
			HTREEITEM Get_LastVisible(HWND hWnd);
			
			void Set_CurrentSelection(HWND hWnd, HTREEITEM hNewSelection);
			HTREEITEM Get_CurrentSelection(HWND hWnd);

			HTREEITEM Add(HWND hWnd, HTREEITEM hParent, const char* szString);
			HTREEITEM Insert(HWND hWnd, HTREEITEM hParent, HTREEITEM hWhere, const char* szString);
			void Delete(HWND hWnd, HTREEITEM hDelete);
			void DeleteAll(HWND hWnd);

			void EnsureVisible(HWND hWnd, HTREEITEM hEnableSelection);
			bool IsExpand(HWND hWnd, HTREEITEM hCheckFor);
			void Expand(HWND hWnd, HTREEITEM hExpand);
			void Collapse(HWND hWnd, HTREEITEM hCollapse);
			void ToggleExpandCollapse(HWND hWnd, HTREEITEM hToggle);

			HTREEITEM Get_ItemFromPos(HWND hWnd, long x, long y);

			bool IsChecked(HWND hWnd, HTREEITEM hCheckFor);
			void Set_Check(HWND hWnd, HTREEITEM hCheck);
			void UnSet_Check(HWND hWnd, HTREEITEM hCheck);

			bool IsValidValue(HTREEITEM hCheckFor);
		}
	}
}
#endif /* _UTIL_H_FE2915E7_4CEA_40EC_B75D_C221A34BB901_ */