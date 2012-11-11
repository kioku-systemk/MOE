#include "stdafx.h"
#include "Util.h"

namespace Renderer{
	namespace GL{
		bool InitOpenGL(HWND hWnd, HGLRC* phRC, int width, int height){
			static PIXELFORMATDESCRIPTOR pfd=
			{
				sizeof(PIXELFORMATDESCRIPTOR),
				1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA,
				24,
				0, 0, 0, 0, 0, 0,
				0,
				0,
				0,
				0, 0, 0, 0,
				16,	//Z-Buffer
				1,  //Stencil Buffer
				0,
				PFD_MAIN_PLANE,
				0, //Reserved
				0, 0, 0	
			};

			HDC hDC=GetDC(hWnd);
			HGLRC hRC;
			if(!SetPixelFormat(hDC,ChoosePixelFormat(hDC,&pfd),&pfd) || !(hRC=wglCreateContext(hDC)) || !wglMakeCurrent(hDC,hRC)){
				return false;
			}
			*phRC = hRC;
			ReleaseDC(hWnd, hDC);

			ResizeWindow(width, height);

			//set initial states
			//glShadeModel(GL_FLAT);
			//glEnable(GL_ALPHA_TEST);
			//glAlphaFunc(GL_GREATER, 0.0001f);
			//glDisable(GL_LIGHTING);
			//glDisable(GL_LIGHT0);
			//glDisable(GL_BLEND);
			//glEnable(GL_DEPTH_TEST);
			//glDepthFunc(GL_ALWAYS);

			//this->ClearScreen();//clear scene buffer.
			//ValidateRect(NULL, NULL);

			return true;//done.
		}

		void ResizeWindow(int iWidth, int iHeight){
			glViewport(0,0, iWidth,iHeight);
		}

		void ViewOrtho(float fWorldWidth, float fWorldHeight, float fNear, float fFar){
			Renderer::GL::MatrixMode(Renderer::GL::PROJECTION);
			Renderer::GL::LoadIdentity();
			glOrtho(0.0, fWorldWidth, 0.0, fWorldHeight, fNear, fFar);
			Renderer::GL::MatrixMode(Renderer::GL::MODELVIEW);
		}

		void ViewPerspective(float fFovy, float fWorldWidth, float fWorldHeight, float fNear, float fFar){
			Renderer::GL::MatrixMode(Renderer::GL::PROJECTION);
			Renderer::GL::LoadIdentity();
			gluPerspective(fFovy, static_cast<double>(fWorldWidth/fWorldHeight), fNear, fFar);
			Renderer::GL::MatrixMode(Renderer::GL::MODELVIEW);
		}

		bool UnInitOpenGL(HGLRC& hRC){
			return (wglMakeCurrent(NULL,NULL) && wglDeleteContext(hRC));
		}

		void ActivateScreen(HWND hWnd, HGLRC& hRC){
			HDC hDC = GetDC(hWnd);
				wglMakeCurrent(hDC, hRC); ///* １スレッドで複数のOpenGLをInitしてるときは，コレを呼び出すとだめみたい（初期化時は必要）@Radeon */
			ReleaseDC(hWnd, hDC);
		}

		void ClearScreen(float r, float g, float b, float a){
			glClearColor(r,g,b,a);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		}

		void RedrawScreen(HWND hWnd/*, HGLRC& hRC*/){
			HDC hDC = GetDC(hWnd);
				SwapBuffers(hDC);
			ReleaseDC(hWnd, hDC);
		}

		void FillScreen(float r, float g, float b, float a){
			Renderer::GL::PushViewMatrix();
			Renderer::GL::ViewOrtho(1.0f, 1.0f);

			Renderer::GL::DrawColor(r, g, b, a);
			Renderer::GL::DrawCenteredRect(1.0f, 1.0f);

			Renderer::GL::PopMatrix();
		}

		void FillScreen(RendererColor& color){
			Renderer::GL::FillScreen(color.r, color.g, color.b, color.a);
		}

		void DrawRect(float width, float height){
			Renderer::GL::DrawRect(0.0f, 0.0f, width, height);
		}

