#include "stdafx.h"
#include "common.h"
#include "MainWindow.h"
#include "Draw.h"
#include "Convert.h"
#include <olectl.h>

const char* parser_with_explanation[parser_num] = {
"circle(int x, int y, int radius);",
"ellipse(int x, int y, int radius_x, int radius_y);",
"rectangle(int x, int y, int width, int height);",
"text(int x, int y, int fontheight, int isBold, int isItalic, int isMakeCenterX, int isMakeCenterY, char* text);",
"color(UCHAR r, UCHAR g, UCHAR b, UCHAR a);",
"fill(UCHAR r, UCHAR g, UCHAR b, UCHAR a);",
"reserved(DON'T USE);",
"reserved(DON'T USE);",
"perlin(int start_r, int end_r, int mix);",
"sinenv(int brightness, int width, int height);",
"sinplasma(int scale, int offset_x, int offset_y);",
"operator(r,+,-,*,/,%,&,|,^,~);",
"update(void);",
"transparent(UCHAR r, UCHAR g, UCHAR b, UCHAR a);",
"push(void);",
"pop(void);",
"invert(void);",
"changechannel(R|G|B|A);",
"channel(R|G|B|A);",
"op(OPERATOR);",
"emboss(int strength);",
"roundrect(int x, int y, int width, int height, int round_w, int round_h)",
"edge(int strength);",
"sharp(int strength, int center_strength);",
"reserved(DON'T USE);",
"check(int div_x, int div_y, UCHAR r0, g0, b0, a0, UCHAR r1, g1, b1, a1)",
"srand(int new_seed);",
"rect(int x, int y, int width, int height);",
"font(char* szFontName)",
"polygon(nVertex, x,y, ...)",
"rgbtoa(void);",
"mono(void);",
"move(int x, int y, int isWrap)",
"blur(int x_strength, int y_strength);",
"normal(void);",
"anti(void);",
"polyline(nVertex, x,y, ...);"
};

//#pragma comment(lib, "shlwapi")
//#pragma comment(lib, "comctl32.lib")
#define APP_VER 0.84
#define APP_TTL "Texture Generator"
#define DEFAULT_FORMULA "check(50,50,0,0,0,0,255,255,255,255);\r\noperator(*);\r\nsinenv(255,255,255);\r\nop(r);\r\ntext(0,80,30,0,1,1,\"texture generator\");"
#define KTF_SZHEADER "KTF"
#define KTF_VERSIONIDENTIFIER '!'
#define KTF_VERSION_OBSOLEET 1.0f

#define TEXT_UNDO_LIMIT 1024

void SaveBMP(const char* filename, int width, int height, int bpp, int r = -1, int g = -1, int b = -1);
LRESULT CALLBACK HWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void OpenTextTexture(const char* szFile);
void InitGUI();
void SetupDialog();
void MessageLoop();
void SendToClipboard(const char* szSrc);
void VerifyOnExit();

CWindowGL win;
KTextureEdit ktex;
HWND g_hDlg = NULL;
HACCEL volatile hAccel = NULL;
HACCEL hMainAccel = NULL;

WNDPROC g_OldEditProc;
HMODULE g_hRichEditLib;

char* g_szTextureString;
char g_szLastFileName[MAX_PATH] = {'\0'};
extern UINT tex_name;
extern char g_szFinalTexture[TEXT_BUFFER_SIZE];

bool isAnimating;
bool isCompiling;

bool isCapturing = false;
POINT* mousept = NULL;
long currentpoint = -1;
long current_function = -1;

extern UINT under_tex;
extern float under_tex_alpha;
extern float alphatex_alpha;
extern float mastertex_alpha;

void Render() //OnDraw()を呼びたいだけ
{
	InvalidateRect(hMainWnd, NULL, FALSE);
}

void InitGUI() //ウィンドウ周りの初期化
{
	//GUI
	HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU));
	//HMENU hSubMenu = GetSubMenu(hMenu, 0);
	//EnableMenuItem(hSubMenu, 0, MF_BYPOSITION|MF_GRAYED);
	//EnableMenuItem(hSubMenu, 1, MF_BYPOSITION|MF_GRAYED);
	//EnableMenuItem(hSubMenu, 2, MF_BYPOSITION|MF_GRAYED);
	//EnableMenuItem(hSubMenu, 3, MF_BYPOSITION|MF_GRAYED);
	SetMenu(hMainWnd, hMenu);

	hAccel = LoadAccelerators(GetModuleHandle(NULL), "MYACCEL");
	hMainAccel = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_ACCELERATOR1));

	//テクスチャ情報入力ダイアログ作成
	g_szTextureString = (char*)GlobalAlloc(GPTR, sizeof(char) * TEXT_BUFFER_SIZE);
	lstrcpy(g_szTextureString, DEFAULT_FORMULA);

	g_hRichEditLib = LoadLibrary("riched20.dll");
	SetupDialog();
	//ktex.SetTextureSize(32,32);
}

void SendToClipboard(const char* szSrc)
{
	HGLOBAL	hGlobal;
	LPSTR szStr;
	int	i;
	hGlobal	= GlobalAlloc(GHND,	lstrlen(szSrc) + 1);
	if(hGlobal==NULL)return;

	szStr =	(LPSTR)GlobalLock(hGlobal);
	for	(i=0; i<lstrlen(szSrc)+1; i++) szStr[i] = szSrc[i];
	GlobalUnlock(hGlobal);
	
	if(!OpenClipboard(hMainWnd)){
		GlobalFree(hGlobal);
		return;
	}
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hGlobal);
	CloseClipboard();
}

void ChooseAndSetEditFont(HWND hRichEdit)
{
	if(hRichEdit==NULL) return;
	CHARFORMAT cfm;
	CHOOSEFONT cf;
	LOGFONT	lf;
	HDC	hDC;

	ZeroMemory(&cf, sizeof(CHOOSEFONT));
	ZeroMemory(&lf,	sizeof(LOGFONT));
	cfm.cbSize = sizeof(CHARFORMAT);

	DWORD dwMask = CFM_BOLD | CFM_CHARSET | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT | CFM_UNDERLINE;
	
	//今までの設定を取得してそれをCHOOSEFONT構造体に渡す
	SendMessage(hRichEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&cfm);
	hDC	= GetDC(hRichEdit);
		lf.lfHeight	= MulDiv(cfm.yHeight, GetDeviceCaps(hDC, LOGPIXELSY), -1440);
	ReleaseDC(hRichEdit, hDC);
	
	cfm.dwMask = dwMask;
	if(cfm.dwEffects &	CFE_BOLD)		lf.lfWeight	= FW_BOLD;
	else								lf.lfWeight	= FW_NORMAL;
	if(cfm.dwEffects &	CFE_ITALIC)		lf.lfItalic	= TRUE;
	if(cfm.dwEffects &	CFE_UNDERLINE)	lf.lfUnderline = TRUE;
	if(cfm.dwEffects &	CFE_STRIKEOUT)	lf.lfStrikeOut = TRUE;
	lf.lfCharSet		= cfm.bCharSet;
	lf.lfQuality		= DEFAULT_QUALITY;
	lf.lfPitchAndFamily	= cfm.bPitchAndFamily;
	lstrcpy(	lf.lfFaceName, cfm.szFaceName );
	cf.rgbColors		= cfm.crTextColor;
	cf.lStructSize		= sizeof(CHOOSEFONT);
	cf.hwndOwner		= hRichEdit;
	cf.lpLogFont		= &lf;
	cf.Flags			= CF_SCREENFONTS |	CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
	
	//フォント選択ダイアログを出して新しい設定を取得する
	if(ChooseFont(&cf))
	{
		cfm.cbSize = sizeof(CHARFORMAT);
		cfm.dwMask = dwMask;
		cfm.yHeight	= 2	* cf.iPointSize;
		cfm.dwEffects =	0;
		if(lf.lfWeight >= FW_BOLD)	cfm.dwEffects |= CFE_BOLD;
		if(lf.lfItalic)				cfm.dwEffects |= CFE_ITALIC;
		if(lf.lfUnderline)			cfm.dwEffects |= CFE_UNDERLINE;
		if(lf.lfStrikeOut)			cfm.dwEffects |= CFE_STRIKEOUT;
		cfm.crTextColor		= (COLORREF)cf.rgbColors;
		cfm.bPitchAndFamily	= lf.lfPitchAndFamily;
		cfm.bCharSet		= lf.lfCharSet;
		lstrcpy(cfm.szFaceName, lf.lfFaceName);
		SendMessage(hRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfm);
		//SendMessage(hRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);
	}
	SetFocus(hRichEdit);
}