		void DrawRect(float x, float y, float width, float height){
			glBegin(GL_TRIANGLE_STRIP);
				glVertex2f(x,       y+height);
				glVertex2f(x,       y       );
				glVertex2f(x+width, y+height);
				glVertex2f(x+width, y       );
			glEnd();
		}

		void DrawCenteredRect(float width, float height){
			Renderer::GL::DrawCenteredRect(0.0f, 0.0f, width, height);
		}

		void DrawCenteredRect(float xoffset, float yoffset, float width, float height){
			Renderer::GL::DrawCenteredRect(xoffset, yoffset, width, height, 1.0f, 1.0f);
		}

		void DrawCenteredRect(float xoffset, float yoffset, float width, float height, float screenwidth, float screeneheight){
			RendererVector2 v2 = GetCenteredPos(width, height);
			Renderer::GL::DrawRect(v2.x+xoffset, v2.y+yoffset, width, height);
		}

		void LineSmoothState(bool new_state){
			(new_state) ? glEnable(GL_LINE_SMOOTH) : glDisable(GL_LINE_SMOOTH);
		}

		void LineWidth(float fWidth){
			glLineWidth(fWidth);
		}

		void DrawLine(float frmx, float frmy, float tox, float toy){
			glBegin(GL_LINES);
				glVertex2f(frmx, frmy);
				glVertex2f(tox, toy);
			glEnd();
		}

		void DrawColor(float r, float g, float b, float a){
			glColor4f(r, g, b, a);
		}
		
		void DrawColor(RendererColor& color){
			Renderer::GL::DrawColor(color.r, color.g, color.b, color.a);
		}

		RendererVector2 GetCenteredPos(float width, float height){
			return Renderer::GL::GetCenteredPos(width, height, 1.0f, 1.0f);
		}

		RendererVector2 GetCenteredPos(float width, float height, float screenwidth, float screeneheight){
			return RendererVector2((screenwidth - width)/2.0f, (screeneheight - height)/2.0f);
		}

		void Translate(float x, float y, float z){
			glTranslatef(x, y, z);
		}

		void Rotate(float x, float y, float z){
			glRotatef(x, 1.0f, 0.0f, 0.0f);
			glRotatef(y, 0.0f, 1.0f, 0.0f);
			glRotatef(z, 0.0f, 0.0f, 1.0f);
		}

		void Scale(float x, float y, float z){
			glScalef(x, y, z);
		}

		void MatrixMode(Renderer::GL::eMatrixMode matrix_mode){
			glMatrixMode(static_cast<GLenum>(matrix_mode));
		}

		void LoadIdentity(){
			glLoadIdentity();
		}

		void PushMatrix(){
			glPushMatrix();
		}

		void PopMatrix(){
			glPopMatrix();
		}

		void PushViewMatrix(){
			Renderer::GL::MatrixMode(Renderer::GL::PROJECTION);
				glPushMatrix();
			Renderer::GL::MatrixMode(Renderer::GL::MODELVIEW);
		}

		void PopViewMatrix(){
			Renderer::GL::MatrixMode(Renderer::GL::PROJECTION);
				glPopMatrix();
			Renderer::GL::MatrixMode(Renderer::GL::MODELVIEW);
		}

		void PushStates(){
			glPushAttrib(GL_ALL_ATTRIB_BITS);
		}

		void PopStates(){
			glPopAttrib();
		}

		void PostRedisplay(){
			/* not impl */
		}

		void DepthTestState(bool new_state){
			(new_state) ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		}

		void LightingState(bool new_state){
			(new_state) ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
		}

		void LightState(unsigned int light, bool new_state){
			(new_state) ? glEnable(GL_LIGHT0 + light) : glDisable(GL_LIGHT0 + light);
		}

	}
}

namespace String{
	namespace Output{
		namespace GL{
			GLuint m_uiAsciiList = 0;
			const int ASCII_CHARMAX = 127;
			bool m_isCreated = false;

			void __DeleteFontList(){
				if (glIsList(m_uiAsciiList)) {
					glDeleteLists(m_uiAsciiList, ASCII_CHARMAX);
				}
				//if (glIsList(m_uiSjisList)) {
				//	glDeleteLists(m_uiSjisList, SJIS_CHARMAX);
				//}
				m_isCreated = false;
			}

			void __CreateFontList(){
				m_uiAsciiList = glGenLists(ASCII_CHARMAX);
				unsigned int a = m_uiAsciiList;
				//m_uiSjisList = glGenLists(SJIS_CHARMAX);	

				//FillMemory(m_sjisCache, sizeof(GLuint) * SJIS_CHARMAX, CACHE_NONE);

				static LOGFONT logfont;
				logfont.lfHeight		= 0;
				logfont.lfWidth			= 0;
				logfont.lfEscapement	= 0;
				logfont.lfOrientation	= logfont.lfEscapement;
				logfont.lfWeight		= FW_NORMAL;
				logfont.lfItalic		= FALSE;
				logfont.lfUnderline		= FALSE;
				logfont.lfStrikeOut		= FALSE;
				//HDC hDesktopDC = GetDC(HWND_DESKTOP);
				logfont.lfCharSet		= DEFAULT_CHARSET;//GetTextCharset(hDesktopDC);//SHIFTJIS_CHARSET;
				//ReleaseDC(HWND_DESKTOP, hDesktopDC);
				logfont.lfOutPrecision	= OUT_TT_ONLY_PRECIS;
				logfont.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
				logfont.lfPitchAndFamily= FF_DONTCARE | DEFAULT_PITCH;
				logfont.lfQuality		= ANTIALIASED_QUALITY;
				lstrcpy(logfont.lfFaceName, "Courier New");
				//lstrcpy(logfont.lfFaceName, "ＭＳ ゴシック");

				HDC hDC = GetDC(GetDesktopWindow());/* デスクトップを拝借（コモンコントロールのウィンドウでOpenGLを使ってるときに, wglGetCurrentDC()で取得したやつだとだめなことがあった @ Radeon */
				HFONT hOld = (HFONT)SelectObject(hDC, CreateFontIndirect(&logfont));
					BOOL bRet = wglUseFontOutlines(hDC, 0, ASCII_CHARMAX, m_uiAsciiList, 0.0f, 0.0f, WGL_FONT_POLYGONS, NULL);
				DeleteObject(SelectObject(hDC, hOld));
				ReleaseDC(GetDesktopWindow(), hDC);

				m_isCreated = true;
			}

			GLuint __QueryFontChache(unsigned short w){
			//	GLuint uiCache = m_sjisCache[w];
			//	if(uiCache==CACHE_NONE){
			//		HFONT hFont = CreateFontIndirect(&m_logfont);
			//		HFONT oldfont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
			//			wglUseFontOutlines(wglGetCurrentDC(), w, 1, m_uiSjisList + w, 0.0f, 0.0f, WGL_FONT_POLYGONS, NULL);
			//		DeleteObject(SelectObject(wglGetCurrentDC(), oldfont));

			//		uiCache = m_sjisCache[w] = w;
			//	}
			//	return uiCache + m_uiSjisList;	
				return 0;
			}

			void DrawString(const char* format, ...){
				char str[1024];
				va_list ap;
				va_start (ap,format);
				vsprintf(str,format,ap);
				va_end(ap);

				if(!m_isCreated){ String::Output::GL::__CreateFontList(); }

				//if(m_isNeedToRebuildFont){
				//	this->RebuildFont();
				//	m_isNeedToRebuildFont = false;
				//}

				glPushAttrib(GL_POLYGON_BIT);
				glPushMatrix();

				//glEnable(GL_BLEND);
				//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				//glColor4fv(m_fColor);
				//glTranslatef(x, y, 0.0f);
				//glScalef(m_fSizeX, m_fSizeY, 0.0);

				int l = lstrlen(str);
				int i = 0;
				for(int j = 0; j < l; j++) {
					char c = str[i++];

					if (IsDBCSLeadByte(c)) {
	//#define MAKEUSHORT(makeushortu__,makeushortl__) ((unsigned short)((((unsigned char)makeushortu__)<<8) | (((unsigned short)((unsigned char)makeushortl__)))))
						//glCallList( __QueryCache( MAKEUSHORT(c,str[i++]) ) );
	//#undef MAKEUSHORT
						j++;
					} else {
						glCallList(m_uiAsciiList + (unsigned int)c);
					}
				}

				//glDisable(GL_BLEND);

				glPopMatrix();
				glPopAttrib();	
			}
		}