//テクスチャ式入力用ダイアログ作成＆エディットボックスフォント変更＆フォーカスあわせ
void SetupDialog()
{
	//if(g_hDlg)// すでにあるなら一度破棄する
	//{
	//	DestroyWindow(g_hDlg);
	//	g_hDlg = NULL;
	//}
	if(g_hDlg){
		SetFocus(g_hDlg);
		return;
	}

	//設定用ダイアログ生成
	g_hDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG), hMainWnd, (DLGPROC)DialogProc);
	ShowWindow(g_hDlg, TRUE);

	//HFONT hFont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, SHIFTJIS_CHARSET,
 //               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
 //               DEFAULT_PITCH, "ＭＳ ゴシック");
	HWND hEdit = GetDlgItem(g_hDlg, IDC_EDIT1);
	//SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, (LPARAM)MAKELONG(TRUE, 0));
	//SendMessage(hEdit, EM_SETLIMITTEXT, (WPARAM)TEXT_BUFFER_SIZE, 0);

	//CHARFORMAT cfm;
	//ZeroMemory(&cfm, sizeof(CHARFORMAT));
	//cfm.cbSize = sizeof(CHARFORMAT);
	//SendMessage(hEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&cfm);
	//
	////cfm.dwMask = CFM_CHARSET | CFM_COLOR | CFM_FACE | CFM_SIZE;
	//cfm.dwMask = CFM_BOLD | CFM_CHARSET | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT | CFM_UNDERLINE;;
	//cfm.bCharSet = SHIFTJIS_CHARSET;
 //   lstrcpy(cfm.szFaceName, TEXT("ＭＳ ゴシック"));
	//cfm.yHeight = 20*10;

	//if(SendMessage(hEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfm)==0)
	//	MessageBox(0,0,0,0);

	CHARFORMAT cfm;
    memset(&cfm, 0, sizeof(CHARFORMAT));
	cfm.cbSize = sizeof(CHARFORMAT);
	cfm.dwMask = CFM_BOLD | CFM_CHARSET | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT | CFM_UNDERLINE;
	cfm.yHeight	= 20 * 10;
	cfm.dwEffects =	0;
	cfm.crTextColor		= (COLORREF)0;
	cfm.bPitchAndFamily	= 49;
	cfm.bCharSet		= 0;
	lstrcpy(cfm.szFaceName, "Courier New");
	SendMessage(hEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfm);

	//SendMessage(hEdit, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cfm);
	SendMessage(hEdit, EM_EXLIMITTEXT, (WPARAM)TEXT_BUFFER_SIZE, 0);
	SendMessage(hEdit, EM_SETUNDOLIMIT, (WPARAM)TEXT_UNDO_LIMIT, 0);
	SetWindowText(hEdit, g_szTextureString);

	SetFocus(hEdit);
}


//EditBox Procedure - Subclassed
LRESULT CALLBACK EditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONUP:{
	//case WM_CHAR:
		//if(wParam == VK_TAB)
		//{
		//	
		//	return 0;
		//}
		//win.CSetWindowText(hMainWnd, "%c", wParam);
		long nSelStart, nSelEnd;
		SendMessage(hWnd, EM_GETSEL, (WPARAM)&nSelStart, (LPARAM)&nSelEnd);
		long nLine;
		nLine = (long)SendMessage(hWnd, EM_LINEFROMCHAR, -1, 0);
		nLine += 1;
		long nLineIndex = 0;
		nLineIndex = (long)SendMessage(hWnd, EM_LINEINDEX, nLine -1, 0);
		long cy = nLine;
		long cx = nSelStart - nLineIndex + 1;//一番左にあるときに1が入る
		long abx = nSelStart; //絶対的なX
		
		//current_function
		char string[TEXT_BUFFER_SIZE] = {'\0'};
		int i, j;
		GetDlgItemText(g_hDlg, IDC_EDIT1, string, TEXT_BUFFER_SIZE * sizeof(char));

		//RemoveCharFromString(string, '\r', 0);
		int cppcommentret = 0;
		int ccommentret = 0;
		int commentdif = RemoveCPPCommentFromString(string, &cppcommentret);
		commentdif+= RemoveCCommentFromString(string, &ccommentret);
		commentdif-=cppcommentret;
		commentdif-=ccommentret;

		int diff = RemoveCaridgeReturnFromString(string);
		abx-=(nLine-1)*1;
		if(commentdif>=0) abx-=commentdif;

		//char asdf = string[abx];
		static char checkstr[TEXT_BUFFER_SIZE] = {'\0'};
		//for(i=abx; string[i]!='\0'; i++){
		//	if(string[i]=='('
		//}
		for(i=abx; i>=0; i--){
			//if((string[i]=='\n' && string[i-1]=='\r')//){
			//	i-=2;
			//}
			if(string[i]==';' || i==0){

				ZeroMemory(checkstr, sizeof(checkstr));
				int checkstrptr=0;
				for(j=i; j<=abx; j++){
					if(string[j]=='('){
						break;
					}
					//if(string[j]!=';' && (string[j]!='\n' && string[j-1]!='\r')){
					if(string[j]!=';'){
						checkstr[checkstrptr++] = string[j];
					}else{
					
					}
				}
				BOOL isFound = FALSE;
				for(j=0; j<parser_num; j++){
					//if(0 == strnicmp(checkstr, parser[j], strlen(parser[j]))){
					if(StrStrI(parser[j], checkstr)!=NULL){
						current_function = j;
						isFound = TRUE;
						break;
					}else{
						//nop
					}
				}
				if(isFound) break;//get out of loop and this procedure.

			}
		}

		//if(current_function>=0){
		//win.CSetWindowText(hMainWnd, "%d, %c", commentdif, string[abx]);
		//win.CSetWindowText(hMainWnd, "%s", checkstr);
		//}
		if(current_function>=0){
			win.CSetWindowText(GetDlgItem(g_hDlg, IDC_SUGGEST), "suggestion:%s", parser_with_explanation[current_function]);
		}
	}break;
	}
	return CallWindowProc(g_OldEditProc, hWnd, msg, wParam, lParam);
}

extern int g_ErrReason;
extern int g_ErrSymbol;
extern enum g_eErr;
void ShowError(HWND hWnd)
{
	const char err_string[9][128] = {"No Error.", "Less Argument.", "Over Argument.", "Number of Argument is wrong.", "Symbol Not Found.", "Opened Arc \'(\' Not Found.", "Closed Arc \')\' Not Found.", "Unexpected EOS Found.", "Incorrect Fuction Call."};
	win.CSetWindowText(hWnd, "%s : %s", err_string[g_ErrReason], (g_ErrSymbol>=0) ? parser[g_ErrSymbol] : "");
}

static int lastwindowsize = ID_WINDOWSIZE_X1;