		namespace Win32{
			void MessageBox(const char* format, ...)
			{
				char str[1024];
				va_list ap;
				va_start(ap,format);
				_vsnprintf(str, COUNTOF_ARRAY(str), format, ap);
				va_end(ap);

				::MessageBox(NULL,str,"",0);
			}

			void SetWindowText(HWND hWnd, const char* format, ...)
			{
				char str[1024];
				va_list ap;
				va_start(ap,format);
				_vsnprintf(str, COUNTOF_ARRAY(str), format, ap);
				va_end(ap);

				::SetWindowText(hWnd, str);
			}

			void OutputDebugStringFormatted(const char* format, ...)
			{
				char str[1024];
				va_list ap;
				va_start(ap,format);
				_vsnprintf(str, COUNTOF_ARRAY(str), format, ap);
				va_end(ap);

				::OutputDebugString(str);
			}
		}
	}

	void FormatString(char* szRecv, unsigned long nSize, const char* format, ...){
		va_list ap;
		va_start(ap,format);
		_vsnprintf(szRecv, nSize, format, ap);
		va_end(ap);
	}
}

namespace File{
	namespace Win32{
		namespace Dialog{
			const OPENFILENAME& __GetOpenOFN(HWND hWnd, const char* szFilter, char* szFileNameBuffer, const int nSizeBuffer, BOOL isAllowMultipleSelection){
				static OPENFILENAME  ofn;
				ZeroMemory( &ofn, sizeof(OPENFILENAME) );
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = szFilter;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_HIDEREADONLY;
				if(isAllowMultipleSelection){ ofn.Flags |= OFN_ALLOWMULTISELECT; }
				ofn.lpstrFile = szFileNameBuffer;
				ofn.nMaxFile = nSizeBuffer;
				ofn.lpstrDefExt = "";//none
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.lpstrFileTitle = NULL; //ファイル名
				ofn.lpstrTitle = "Open";

				return ofn;
			}

			const OPENFILENAME& __GetSaveOFN(HWND hWnd, const char* szFilter, char* szFileNameBuffer, const int nSizeBuffer, BOOL isAllowMultipleSelection){
				static OPENFILENAME  ofn;
				ZeroMemory( &ofn, sizeof(OPENFILENAME) );
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = szFilter;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_HIDEREADONLY;
				if(isAllowMultipleSelection){ ofn.Flags |= OFN_ALLOWMULTISELECT; }
				ofn.lpstrFile = szFileNameBuffer;
				ofn.nMaxFile = nSizeBuffer;
				ofn.lpstrDefExt = "";//none
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.lpstrFileTitle = NULL; //ファイル名
				ofn.lpstrTitle = "Save";

				return ofn;
			}

			const char* __MakeFilterString(const DialogFileTypeParam& dp){/* std::string や lstrXXX は使えない */
				static char szFilter[1024];

				lstrcpy(szFilter, "");
				int nFilterCnt = 0;

				DialogFileTypeParam* pDp = const_cast<DialogFileTypeParam*>(&dp);
				while(pDp){
					char szAppendFilter[1024] = {'\0'};
					int nAppendCnt = 0;

					for(int i=0; i<lstrlen(pDp->szDescription); i++){
						szAppendFilter[nAppendCnt++] = pDp->szDescription[i];
					}

					const char* szToken1 = "(";
					const int   nTokenSize1 = 1;
					for(int i=0; i<nTokenSize1; i++){
						szAppendFilter[nAppendCnt++] = szToken1[i];
					}
					//const char* szToken1 = "(*.";
					//const int   nTokenSize1 = 3;
					//for(int i=0; i<nTokenSize1; i++){
					//	szAppendFilter[nAppendCnt++] = szToken1[i];
					//}

					for(int i=0; i<lstrlen(pDp->szExt); i++){
						szAppendFilter[nAppendCnt++] = pDp->szExt[i];
					}

					const char* szToken2 = ")\0";
					const int   nTokenSize2 = 2;
					for(int i=0; i<nTokenSize2; i++){
						szAppendFilter[nAppendCnt++] = szToken2[i];
					}

					//const char* szToken3 = "*.";
					//const int   nTokenSize3 = 2;
					//for(int i=0; i<nTokenSize3; i++){
					//	szAppendFilter[nAppendCnt++] = szToken3[i];
					//}

					for(int i=0; i<lstrlen(pDp->szExt); i++){
						szAppendFilter[nAppendCnt++] = pDp->szExt[i];
					}

					const char* szToken4 = "\0";
					const int   nTokenSize4 = 1;
					for(int i=0; i<nTokenSize4; i++){
						szAppendFilter[nAppendCnt++] = szToken4[i];
					}

					for(int i=0; i<nAppendCnt; i++){
						szFilter[nFilterCnt++] = szAppendFilter[i];
					}

					pDp = pDp->next;
				}
				szFilter[nFilterCnt++] = '\0';

				return static_cast<const char*>(szFilter);
				//return "map object file(*,map)\0*.map\0\0";//static_cast<const char*>(szFilter);
			}

			void __CreateDialogFileTypeParam(DialogFileTypeParam** ppDp, const char* szDescription, const char* szExt){
				(*ppDp) = new DialogFileTypeParam;
				(*ppDp)->szDescription = new char[lstrlen(szDescription)+1];
				lstrcpy((*ppDp)->szDescription, szDescription);
				(*ppDp)->szExt = new char[lstrlen(szExt)+1];
				lstrcpy((*ppDp)->szExt, szExt);
				(*ppDp)->next = NULL;
			}

			bool MakeDialogFileTypeParam(DialogFileTypeParam** ppDpHead, const char* szDescription, const char* szExt){
				if(ppDpHead == NULL){ return false; }

				if((*ppDpHead) == NULL){
					__CreateDialogFileTypeParam(ppDpHead, szDescription, szExt);
				}else{
					DialogFileTypeParam* pDp = (*ppDpHead);
					DialogFileTypeParam* pDpBefore = NULL;

					while(pDp){
						pDpBefore = pDp;
						pDp = pDp->next;
					}

					__CreateDialogFileTypeParam(&pDp, szDescription, szExt);
					pDpBefore->next = pDp;
				}
				//*ppDpHead = pDp;
				return true;
			}

			bool FreeDialogFileTypeParam(const DialogFileTypeParam* pDpHead){
				DialogFileTypeParam* pDp = const_cast<DialogFileTypeParam*>(pDpHead);
				DialogFileTypeParam* pDpTmp;
				while(pDp){
					delete[] pDp->szDescription;
					delete[] pDp->szExt;
					pDpTmp = pDp->next;
					delete pDp;

					pDp = pDpTmp;
				}
				return true;
			}

			void __SplitMultipleFilePath(const char* szMultipleFileBuffer, char** szPathName/*[MAX_PATH]*/, int* nNum){
				char szFolder[MAX_PATH];
				const char* lpEnd = strchr( szMultipleFileBuffer, '\0' );
				char* lpszNextString = const_cast<char*>(lpEnd + 1);
				if(lpEnd){
					if(*(lpszNextString)=='\0'){ //  ファイルが１つしか選択されていない
						lstrcpy(szPathName[0], szMultipleFileBuffer);
						if(nNum){ *nNum = 1; }
					}else{	// ファイルが複数選択されている
						int nLength;

						// フォルダ名の取得
						lstrcpy( szFolder, szMultipleFileBuffer );
						nLength = lstrlen( szFolder );
						// 「\」記号で終わっていなければ付加
						if( szFolder[nLength] != '\\' ){
							lstrcat( szFolder, "\\" );
						}

						int nCnt;
						for(nCnt=0; (*(lpszNextString)!='\0'); nCnt++){
							// 選択されたファイルのパス名を作成
							lstrcpy( szPathName[nCnt], szFolder );
							lstrcat( szPathName[nCnt], lpszNextString );
							// 次のファイル名を探す
							lpEnd = strchr( lpszNextString, '\0' );
							lpszNextString = const_cast<char*>(lpEnd + 1);
						}
						*nNum = nCnt++;
					}
				}else{
					if(nNum){ *nNum = 0; }
				}			
			}