//ダイアログプロシージャ
LRESULT CALLBACK DialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:{
			RECT WindowRect;
			GetWindowRect(hMainWnd, &WindowRect);
			SetWindowPos(hWnd, 0, WindowRect.left+258, WindowRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
			
			g_OldEditProc = (WNDPROC)GetWindowLong(GetDlgItem(hWnd, IDC_EDIT1), GWL_WNDPROC);
			SetWindowLong(GetDlgItem(hWnd, IDC_EDIT1), GWL_WNDPROC, (LONG)EditProc);
			return TRUE;
	   }
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case ID_SELECT_ALL:
					SendMessage(GetDlgItem(hWnd, IDC_EDIT1), EM_SETSEL, 0, -1);
					break;
				case ID_DO_EFFECT:
				{
					if(isCompiling){
						return FALSE;
					}
					isCompiling = true;
					win.CSetWindowText(hWnd, "Compiling...");
					
					ZeroMemory(g_szTextureString, TEXT_BUFFER_SIZE * sizeof(char));
					GetDlgItemText(hWnd, IDC_EDIT1, g_szTextureString, TEXT_BUFFER_SIZE * sizeof(char));
					UpdateTexture(g_szTextureString);
					ShowError(hWnd);

					//SendMessage(hMainWnd, WM_COMMAND, lastwindowsize, 0);
					SetFocus(GetDlgItem(hWnd, IDC_EDIT1));
					isCompiling = false;
					break;
				}
				//case ID_TAB_PRESSED:
				//	SetFocus(::GetNextDlgTabItem(hWnd, GetDlgItem(hWnd, IDC_EDIT1), FALSE));
				//	break;
				case IDC_SIZE_WIDTH:
					break;
				case IDC_SIZE_HEIGHT:
					break;
				case IDC_SIZE_OK:{
					long width = GetDlgItemInt(hWnd, IDC_SIZE_WIDTH, NULL, FALSE);
					long height = GetDlgItemInt(hWnd, IDC_SIZE_HEIGHT, NULL, FALSE);

					LONG style = GetWindowLong(hMainWnd, GWL_STYLE);
					LONG exstyle = GetWindowLong(hMainWnd, GWL_EXSTYLE);
					RECT WindowRect;
					SetRect(&WindowRect, 0, 0, width, height+GetSystemMetrics(SM_CYMENU));
					AdjustWindowRectEx(&WindowRect, style, FALSE, exstyle);
					SetWindowPos(hMainWnd, 0, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, SWP_NOZORDER|SWP_NOMOVE);					
					break;
				 }
				default:
					SendMessage(win.CGethWnd(), WM_COMMAND, wParam, 0);
					break;
			}
			return TRUE;
		}
		case WM_KEYDOWN:
			if(wParam==VK_TAB)
			{
				//keybd_event(VK_TAB, VK_CONTROL, KEYEVENTF_EXTENDEDKEY | 0, 0);
				//keybd_event(VK_TAB, VK_CONTROL, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				//SendMessage(GetDlgItem(hWnd, IDC_EDIT1), WM_KEYDOWN, (WPARAM)VK_TAB, 0);
				//SendMessage(GetDlgItem(hWnd, IDC_EDIT1), WM_KEYUP, (WPARAM)VK_TAB, 0);
//				SendMessage(GetDlgItem(hWnd, IDC_EDIT1), EM_SETCHAR, (WPARAM)VK_TAB, 0);
//				return TRUE;
			}
			break;
		case WM_DROPFILES:
		{
			HDROP hDrop;
			char szFilename[1024] = {0};
			hDrop = (HDROP)wParam;
			DragQueryFile(hDrop, 0, szFilename, 1024);
			DragFinish(hDrop);
	
			//if(IDNO==MessageBox(NULL, "Open a dropped file?", szFilename, MB_YESNO|MB_ICONQUESTION)) break;
			OpenTextTexture(szFilename);
			//CallWindowProc((WNDPROC)GetWindowLong(g_hDlg, GWL_WNDPROC), g_hDlg, WM_COMMAND, ID_DO_EFFECT, 0);
			return TRUE;
		}
		case WM_NOTIFY:
		{
			break;
		}
		case WM_SIZE:
		{
			int nw = LOWORD(lParam);
			int nh = HIWORD(lParam);

			int ex = 10;
			int ey = 10;
			float edit_ratio = 0.75f;
			MoveWindow(GetDlgItem(hWnd, IDC_EDIT1), ex, ey, nw-(ex+ey), (int)(nh*edit_ratio), FALSE);

			float dbutton_pos_ratiox = 0.65f;
			float dbutton_pos_ratioy = 0.88f;
			float cbutton_pos_ratiox = 0.83f;
			float cbutton_pos_ratioy = 0.88f;

			SetWindowPos(GetDlgItem(hWnd, ID_DO_EFFECT), 0, (int)(nw*dbutton_pos_ratiox), (int)(nh*dbutton_pos_ratioy), 0, 0, SWP_NOSIZE|SWP_NOZORDER);
			SetWindowPos(GetDlgItem(hWnd, IDCANCEL),    0, (int)(nw*cbutton_pos_ratiox), (int)(nh*cbutton_pos_ratioy), 0, 0, SWP_NOSIZE|SWP_NOZORDER);

			float sug_pos_ratiox = 0.1f;
			float sug_pos_ratioy = 0.80f;
			SetWindowPos(GetDlgItem(hWnd, IDC_SUGGEST), 0, (int)(nw*sug_pos_ratiox), (int)(nh*sug_pos_ratioy), 0, 0, SWP_NOSIZE|SWP_NOZORDER);

			float static_pos_ratiox = 0.2f;
			float static_pos_ratioy = 0.85f;
			SetWindowPos(GetDlgItem(hWnd, IDC_BYTES), 0, (int)(nw*static_pos_ratiox), (int)(nh*static_pos_ratioy), 0, 0, SWP_NOSIZE|SWP_NOZORDER);

			//size box
			float inputw_pos_ratiox = 0.2f;
			float inputw_pos_ratioy = 0.92f;
			SetWindowPos(GetDlgItem(hWnd, IDC_SIZE_WIDTH), 0, (int)(nw*inputw_pos_ratiox), (int)(nh*inputw_pos_ratioy), 0, 0, SWP_NOSIZE|SWP_NOZORDER);

			float inputstatic_pos_ratiox = 0.3f;
			float inputstatic_pos_ratioy = 0.92f;
			SetWindowPos(GetDlgItem(hWnd, IDC_SIZE_STATIC), 0, (int)(nw*inputstatic_pos_ratiox), (int)(nh*inputstatic_pos_ratioy), 0, 0, SWP_NOSIZE|SWP_NOZORDER);

			float inputh_pos_ratiox = 0.33f;
			float inputh_pos_ratioy = 0.92f;
			SetWindowPos(GetDlgItem(hWnd, IDC_SIZE_HEIGHT), 0, (int)(nw*inputh_pos_ratiox), (int)(nh*inputh_pos_ratioy), 0, 0, SWP_NOSIZE|SWP_NOZORDER);

			float inputset_pos_ratiox = 0.42f;
			float inputset_pos_ratioy = 0.92f;
			SetWindowPos(GetDlgItem(hWnd, IDC_SIZE_OK), 0, (int)(nw*inputset_pos_ratiox), (int)(nh*inputset_pos_ratioy), 0, 0, SWP_NOSIZE|SWP_NOZORDER);

			InvalidateRect(hWnd, NULL, TRUE);
			return TRUE;
		}
		case WM_DESTROY:
			DestroyWindow(GetDlgItem(hWnd, IDC_EDIT1));
			g_hDlg = NULL;
			break;
	}
	return FALSE;
}

int GetOpenFileNameSingle(HWND hOwnerWnd, const char* szExtention, char* szRecv, bool isAllowAllType)
{
	lstrcpy(szRecv, "");

	TCHAR szFilter[512];
	TCHAR szToken[2][64];
	
	wsprintf(szToken[0], "Supported files(*.%s)", szExtention);
	int i;
	for(i=0; i<lstrlen(szToken[0])+1; i++)
		szFilter[i] = szToken[0][i];
	//szFilter[i] = '\0';

	wsprintf(szToken[1], "*.%s", szExtention);
	int previous = lstrlen(szToken[0])+1;
	//int previous = lstrlen(szToken[0])+1+1;
	int dest = lstrlen(szToken[1])+1;
	int j;
	for(j=0; j<dest; j++)
		szFilter[j+previous] = szToken[1][j];

	if(isAllowAllType == false)	szFilter[j+previous] = '\0';
	else
	{
		//"All Files(*.*)"
		int k;
		previous += lstrlen(szToken[1])+1;
		//previous = lstrlen(szFilter)+1+1;
		char szSrc[64];
		lstrcpy(szSrc, "All Files(*.*)");
		for(k=0; k<lstrlen(szSrc)+1; k++)
			szFilter[k+previous] = szSrc[k];

		previous += lstrlen(szSrc) + 1;
		lstrcpy(szSrc, "*.*");
		for(k=0; k<lstrlen(szSrc)+1; k++)
			szFilter[k+previous] = szSrc[k];
		szFilter[k+previous] = '\0';
	}
	
	TCHAR szTitle[512];
	wsprintf(szTitle, "Opening .%s file", szExtention);

	TCHAR szCurrentDirectory[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurrentDirectory);

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hOwnerWnd;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = szRecv;
	ofn.nMaxFile = 1024;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = szExtention;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrFileTitle = NULL; //ファイル名
	ofn.lpstrTitle = szTitle;
//	ofn.lpstrInitialDir = szCurrentDirectory;
	if(GetOpenFileName(&ofn)) return TRUE;
	return FALSE;
}