			bool Open(HWND hWnd, const DialogFileTypeParam* dp, char** szPathName/*[MAX_PATH]*/, int* nSelection){
				char* szFileNameBuffer = new char[File::Win32::Dialog::SIZE_FILENAME_BUFFER];
				lstrcpy(szFileNameBuffer, "");// we must do this to get GetOpenFileName API works.

				const char* szFilter = __MakeFilterString(*dp);
				BOOL isAllowMultipleSelection = nSelection ? TRUE : FALSE;
				OPENFILENAME ofn = __GetOpenOFN(hWnd, szFilter, szFileNameBuffer, File::Win32::Dialog::SIZE_FILENAME_BUFFER, isAllowMultipleSelection);

				bool isWentOK;
				if(GetOpenFileName( &ofn )==TRUE){
					if(isAllowMultipleSelection){
						__SplitMultipleFilePath(szFileNameBuffer, szPathName, nSelection);
					}else{
						lstrcpy(szPathName[0], szFileNameBuffer);
					}
					isWentOK = true;
				}else{
					isWentOK = false;
				}

				delete[] szFileNameBuffer;
				return isWentOK;
			}

			bool Save(HWND hWnd, const DialogFileTypeParam* dp, char** szPathName/*[MAX_PATH]*/, int* nSelection){
				char* szFileNameBuffer = new char[File::Win32::Dialog::SIZE_FILENAME_BUFFER];
				lstrcpy(szFileNameBuffer, "");// we must do this to get GetOpenFileName API works.

				const char* szFilter = __MakeFilterString(*dp);
				BOOL isAllowMultipleSelection = nSelection ? TRUE : FALSE;
				OPENFILENAME ofn = __GetSaveOFN(hWnd, szFilter, szFileNameBuffer, File::Win32::Dialog::SIZE_FILENAME_BUFFER, isAllowMultipleSelection);

				bool isWentOK;
				if(GetSaveFileName( &ofn )==TRUE){
					if(isAllowMultipleSelection){
						__SplitMultipleFilePath(szFileNameBuffer, szPathName, nSelection);
					}else{
						lstrcpy(szPathName[0], szFileNameBuffer);
					}
					isWentOK = true;
				}else{
					isWentOK = false;
				}

				delete[] szFileNameBuffer;
				return isWentOK;
			}

			bool Open(HWND hWnd, const DialogFileTypeParam* dp, std::string* strPathName/* [nMaxSelection] */, int* nSelection){
				char** szPathName = new char*[File::Win32::Dialog::MAX_SELECTION_OF_FILE];
				for(int i=0; i<File::Win32::Dialog::MAX_SELECTION_OF_FILE; i++){
					szPathName[i] = new char[MAX_PATH];
				}

				bool isWentOK;
				if(File::Win32::Dialog::Open(hWnd, dp, szPathName, nSelection)){
					if(nSelection){
						for(int i=0; i<*nSelection; i++){
							strPathName[i] = szPathName[i];
						}
					}else{
						strPathName[0] = szPathName[0];
					}
					isWentOK = true;
				}else{
					isWentOK = false;
				}

				for(int i=0; i<File::Win32::Dialog::MAX_SELECTION_OF_FILE; i++){
					delete[] szPathName[i];
				}
				delete[] szPathName;
				return isWentOK;
			}

			bool Save(HWND hWnd, const DialogFileTypeParam* dp, std::string* strPathName/* [nMaxSelection] */, int* nSelection){
				char** szPathName = new char*[File::Win32::Dialog::MAX_SELECTION_OF_FILE];
				for(int i=0; i<File::Win32::Dialog::MAX_SELECTION_OF_FILE; i++){
					szPathName[i] = new char[MAX_PATH];
				}

				bool isWentOK;
				if(File::Win32::Dialog::Save(hWnd, dp, szPathName, nSelection)){
					if(nSelection){
						for(int i=0; i<*nSelection; i++){
							strPathName[i] = szPathName[i];
						}
					}else{
						strPathName[0] = szPathName[0];
					}
					isWentOK = true;
				}else{
					isWentOK = false;
				}

				for(int i=0; i<File::Win32::Dialog::MAX_SELECTION_OF_FILE; i++){
					delete[] szPathName[i];
				}
				delete[] szPathName;
				return isWentOK;
			}

		}
	}

	bool IsEof(FILE* fp){
		long fpos = ftell(fp);
		fgetc(fp);
		if(feof(fp)==0){
			fseek(fp, fpos, SEEK_SET);
			return false;
		}
		return true;
	}

}

namespace System{
	namespace Win32{
		void SetForegroundWindowEx(HWND hTargetWnd, DWORD dwForegroundThreadID){
			DWORD dwTargetID, dwForegroundID;
			DWORD sp_time = 0;

			if(dwForegroundThreadID!=0){
				dwForegroundID = dwForegroundThreadID;
			}else{
				dwForegroundID = GetCurrentThreadId();
			}
			dwTargetID = GetWindowThreadProcessId(hTargetWnd, NULL );
			AttachThreadInput(dwTargetID, dwForegroundID, TRUE );
			SystemParametersInfo( SPI_GETFOREGROUNDLOCKTIMEOUT,0,&sp_time,0);
			SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT,0,(LPVOID)0,0);
			SetForegroundWindow(hTargetWnd);
			SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT,0,&sp_time,0);
			AttachThreadInput(dwTargetID, dwForegroundID, FALSE );
			//BringWindowToTop(hTargetWnd);
		}
	}
}

namespace Control{
	namespace Win32{
		namespace ListBox{
			int __HLISTITEMtoINT(HLISTITEM hListItem){
				return static_cast<int>(hListItem);
			}
			HLISTITEM __INTtoHLISTITEM(int nListItem){
				return static_cast<HLISTITEM>(nListItem);
			}
			
			void Set_CurrentSelection(HWND hWnd, HLISTITEM hNewSelection){
				ListBox_SetCurSel(hWnd, __HLISTITEMtoINT(hNewSelection));
			}
			HLISTITEM Get_CurrentSelection(HWND hWnd){
				return __INTtoHLISTITEM( ListBox_GetCurSel(hWnd) );
			}

			HLISTITEM Add(HWND hWnd, const char* szString){
				return __INTtoHLISTITEM( ListBox_AddString(hWnd, szString) );
			}
			HLISTITEM Insert(HWND hWnd, HLISTITEM hWhere, const char* szString){
				return __INTtoHLISTITEM( ListBox_InsertString(hWnd, __HLISTITEMtoINT(hWhere), szString) );
			}
			void Delete(HWND hWnd, HLISTITEM hDelete){
				ListBox_DeleteString(hWnd, __HLISTITEMtoINT(hDelete));
			}
			void DeleteAll(HWND hWnd){
				ListBox_ResetContent(hWnd);
			}

			bool IsValidValue(HLISTITEM hCheckFor){
				return (__HLISTITEMtoINT(hCheckFor) >= 0);
			}
			int Get_Index(HLISTITEM hConvert){
				return __HLISTITEMtoINT(hConvert);
			}
			int Get_PrevIndex(HLISTITEM hNext){
				return __HLISTITEMtoINT(hNext-1);
			}
			int Get_NextIndex(HLISTITEM hPrevious){
				return __HLISTITEMtoINT(hPrevious+1);
			}
		}
		namespace TreeView{
			HTREEITEM Get_Root(HWND hWnd){
				return TreeView_GetRoot(hWnd);
			}
			HTREEITEM Get_Parent(HWND hWnd, HTREEITEM hChild){
				return TreeView_GetParent(hWnd, hChild);
			}
			HTREEITEM Get_Child(HWND hWnd, HTREEITEM hParent){
				return TreeView_GetChild(hWnd, hParent);
			}
			HTREEITEM Get_PrevSibling(HWND hWnd, HTREEITEM hSomeWhere){
				return TreeView_GetPrevSibling(hWnd, hSomeWhere);
			}
			HTREEITEM Get_NextSibling(HWND hWnd, HTREEITEM hSomeWhere){
				return TreeView_GetNextSibling(hWnd, hSomeWhere);
			}
			HTREEITEM Get_FirstVisible(HWND hWnd){
				return TreeView_GetFirstVisible(hWnd);
			}
			HTREEITEM Get_NextVisible(HWND hWnd, HTREEITEM hSomeWhere){
				return TreeView_GetNextVisible(hWnd, hSomeWhere);
			}
			HTREEITEM Get_PrevVisible(HWND hWnd, HTREEITEM hSomeWhere){
				return TreeView_GetPrevVisible(hWnd, hSomeWhere);
			}
			HTREEITEM Get_LastVisible(HWND hWnd){
				return TreeView_GetLastVisible(hWnd);
			}