int GetSaveFileNameSingle(HWND hOwnerWnd, const char* szExtention, char* szRecv, bool isAllowAllType)
{
	lstrcpy(szRecv, "");
	TCHAR szFilter[512];
	TCHAR szToken[2][64];
	
	wsprintf(szToken[0], "Supported files(*.%s)", szExtention);
	int i;
	for(i=0; i<lstrlen(szToken[0])+1; i++)
		szFilter[i] = szToken[0][i];
	//szFilter[i] = '\0';

	wsprintf(szToken[1], "*.%s", szExtention);
	int previous = lstrlen(szToken[0])+1;
	//int previous = lstrlen(szToken[0])+1+1;
	int dest = lstrlen(szToken[1])+1;
	int j;
	for(j=0; j<dest; j++)
		szFilter[j+previous] = szToken[1][j];

	if(isAllowAllType == false)	szFilter[j+previous] = '\0';
	else
	{
		//"All Files(*.*)"
		int k;
		previous += lstrlen(szToken[1])+1;
		//previous = lstrlen(szFilter)+1+1;
		char szSrc[64];
		lstrcpy(szSrc, "All Files(*.*)");
		for(k=0; k<lstrlen(szSrc)+1; k++)
			szFilter[k+previous] = szSrc[k];

		previous += lstrlen(szSrc) + 1;
		lstrcpy(szSrc, "*.*");
		for(k=0; k<lstrlen(szSrc)+1; k++)
			szFilter[k+previous] = szSrc[k];
		szFilter[k+previous] = '\0';
	}

	TCHAR szTitle[512];
	wsprintf(szTitle, "Opening .%s file", szExtention);

	TCHAR szCurrentDirectory[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szCurrentDirectory);

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hOwnerWnd;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szRecv;
	ofn.nMaxFile = 1024;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = szExtention;
//	ofn.lpstrInitialDir = szCurrentDirectory;
	if(GetSaveFileName((LPOPENFILENAME)&ofn)) return TRUE;
	return FALSE;
}

bool IsEof(FILE* fp)
{
	long fpos = ftell(fp);
	fgetc(fp);
	if(feof(fp)==0)
	{
		fseek(fp, fpos, SEEK_SET);
		return false;
	}
	return true;
}

void OpenTextTexture(const char* szFile)
{
	lstrcpy(g_szLastFileName, szFile);
	ZeroMemory(g_szTextureString, TEXT_BUFFER_SIZE);
	ZeroMemory(g_szFinalTexture, TEXT_BUFFER_SIZE);
	FILE* fp = fopen(szFile, "rb");
	//for(int i=0; IsEof(fp); i++)	
	//{
	//	g_szTextureString[i] = fgetc(fp);		
	//}
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(g_szTextureString, size, 1, fp);
	fclose(fp);

	//旧KTGで保存された, 不正なテキストファイルを修正する処理
	//char* tmp = (char*)GlobalAlloc(GPTR, sizeof(char)*TEXT_BUFFER_SIZE);
	//CopyMemory(tmp, g_szTextureString, TEXT_BUFFER_SIZE);
	//lstrcpy(tmp, g_szTextureString);
	//int diff = 0;
	for(int i=0; i<TEXT_BUFFER_SIZE; i++){//have to use lstrlen()!
		//if(g_szTextureString[i]=='\r' && g_szTextureString[i+1]=='\r'){
		if(g_szTextureString[i]=='\r' && g_szTextureString[i+1]!='\n'){
			//int diff=0;
			//diff = 10;
			//for(int j=i-diff;j<TEXT_BUFFER_SIZE-i-diff;j++){
			for(int j=i;j<TEXT_BUFFER_SIZE-i;j++){
				g_szTextureString[j]=g_szTextureString[j+1];
			}
			i--;
		}
	}
	//CopyMemory(g_szTextureString, tmp, TEXT_BUFFER_SIZE);
	//lstrcpy(g_szTextureString, tmp);
	//GlobalFree(tmp);

	//HANDLE hFile = CreateFile(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING,	FILE_ATTRIBUTE_NORMAL, NULL);
	//if(!hFile){
	//	win.CMessageBox("ファイルのオープンに失敗");
	//	return;
	//}
	//DWORD dwDumm;
	//ReadFile(hFile, g_szTextureString, GetFileSize(hFile, NULL), &dwDumm, NULL);
	//CloseHandle(hFile);
	SetDlgItemText(g_hDlg, IDC_EDIT1, g_szTextureString);
	//CallWindowProc((WNDPROC)GetWindowLong(g_hDlg, GWL_WNDPROC), g_hDlg, WM_COMMAND, ID_DO_EFFECT, 0);
}

void SaveTextTexture()
{
	ZeroMemory(g_szTextureString, TEXT_BUFFER_SIZE*sizeof(char));
	GetDlgItemText(g_hDlg, IDC_EDIT1, g_szTextureString, TEXT_BUFFER_SIZE*sizeof(char));
	if(g_szTextureString[0] == '\0') return;
	if(g_szLastFileName[0]=='\0'){
		if(!GetSaveFileNameSingle(NULL, "txt", g_szLastFileName, TRUE)) return;
		//lstrcpy(g_szLastFileName, szFile);
	}
	FILE* fp = fopen(g_szLastFileName, "wb");
	if(!fp){
		win.CMessageBox("ファイルのオープンに失敗");
		return;
	}
	fwrite(g_szTextureString, lstrlen(g_szTextureString)+1, 1, fp);
	fclose(fp);
}

void ExtractKTFToText(const char* texture, char* szDestination, float versioninfo){
	int i;
	if(versioninfo<=KTF_VERSION_OBSOLEET){
		for(i=0; i<(lstrlen(texture)+1); i++){
			lstrcat(szDestination, parser[texture[i]-'A']);

			
		}
	}
}

void OpenKTF(const char* szFileName){
	FILE* fp = fopen(szFileName, "rb");
	if(!fp) return;

	ZeroMemory(g_szLastFileName, MAX_PATH); //KTFの場合は上書き保存を認めない(読み込んだ時点で書式がKTFじゃなくなるしね).
	ZeroMemory(g_szTextureString, TEXT_BUFFER_SIZE);
	ZeroMemory(g_szFinalTexture, TEXT_BUFFER_SIZE);
	int i;

	char* header = (char*)GlobalAlloc(GPTR, sizeof(char) * (lstrlen(KTF_SZHEADER)+1));
		for(i=0; i<lstrlen(KTF_SZHEADER); i++) header[i] = fgetc(fp);
		if(lstrcmp(header, KTF_SZHEADER)==0){
			char version_identifier = fgetc(fp);
			char* texture_string = (char*)GlobalAlloc(GPTR, sizeof(char) * TEXT_BUFFER_SIZE);
			if(version_identifier!=KTF_VERSIONIDENTIFIER){ //KTFバージョン<=1
				//fseek(fp, lstrlen(KTF_SZHEADER), SEEK_SET);
				//fseek(fp, 0, SEEK_END);
				//long fsize = ftell(fp);
				fseek(fp, lstrlen(KTF_SZHEADER), SEEK_SET);
                
				for(i=0; !IsEof(fp); i++){
					texture_string[i] = fgetc(fp);
				}
				ExtractKTFToText(texture_string, g_szTextureString, KTF_VERSION_OBSOLEET);
			}
			GlobalFree(texture_string);
		}
		SetDlgItemText(g_hDlg, IDC_EDIT1, g_szTextureString);
		fclose(fp);
	GlobalFree(header);
}

void LoadExternalTexture(const char* filename){
	IPicture*    iPict = NULL;//IPictureインターフェース
	IStream*     iStream = NULL;
	FILE* fp;

	if((fp = fopen(filename,"rb"))==NULL){
		return;
	}

	fseek(fp,0,SEEK_END);
	long fsize = ftell(fp);
	fseek(fp,0,SEEK_SET);
	unsigned char* fbuf = (unsigned char*)GlobalAlloc(GPTR,fsize);//ファイルサイズを調べる
	fread(fbuf,fsize,1,fp);//ファイルをバッファに読み込む
	
	if( SUCCEEDED(CreateStreamOnHGlobal( (HGLOBAL)fbuf, FALSE, &iStream ))){
		if( FAILED(OleLoadPicture( iStream, fsize, TRUE, IID_IPicture, (void**)&iPict))){
			GlobalFree(fbuf);
			return;
		}
		
		if(iStream != NULL ){
			iStream->Release();
			iStream = NULL ;
		}
		GlobalFree(fbuf);

		long lx,ly;
		iPict->get_Width( &lx ) ;
		iPict->get_Height( &ly ) ;

		// 画像の大きさを取得.
		HDC hDDC = ::GetDC(NULL);
			int nw = MulDiv( lx, GetDeviceCaps( hDDC, LOGPIXELSX), 2540 ) ;
			int nh = MulDiv( ly, GetDeviceCaps( hDDC, LOGPIXELSY), 2540 ) ;
		ReleaseDC(NULL, hDDC);

		//ビットマップか調べる
		char bm[2];
		fseek(fp, 0, SEEK_SET);
		fread(&bm, 2*sizeof(char), 1, fp);
		fseek(fp, 0, SEEK_SET);

		bool is32Bit = false; //32BitならiPict経由で読まない
		BITMAPFILEHEADER bfh;
		BITMAPINFOHEADER bih;
		UCHAR* tex_pixel;


		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, under_tex);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


		if(bm[0]=='B' && bm[1]=='M'){ //ビットマップだった
			//ヘッダを読み込む
			fread(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
			fread(&bih, sizeof(BITMAPINFOHEADER), 1, fp);

			long imagewidth = (nw * 4 * sizeof(UCHAR)) & ~0x03;
			long imagesize = imagewidth * nh;

			if(bih.biBitCount==32){
				is32Bit = true;
				//一時バッファを確保
				UCHAR* pTempPixels = (UCHAR*)GlobalAlloc(GPTR, imagesize);
				tex_pixel = (UCHAR*)GlobalAlloc(GPTR, nw * nh * 4);
				fread(pTempPixels, imagesize, 1, fp);

				//メインバッファに書き込み
				long offset_base = 4 - nw*4/4;
				long offset = 0;
				for(int h = 0; h<nh; h++){
					for(int w = 0; w<nw; w++){
						int d = (w*4 + (((nh)-1-h)*nw*4));
						int g = (w*4 + h*nw*4);
						d+=offset;
						tex_pixel[g+0] = pTempPixels[d+2];
						tex_pixel[g+1] = pTempPixels[d+1];
						tex_pixel[g+2] = pTempPixels[d+0];
						tex_pixel[g+3] = pTempPixels[d+3];
					}
					offset += offset_base;
				}
				GlobalFree(pTempPixels);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nw, nh, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_pixel);
				GlobalFree(tex_pixel);
			}
		}

		if(!is32Bit){
			//24ビットビットマップ領域の作成
			HDC hDC;
			HBITMAP hBitmap, hOldBitmap;
			unsigned char* pTempPixels;

			long imagewidth = (nw * 3 * sizeof(UCHAR)) & ~0x03;//補正された横幅
			long imagesize = imagewidth * nh; //横幅が２の階乗じゃないと読めない

			BITMAPINFO	BmpInfo;
			ZeroMemory(&BmpInfo, sizeof(BITMAPINFO));
			BmpInfo.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
			BmpInfo.bmiHeader.biBitCount	= 24;
			BmpInfo.bmiHeader.biPlanes		= 1;
			BmpInfo.bmiHeader.biWidth		= nw;
			BmpInfo.bmiHeader.biHeight		= nh;
			BmpInfo.bmiHeader.biSizeImage	= imagesize;
			BmpInfo.bmiHeader.biCompression = BI_RGB;
			hBitmap = CreateDIBSection(NULL, &BmpInfo, DIB_RGB_COLORS, (void**)&pTempPixels, NULL, 0);
			HDC hDeskDC = GetDC(HWND_DESKTOP);
				hDC = CreateCompatibleDC(hDeskDC);
			ReleaseDC(HWND_DESKTOP, hDeskDC);
			hOldBitmap = (HBITMAP)SelectObject(hDC, (HGDIOBJ)hBitmap);
				iPict->Render(hDC ,0,0, nw, nh, 0,ly,lx,-ly,NULL);

				tex_pixel = (UCHAR*)GlobalAlloc(GPTR, nw * nh * 4);
				//メインバッファに書き込み
				long offset_base = nw * 3 - imagewidth;
				//long offset = 0;
				for(int h = 0; h<nh; h++){
					for(int w = 0; w<nw; w++){
						int d = (w*3 + (nh-1-h)*nw*3);
						int g = (w*4 + h*nw*4);
						//d += offset_base;
						tex_pixel[g+0] = pTempPixels[d+2];
						tex_pixel[g+1] = pTempPixels[d+1];
						tex_pixel[g+2] = pTempPixels[d+0];
						tex_pixel[g+3] = 255;
					}
					//offset += offset_base;
				}
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nw, nh, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_pixel);
				GlobalFree(tex_pixel);
				
			SelectObject(hDC, hOldBitmap); //DIBSectionモードを抜ける
			DeleteObject(hBitmap);				//CreateDIBSectionで確保されたメモリを解放させる
			DeleteDC(hDC);					//メモリデバイスコンテキストの破棄
		}
		iPict->Release();
		fclose(fp);
	}
}