			void Set_CurrentSelection(HWND hWnd, HTREEITEM hNewSelection){
				if( hNewSelection==HTREEITEM_ROOT ){ hNewSelection = Get_FirstVisible(hWnd); }
				else if( hNewSelection==HTREEITEM_LAST ){ hNewSelection = Get_LastVisible(hWnd); }
				TreeView_Select(hWnd, hNewSelection, TVGN_CARET);
			}
			HTREEITEM Get_CurrentSelection(HWND hWnd){
				return TreeView_GetSelection(hWnd);
			}

			HTREEITEM Add(HWND hWnd, HTREEITEM hParent, const char* szString){
				return Control::Win32::TreeView::Insert(hWnd, hParent, TVI_LAST, szString);
			}
			HTREEITEM Insert(HWND hWnd, HTREEITEM hParent, HTREEITEM hWhere, const char* szString){
				TVINSERTSTRUCT is;
				ZeroMemory(&is, sizeof(TVINSERTSTRUCT));

				is.hParent		= hParent;
				is.hInsertAfter = hWhere;
				is.item.mask	= TVIF_TEXT;
				is.item.pszText = const_cast<char*>(szString);
				return TreeView_InsertItem(hWnd, &is);
			}
			void Delete(HWND hWnd, HTREEITEM hDelete){
				TreeView_DeleteItem(hWnd, hDelete);
			}
			void DeleteAll(HWND hWnd){
				TreeView_DeleteAllItems(hWnd);
			}

			void EnsureVisible(HWND hWnd, HTREEITEM hEnableSelection){
				TreeView_EnsureVisible(hWnd, hEnableSelection);
			}

			bool IsExpand(HWND hWnd, HTREEITEM hCheckFor){
				TVITEM tvItem;
				ZeroMemory(&tvItem, sizeof(TVITEM));

				tvItem.mask = TVIF_HANDLE | TVIF_STATE;
				tvItem.hItem = hCheckFor;
				TreeView_GetItem(hWnd, &tvItem);
				return ((tvItem.state & TVIS_EXPANDED)==1);
			}
			void Expand(HWND hWnd, HTREEITEM hExpand){
				TreeView_Expand(hWnd, hExpand, TVE_EXPAND);
			}
			void Collapse(HWND hWnd, HTREEITEM hCollapse){
				TreeView_Expand(hWnd, hCollapse, TVE_COLLAPSE);
			}
			void ToggleExpandCollapse(HWND hWnd, HTREEITEM hToggle){
				TreeView_Expand(hWnd, hToggle, TVE_TOGGLE);
			}

			HTREEITEM Get_ItemFromPos(HWND hWnd, long x, long y){
				TVHITTESTINFO ht;
				ZeroMemory(&ht, sizeof(TVHITTESTINFO));

				ht.pt.x = x;
				ht.pt.y = y;
				TreeView_HitTest(hWnd, &ht);
				return ht.hItem;
			}

			bool IsChecked(HWND hWnd, HTREEITEM hCheckFor){
				TVITEM tvItem;
				ZeroMemory(&tvItem, sizeof(TVITEM));

				HTREEITEM hItem = hCheckFor;
				tvItem.mask = TVIF_HANDLE | TVIF_STATE;
				tvItem.hItem = hItem;
				tvItem.stateMask = TVIS_STATEIMAGEMASK;

				TreeView_GetItem(hWnd, &tvItem);
				return ( ((tvItem.state >> 12)-1) == 1 );
			}
			void Set_Check(HWND hWnd, HTREEITEM hCheck){
				TreeView_SetCheckState(hWnd, hCheck, TRUE);
			}
			void UnSet_Check(HWND hWnd, HTREEITEM hCheck){
				TreeView_SetCheckState(hWnd, hCheck, FALSE);
			}

			bool IsValidValue(HTREEITEM hCheckFor){
				return (hCheckFor!=NULL);
			}
		}
	}
}