LRESULT CALLBACK HWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_COMMAND:
		{
			HWND hEdit = GetDlgItem(g_hDlg, IDC_EDIT1);
			switch(LOWORD(wParam))
			{
			case ID_FILE_NEW_TEXTURE:
				if(MessageBox(hWnd, "\"New\" this operation will clear all buffers in current application.\nContinue?", "Question", MB_SYSTEMMODAL|MB_ICONQUESTION|MB_YESNO)==IDNO) break;

				lstrcpy(g_szTextureString, "");
				lstrcpy(g_szLastFileName, "");
				lstrcpy(g_szFinalTexture, "");
				SetDlgItemText(g_hDlg, IDC_EDIT1, g_szTextureString);
				SendMessage(hEdit, EM_EMPTYUNDOBUFFER, 0, 0);
			break;
			case ID_FILE_OPEN_TEXTURE:
			{
				char szFile[MAX_PATH];
				if(GetOpenFileNameSingle(NULL, "txt", szFile, TRUE))
				{
					OpenTextTexture(szFile);
					SendMessage(GetDlgItem(g_hDlg, IDC_EDIT1), EM_EMPTYUNDOBUFFER, 0, 0);
					win.CSetWindowText(g_hDlg, "Text file was Loaded from %s", szFile);
				}
				break;
			}
			case ID_OPEN_PICTURE:
			{
				glGenTextures(1, &under_tex);
				char szFile[MAX_PATH];
				if(GetOpenFileNameSingle(NULL, "bmp", szFile, TRUE))
				{
					LoadExternalTexture(szFile);
					win.CSetWindowText(g_hDlg, "External picture was Loaded from %s", szFile);
				}
				Render();
				break;
			}
			case ID_FILE_SAVE_OW:
			{
				SaveTextTexture();
				if(g_szLastFileName[0] != '\0') win.CSetWindowText(g_hDlg, "Text file was Over-Written to %s", g_szLastFileName);
				break;
			}
			case ID_FILE_SAVEAS:
			{
				ZeroMemory(g_szLastFileName, MAX_PATH);
				SaveTextTexture();
				if(g_szLastFileName[0] != '\0') win.CSetWindowText(g_hDlg, "Text file was Saved to %s", g_szLastFileName);
				break;
			}
			case ID_FILE_EXPORT:
			{
				if(g_szFinalTexture[0] == '\0'){
					win.CSetWindowText(g_hDlg, "Do effect before exporting!");
					return 0;
				}
				char szFile[MAX_PATH];
				if(GetSaveFileNameSingle(NULL, "ktf", szFile, FALSE))
				{
					//win.CMessageBox(g_szFinalTexture);
					PathAddExtension(szFile, ".ktf");
					FILE* fp = fopen(szFile, "wt");
						fprintf(fp, "KTF%s", g_szFinalTexture);
					fclose(fp);
					win.CSetWindowText(g_hDlg, "KTF was Exported to %s", szFile);
				}
					//ktex.GenerateTexture(,"");
				break;
			}
			case ID_EXPORT_24BMP:
			{
				char szBmpFileName[MAX_PATH];
				if(!GetSaveFileNameSingle(NULL, "bmp", szBmpFileName, TRUE)) break;
				
				SaveBMP(szBmpFileName, ktex.GetWidth(), ktex.GetHeight(), 24);
				win.CSetWindowText(g_hDlg, "BMP(24bit) was Exported to %s", szBmpFileName);
				break;
			}
			case ID_EXPORT_32BMP:
			{
				char szBmpFileName[MAX_PATH];
				if(!GetSaveFileNameSingle(NULL, "bmp", szBmpFileName, TRUE)) break;
				
				SaveBMP(szBmpFileName, ktex.GetWidth(), ktex.GetHeight(), 32);
				win.CSetWindowText(g_hDlg, "BMP(32bit) was Exported to %s", szBmpFileName);
				break;
			}
			case ID_EXPORT_DKIMAGE:
			{
				char szBmpFileName[MAX_PATH];
				if(!GetSaveFileNameSingle(NULL, "bmp", szBmpFileName, TRUE)) break;

				SaveBMP(szBmpFileName, ktex.GetWidth(), ktex.GetHeight(), 24, 0,255,0);
				win.CSetWindowText(g_hDlg, "BMP(24bit-dkImage Compatible) was Exported to %s", szBmpFileName);
				break;
			}
			case ID_FILE_EXIT:
				VerifyOnExit();
			break;
			//-------------Edit-----------
			case ID_EDIT_INPUT:
				SetupDialog();
			break;
			case ID_EDIT_COPYTOCLIPBOARD:
				SendToClipboard(g_szTextureString);
			break;
			//-------------View-----------
			case ID_WINDOWSIZE_X1:
			case ID_WINDOWSIZE_X2:
			case ID_WINDOWSIZE_X3:
			case ID_WINDOWSIZE_X4:
			{
				lastwindowsize = LOWORD(wParam);

				int multiply = LOWORD(wParam) - ID_WINDOWSIZE_X1 + 1;
				int width = ktex.GetWidth();
				int height = ktex.GetHeight();
				LONG style = GetWindowLong(hMainWnd, GWL_STYLE);
				LONG exstyle = GetWindowLong(hMainWnd, GWL_EXSTYLE);
				RECT WindowRect;
				SetRect(&WindowRect, 0, 0, width*multiply, height*multiply+GetSystemMetrics(SM_CYMENU));
			    AdjustWindowRectEx(&WindowRect, style, FALSE, exstyle);
				SetWindowPos(hWnd, 0, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, SWP_NOZORDER|SWP_NOMOVE);

				////RECT WindowRect;
    ////            AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

				////SetWindowPos(hWnd, 0, 0, 0, width*multiply, height*multiply+GetSystemMetrics(SM_CYMENU), SWP_NOZORDER| SWP_NOMOVE);
				break;
			}
			case ID_UNDERTEX_100:
			case ID_UNDERTEX_075:
			case ID_UNDERTEX_050:
			case ID_UNDERTEX_025:
			case ID_UNDERTEX_000:
			{
				int multiply = LOWORD(wParam) - ID_UNDERTEX_000;
				under_tex_alpha = multiply * 0.25f;
				Render();
				break;
			}
			case ID_MASTERTEX_100:
			case ID_MASTERTEX_075:
			case ID_MASTERTEX_050:
			case ID_MASTERTEX_025:
			case ID_MASTERTEX_000:
			{
				int multiply = LOWORD(wParam) - ID_MASTERTEX_000;
				mastertex_alpha = multiply * 0.25f;
				Render();
				break;
			}
			case ID_ALPHATEX_100:
			case ID_ALPHATEX_075:
			case ID_ALPHATEX_050:
			case ID_ALPHATEX_025:
			case ID_ALPHATEX_000:
			{
				int multiply = LOWORD(wParam) - ID_ALPHATEX_000;
				alphatex_alpha = multiply * 0.25f;
				Render();
				break;
			}
			case ID_VIEW_SIZE_128:
				//GetMenuBarInfo(
				//SetWindowPos(hWnd, 0, 0, 0, 128, 128+GetSystemMetrics(SM_CYMENU), SWP_NOZORDER| SWP_NOMOVE);
			break;
			case ID_VIEW_FONT:
				ChooseAndSetEditFont(GetDlgItem(g_hDlg, IDC_EDIT1));
			break;
			//-------------Help-----------
			case ID_HELP_ARC: //ヘルプファイルを読み出す
			{
				char* szCmd = (char*)GlobalAlloc(GPTR, sizeof(char) * MAX_PATH);
				::GetModuleFileName(GetModuleHandle(NULL), szCmd, MAX_PATH);
				::PathRemoveFileSpec(szCmd);
				char* ptr;
				char szExeption[][64] = {"DEBUG", "RELEASE"};
				for(int i=0; i<2; i++)
				{
					if(ptr = StrStrI(szCmd, szExeption[i])){ //DEBUGフォルダの中にあったら
						for(int j=0; j<(lstrlen(szCmd)+1); j++)
						{
							if(&szCmd[j] == ptr)
							{
								szCmd[j] = '\0';
								lstrcat(szCmd, "help.txt");
								break;
							}
						}
						ShellExecute(hMainWnd, "open", szCmd, NULL, NULL, SW_SHOWNORMAL);
						return TRUE;
					}
				}
				//例外処理に引っかからなかったら普通に呼び出す
				lstrcat(szCmd, "\\help.txt");
				ShellExecute(hMainWnd, "open", szCmd, NULL, NULL, SW_SHOWNORMAL);
				return TRUE;
			}
			//case ID_HELP_ANIMATION:
			//	isAnimating = !isAnimating;
			//break;
			default:
			break;
			}
		break;
		}
		case WM_DESTROY:
		case WM_CLOSE:
			VerifyOnExit();
			return FALSE;
		case WM_SIZE:
		{
			return TRUE;
		}
	}
	return TRUE;
}

void VerifyOnExit() //終了確認
{
	if(MessageBox(hMainWnd, "Exit?", "Exit", MB_YESNO|MB_ICONQUESTION|MB_SYSTEMMODAL)==IDYES) PostQuitMessage(0);
}

//モードレスダイアログを使用するため.CWindowのループを呼ばないのでOnIdleが使用できなくなる弊害あり.
void MessageLoop()
{
	MSG msg;
    while(GetMessage(&msg, NULL, 0, 0))
    {
		if(TranslateAccelerator(g_hDlg, hAccel, &msg)){
			continue;
		}else if(IsDialogMessage(g_hDlg, &msg)){
			continue;
		}else{
			if(!TranslateAccelerator(hMainWnd, hMainAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
    }
}

DWORD CALLBACK DataSendProc(DWORD dwCookie, LPBYTE pbBuf, LONG cb, LONG *pcb)
{
	static int progress = 0;
	char* szText = (char*)dwCookie;
	int tlen = lstrlen(szText);
	int transfer = 0;
	if(progress<tlen){
		transfer = (cb>tlen) ? tlen : cb;
		CopyMemory(pbBuf, szText+progress, transfer);
		*pcb = transfer;
		progress += transfer;
	}else{
		*pcb = 0;
		progress = 0;
	}
	//ReadFile((HANDLE)dwCookie, pbBuf, cb, (LPDWORD)pcb, NULL);
	return FALSE;
}

void AppendText(const char* szText){
	EDITSTREAM eds;
	eds.dwCookie = (DWORD_PTR)szText;
	eds.dwError = 0;
	eds.pfnCallback = DataSendProc;
	//SendMessage(hMyEdit, EM_STREAMIN, SF_RTF |SFF_SELECTION, (LPARAM)&eds);
	HWND hEdit = GetDlgItem(g_hDlg, IDC_EDIT1);
	SendMessage(hEdit, EM_STREAMIN, SF_TEXT | SFF_SELECTION, (LPARAM)&eds);
}

void RemoveLastCharFromString(char* szText){
	szText[lstrlen(szText)-1] = '\0';
}

void CreateCoordinateString(char* szRecv){
	char str[64];
	int i;

	if(currentpoint<0) return;
	
	//
	//x,y座標を0-255にキャストする
	//
	RECT rect;
	GetClientRect(hMainWnd, &rect);
	float multiplyx = 256.0f/(float)(rect.right-rect.left);
	float multiplyy = 256.0f/(float)(rect.bottom-rect.top);
	for(i=0; i<=currentpoint; i++){
		mousept[i].x = (long)(mousept[i].x * multiplyx);
		mousept[i].y = (long)(mousept[i].y * multiplyy);
	}

	switch(current_function){
        case 0://circle
		{
			wsprintf(szRecv, "(");
//			for(i=0; i<2; i++){
				wsprintf(str, "%d,%d, ", mousept[0].x, mousept[0].y);
				lstrcat(szRecv, str);
				//wsprintf(str, "%d", mousept[1].x - mousept[0].x);
				//distance = route((px - centerx)^2 + (py - centery)^2)
				long d = 0;
				float a = sqr((float)((mousept[1].x - mousept[0].x)*(mousept[1].x - mousept[0].x) + (mousept[1].y - mousept[0].y)*(mousept[1].y - mousept[0].y)));
				d = (long)a;
				wsprintf(str, "%d", d);//距離を求めるコードに置き換える
				lstrcat(szRecv, str);
//			}
			lstrcat(szRecv, ");");
		}break;
		case 1://elli
		{
			wsprintf(szRecv, "(");
				wsprintf(str, "%d,%d, ", mousept[0].x, mousept[0].y);
				lstrcat(szRecv, str);
				wsprintf(str, "%d,%d, ", mousept[1].x, mousept[1].y);
				lstrcat(szRecv, str);
			RemoveLastCharFromString(szRecv);
			RemoveLastCharFromString(szRecv);
			lstrcat(szRecv, ");");		
		}break;
		case 21:{//roundrect
			wsprintf(szRecv, "(");
				wsprintf(str, "%d,%d, ", mousept[0].x, mousept[0].y);
				lstrcat(szRecv, str);
				wsprintf(str, "%d,%d, ", mousept[1].x - mousept[0].x, mousept[1].y - mousept[0].y);
				lstrcat(szRecv, str);
				wsprintf(str, "%d,%d, ", 10, 10);//round_r
				lstrcat(szRecv, str);
			RemoveLastCharFromString(szRecv);
			RemoveLastCharFromString(szRecv);
			lstrcat(szRecv, ");");
		}break;
		case 2://rectangle
		case 27:{//rect
			wsprintf(szRecv, "(");
//			for(i=0; i<2; i++){
				wsprintf(str, "%d,%d, ", mousept[0].x, mousept[0].y);
				lstrcat(szRecv, str);
				wsprintf(str, "%d,%d, ", mousept[1].x - mousept[0].x, mousept[1].y - mousept[0].y);
				lstrcat(szRecv, str);
//			}
			RemoveLastCharFromString(szRecv);
			RemoveLastCharFromString(szRecv);
			lstrcat(szRecv, ");");
		}break;
        case 29:{//polygon
			wsprintf(szRecv, "(%d, ", currentpoint+1);
			for(i=0; i<=currentpoint; i++){
				wsprintf(str, "%d,%d, ", mousept[i].x, mousept[i].y);
				lstrcat(szRecv, str);
			}
			RemoveLastCharFromString(szRecv);
			RemoveLastCharFromString(szRecv);
			lstrcat(szRecv, ");");
		}break;
        case 37:{//polyline
			wsprintf(szRecv, "(%d, ", currentpoint+1);
			for(i=0; i<=currentpoint; i++){
				wsprintf(str, "%d,%d, ", mousept[i].x, mousept[i].y);
				lstrcat(szRecv, str);
			}
			RemoveLastCharFromString(szRecv);
			RemoveLastCharFromString(szRecv);
			lstrcat(szRecv, ");");
		}break;
	}
}

void KeyEvent(UCHAR key, bool isDown)
{
	if(isDown){
		switch(key){
			case VK_RETURN:{
				char string[4096] = {'\0'};
				CreateCoordinateString(string);
				AppendText(string);
				
				ZeroMemory(mousept, sizeof(POINT)*512);
				currentpoint = -1;
				SetFocus(GetDlgItem(g_hDlg, IDC_EDIT1));
			}break;
 		}
	}
}

void MouseEvent(long x, long y, int btn, bool isDown)
{
	win.CSetWindowText(hMainWnd, "PtCnt:%d, x:%d,y:%d", currentpoint, x, y);
	//win.CSetWindowText(hMainWnd, "mouse: %d, %d", x, y);
	
	//選択決定処理
	if(
		(GetAsyncKeyState(VK_LBUTTON)&0x8000)==0x8000
		&&
		(GetAsyncKeyState(VK_RBUTTON)&0x8000)==0x8000
	   )
	{
		char string[4096] = {'\0'};
		CreateCoordinateString(string);
		AppendText(string);

		ZeroMemory(mousept, sizeof(POINT)*512);
		currentpoint = -1;
		SetFocus(GetDlgItem(g_hDlg, IDC_EDIT1));
	}else{
		//選択処理
		if(btn==LBUTTON){
			if(isDown){
				//isCapturing = true;

				if(++currentpoint>=512){
					currentpoint = 512;
				}
				mousept[currentpoint].x = x;
				mousept[currentpoint].y = y;

				//char str[64];
				//wsprintf(str, "%d,", x);
				//AppendText(str);
				//HWND hEdit = GetDlgItem(g_hDlg, IDC_EDIT1);
				//SetFocus(hEdit);
			}else{
				//isCapturing = false;
				//currentpoint = -1;
			}
		}else if(btn==RBUTTON){
			//*((int*)0) = 100;
			if(isDown){
				mousept[currentpoint].x = mousept[currentpoint].y = 0;
				if(--currentpoint<=-1){
					currentpoint=-1;
				}
				//char str[64];
				//wsprintf(str, "%d,", y);
				//AppendText(str);
				//HWND hEdit = GetDlgItem(g_hDlg, IDC_EDIT1);
				//SetFocus(hEdit);
			}
		}else if(btn==MBUTTON){
			if(isDown){
				
			}
		}else if(btn==WHEEL){
			if(isDown){
				
			}
		}else if(btn==MOUSEMOVE){
	//		if(isCapturing){
			Render();
			//win.RedrawScreen();
	//		}
		}
	}
}

void RebootMe(){
	int nArgc;
	int i;
	WCHAR** wszCmdLine = CommandLineToArgvW(GetCommandLineW(), &nArgc);
	int length;
	char** szArgv = (char**)GlobalAlloc(GPTR, sizeof(char*) * nArgc);
	for(i=0; i<nArgc; i++)
	{
		if((length = ::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, NULL, 0, NULL, NULL))==0) continue;
		szArgv[i] = (char*)GlobalAlloc(GPTR, sizeof(char) * (length+1));
		::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, szArgv[i], length, NULL, NULL);
	}

	WinExec(szArgv[0], SW_SHOWNORMAL);

	for(i=0; i<nArgc; i++){
		GlobalFree(szArgv[i]);
	}
	GlobalFree(szArgv);
}

int cmain()
{
#ifndef _DEBUG
	__try
#endif
	{
		//+GetSystemMetrics(SM_CYMENU)
		win.CCreateWindow(100,100,256, 256, "");
		win.CSetWindowText(hMainWnd, "%s %4.2f    (BUILD DATE:%s %s)", APP_TTL, APP_VER, __DATE__, __TIME__);
		win.CSetCallbackFunctions(KeyEvent, MouseEvent, OnDraw, NULL);
		win.CSetHookProcedure(HWndProc);
		HANDLE hIcon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
		SetClassLong(hMainWnd, GCL_HICON, (LONG)hIcon);

		LONG style = GetWindowLong(hMainWnd, GWL_STYLE);
		LONG exstyle = GetWindowLong(hMainWnd, GWL_EXSTYLE);
		style = style&~WS_THICKFRAME;
		RECT WindowRect;
		SetRect(&WindowRect, 0, 0, 256, 256+GetSystemMetrics(SM_CYMENU));
		AdjustWindowRectEx(&WindowRect, style, FALSE, exstyle);
		SetWindowLong(hMainWnd, GWL_STYLE, style);
		SetWindowPos(hMainWnd, 0, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, SWP_NOZORDER|SWP_NOMOVE);
		
		InitGUI(); mousept = (POINT*)GlobalAlloc(GPTR, sizeof(POINT) * 512);
		InitOpenGL();
		InitConverter();

		int nArgc;
		int i;
		WCHAR** wszCmdLine = CommandLineToArgvW(GetCommandLineW(), &nArgc);
		int length;
		char** szArgv = (char**)GlobalAlloc(GPTR, sizeof(char*) * nArgc);
		for(i=0; i<nArgc; i++)
		{
			if((length = ::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, NULL, 0, NULL, NULL))==0) continue;
			szArgv[i] = (char*)GlobalAlloc(GPTR, sizeof(char) * length);
			::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, szArgv[i], length, NULL, NULL);
		}

		if(nArgc>1){
			FILE* fp = fopen(szArgv[1], "rb");
			if(!fp) return FALSE;

			char* header = (char*)GlobalAlloc(GPTR, sizeof(char) * (lstrlen(KTF_SZHEADER)+1));
				for(i=0; i<lstrlen(KTF_SZHEADER); i++) header[i] = fgetc(fp);
			fclose(fp);

				if(lstrcmp(header, KTF_SZHEADER)!=0){ //KTFヘッダが見つからない場合はテキストファイルとして処理
					OpenTextTexture(szArgv[1]);
				}else{
					MessageBox(0,"Sorry, i can't read KTF file.",0,MB_SYSTEMMODAL);
					//OpenKTF(szArgv[1]);
				}
			GlobalFree(header);
		}

		MessageLoop();

		for(i=0; i<nArgc; i++) GlobalFree(szArgv[i]);
		GlobalFree(szArgv);
		DeInitConverter();
		GlobalFree(mousept);
		GlobalFree(g_szTextureString);
		FreeLibrary(g_hRichEditLib);
	}
#ifndef _DEBUG
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		__try
		{
			//MessageBox(NULL, "すみません。例外処理が発生しました。現在の情報をできるだけ吐き出します。", "c.r.v.", MB_SYSTEMMODAL);
			char szPath[256] = {'\0'};
			GetCurrentDirectory(sizeof(szPath), szPath);
			//GetTempPath(sizeof(szPath), szPath);
			PathRemoveBackslash(szPath);
			lstrcat(szPath, "\\dump-ktg.txt");
			FILE* fp=fopen(szPath, "wb");
			if(fp==NULL){
				MessageBox(NULL, "ファイルのオープンに失敗しました。\nこの後すぐ，例外処理を恣意的に発生させます。", "c.r.v.", MB_OK);
				throw;
			}
			char buffer[TEXT_BUFFER_SIZE];
			ZeroMemory(buffer, TEXT_BUFFER_SIZE * sizeof(char));
			GetDlgItemText(g_hDlg, IDC_EDIT1, buffer, TEXT_BUFFER_SIZE * sizeof(char));
			fwrite(buffer, sizeof(char), TEXT_BUFFER_SIZE, fp);
			//mdl.SaveKMD(fp);
			fclose(fp);

			char szMes[1024];
			wsprintf(szMes, "最新の情報を保存している最中に例外は発生しませんでした。\nただし，保存されたデータが破損している可能性がありますので，使用する際は十分に気をつけてください。\nデータは下記の場所に保存されました。\n%s\n\nこのメッセージボックスを閉じると，アプリケーションを再起動します。",szPath);
			MessageBox(NULL, szMes, "c.r.v.", MB_SYSTEMMODAL);
			RebootMe();
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			MessageBox(NULL, "申し訳ございません。最新の情報を保存している際に例外が発生しました。すべての作業データは失われます。", "c.r.v.", MB_SYSTEMMODAL); 
		}
	}
#endif
	return 0;
}

void SaveBMP(const char* filename, int width, int height, int bpp, int r, int g, int b)
{
	//BITMAPFILEHEADER bfh;
	//BITMAPINFOHEADER bih;
	//FILE* fp;
	//if((fp=fopen(filename,"wb"))==NULL){
	//	MessageBox(NULL,"ファイルが開けません","error",MB_OK);
	//	return;
	//}
	//
	////構造体情報	
	//bih.biBitCount = bpp;
	//bih.biClrImportant = 0;
	//bih.biClrUsed = 0;
	//bih.biCompression = 0;
	//bih.biHeight = height;
	//bih.biPlanes = 1;
	//bih.biSize = sizeof(BITMAPINFOHEADER);
	//bih.biSizeImage = height*width*(bpp/8);
	//bih.biWidth = width;
	//bih.biXPelsPerMeter = 0;
	//bih.biYPelsPerMeter = 0;
	//
	//bfh.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	//bfh.bfReserved1 = 0;
	//bfh.bfReserved2 = 0;
	//bfh.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+bih.biSizeImage;
	//bfh.bfType = 'B'+'M'*256;
	//
	//fwrite(&bfh,sizeof(bfh),1,fp);
	//fwrite(&bih,sizeof(bih),1,fp);

	MessageBox(NULL, "Sorry, this version doesn't support output-to-bmp feature.", "", MB_OK);
	////UCHAR* tex = ktex.GetImagePixel();
	////if(tex == NULL) return;
	//UCHAR* tex = (UCHAR*)GlobalAlloc(GPTR, sizeof(UCHAR) * height * width * 32/8);
	//glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, tex);

	//UCHAR* dib = (UCHAR*)GlobalAlloc(GPTR, sizeof(UCHAR) * height * width * bpp/8);
	//	for(int i=0; i<width; i++){
	//		for(int j=0; j<height; j++){
	//			//int d = ((i)*(bpp/8) + (((height)-1-j)*(width)*(bpp/8)));
	//			//int t = ((i)*(bppGL/8) + (j)*(width)*(bppGL/8));
	//			//dib[d+0] = tex[t+2];
	//			//dib[d+1] = tex[t+1];
	//			//dib[d+2] = tex[t+0];
	//			//if(bpp == 24 && r >-1 && tex[t+3]==0)
	//			//{
	//			//	dib[d+0] = 0;
	//			//	dib[d+1] = 255;
	//			//	dib[d+2] = 0;
	//			//}	
	//			//if(bpp == 32){
	//			//	dib[d+3] = tex[t+3];
	//			//}
	//			int t = ((i)*(bpp/8) + (j)*(width)*(bpp/8));
	//			dib[t+0] = tex[t+2];
	//			dib[t+1] = tex[t+1];
	//			dib[t+2] = tex[t+0];
	//			if(bpp == 24 && r >-1 && tex[t+3]==0)
	//			{
	//				dib[t+0] = 0;
	//				dib[t+1] = 255;
	//				dib[t+2] = 0;
	//			}	
	//			if(bpp == 32){
	//				dib[t+3] = tex[t+3];
	//			}
	//		}
	//	}
	//fwrite(dib, bih.biSizeImage,1,fp);
	//GlobalFree(dib);
	//GlobalFree(tex);
	//fclose(fp);
}