#include "stdafx.h"
#include "timelineview.h"

const char* CTimelineView::SZ_CONTROL_CLASS_NAME = "CTIMELINEVIEW_CLASS";
const int   CTimelineView::RAW_LINE = 10;
const int   CTimelineView::COL_LINE = 10;

float CTimelineView::GetSelectRate()
{
	return select_timeline_rate;
}

void CTimelineView::OnControlDraw(HDC hdc)
{
	RECT rt;
	GetClientRect(my_hwnd,&rt);

	SelectBrush(hdc,GetStockBrush(GRAY_BRUSH));
	Rectangle(hdc,0,0,rt.right,rt.bottom);
	SetBkMode(hdc,0);
	rt.left = 40;
	rt.right -= 50;
	SelectPen(hdc,(HPEN)GetStockPen(BLACK_PEN));

	int x,y;
	for(y=0; y<RAW_LINE; y++){//横線
		int iy = (int)(rt.bottom*y/(float)RAW_LINE);
		MoveToEx(hdc,rt.left,iy,NULL);
		LineTo(hdc,rt.left+rt.right,iy);
		if(y>0){
			char clstr[16];
			sprintf(clstr,"%5.2f",(0.5f-y/(float)COL_LINE)*2.0f);
			TextOut(hdc,0,iy-8,clstr,lstrlen(clstr));
		}
	}
	for(x=0; x<=COL_LINE; x++){//縦線
		int ix = (int)(rt.right*x/(float)COL_LINE) + rt.left;
		MoveToEx(hdc,ix,rt.top,NULL);
		LineTo(hdc,ix,rt.bottom);
		char clstr[16];
		sprintf(clstr,"%.2f",x/(float)COL_LINE);
		TextOut(hdc,ix,rt.bottom-16,clstr,lstrlen(clstr));
	}

	//select time line
	SelectPen(hdc,(HPEN)GetStockPen(WHITE_PEN));
	int sel_x = (int)(select_timeline_rate*rt.right+rt.left);
	MoveToEx(hdc,sel_x,0,NULL);
	LineTo(hdc,sel_x,rt.bottom);

	//select keyframe line
	HPEN redpen = CreatePen(PS_SOLID, 0, RGB(255,0,0));
	HPEN oldpen = SelectPen(hdc,redpen);
	int key_x = (int)(select_keyframe_rate*rt.right+rt.left);
	MoveToEx(hdc,key_x,0,NULL);
	LineTo(hdc,key_x,rt.bottom);
	SelectPen(hdc,oldpen);
	DeleteObject(redpen);
}

HDC CTimelineView::GetBackDC()
{
	OnControlDraw(hBackDC);
	return hBackDC;
}

void CTimelineView::ForceDraw()
{
	//HDC hdc = GetDC(my_hwnd);
	//OnControlDraw(hdc);
	//ReleaseDC(my_hwnd,hdc);
}



void CTimelineView::Create(HWND OwnerhWnd, int x, int y, int width, int height, const char* szTitle)
{
	//ビューを搭載するウィンドウを作る
	WNDCLASSEX wcex;
	if(!GetClassInfoEx(GetModuleHandle(NULL), SZ_CONTROL_CLASS_NAME, &wcex))
	{
		memset(&wcex, 0, sizeof(WNDCLASSEX));
		wcex.cbSize			= sizeof(WNDCLASSEX); 
		wcex.style			= CS_DBLCLKS;
		wcex.lpfnWndProc	= (WNDPROC)WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= GetModuleHandle(NULL);
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)GetStockObject(GRAY_BRUSH);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= SZ_CONTROL_CLASS_NAME;
		wcex.hIconSm		= NULL;

		if(!RegisterClassEx(&wcex)){
			ExitProcess(0);
			return;
		}
	}
	
	if(!(my_hwnd=CreateWindow(SZ_CONTROL_CLASS_NAME, szTitle,  WS_CHILDWINDOW|WS_OVERLAPPED,
								x, y, width, height,
								OwnerhWnd, NULL, GetModuleHandle(NULL),
								(void*)this)))
	{
		ExitProcess(0);
		return;
	}
	owner_hwnd  = OwnerhWnd;
	SetProp(my_hwnd,"CCommonControlInfo",(HANDLE)this);//thisをウィンドウハンドルに関連付ける
	ShowWindow(my_hwnd, SW_SHOWNORMAL);
	UpdateWindow(my_hwnd);
}

LRESULT CALLBACK CTimelineView::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CTimelineView* wnd = (CTimelineView*)GetProp(hWnd,"CCommonControlInfo");
	
	//thisの内容が取得できなければそのままDefWindowProcに渡す。
	if(!wnd) return DefWindowProc(hWnd, msg, wParam, lParam);

	if(wnd->WndProcFuncPtr!=NULL) wnd->WndProcFuncPtr(hWnd, msg, wParam, lParam);

	static int leftclicked=0;
	static bool initflag=true;
	switch(msg)
	{
		case WM_CLOSE://prevent closing window - added by c.r.v.
			return FALSE;
		case WM_CREATE:{
			
		}break;
		case WM_SIZE:{
			RECT rt;
			GetClientRect(hWnd, &rt);
			SelectObject(wnd->hBackDC, wnd->oldBmp); // 裏画面を作成したビットマップに関連付ける。
			DeleteObject(wnd->hBackBmp);
			HDC hDC = GetDC(GetDesktopWindow()); // hWndはウインドウプロシージャのもの
				wnd->hBackBmp = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
			ReleaseDC(GetDesktopWindow(), hDC);
			wnd->oldBmp = (HBITMAP)SelectObject(wnd->hBackDC, wnd->hBackBmp); // 裏画面を作成したビットマップに関連付ける。
		}break;
		case WM_ERASEBKGND:{
			break;
		}break;
		case WM_PAINT:{
			if( initflag )
			{
				initflag = false;
				RECT rt;
				GetClientRect(hWnd, &rt);
				wnd->winRect = rt;
				HDC hDC = GetDC(GetDesktopWindow()); // hWndはウインドウプロシージャのもの
				wnd->hBackDC = CreateCompatibleDC(hDC);
				wnd->hBackBmp = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
				wnd->oldBmp = (HBITMAP)SelectObject(wnd->hBackDC, wnd->hBackBmp); // 裏画面を作成したビットマップに関連付ける。
				ReleaseDC(GetDesktopWindow(), hDC);
			}
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd , &ps);
			BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, wnd->hBackDC, 0,0, SRCCOPY);
			EndPaint(hWnd , &ps);
		}break;

		case WM_LBUTTONDOWN:{
			SetCapture(hWnd);
			leftclicked=1;
			long mx = LOWORD(lParam);
			long my = HIWORD(lParam);
			wnd->SetSelectRate(mx, my);
			InvalidateRect(hWnd,NULL,TRUE);
		}break;
		case WM_LBUTTONUP:{
			ReleaseCapture();
			leftclicked=0;
		}break;
		case WM_MOUSEMOVE:{
			if(leftclicked==1){//連続で左クリックをしているのと同じ
				POINT cp;
				GetCursorPos(&cp);
				ScreenToClient(hWnd,&cp);
				long mx = cp.x;
				long my = cp.y;
				wnd->SetSelectRate(mx, my);
				InvalidateRect(hWnd,NULL,TRUE);
			}
		}break;
	}
	
	//いわゆるここで親クラスのWndProcは呼んではいけないってやつ
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CTimelineView::SetSelectRate(int x, int y)
{
	RECT rt;
	GetClientRect(my_hwnd,&rt);
	select_timeline_rate = (float)((x-40)/(float)(rt.right-10-40));
	if(select_timeline_rate<0.0f) select_timeline_rate=0.0f;
	if(select_timeline_rate>1.0f) select_timeline_rate=1.0f;
}

void CTimelineView::SetSelectRateIndirect(float rate){
	select_timeline_rate = rate;
	if(rate<0.0f) select_timeline_rate=0.0f;
	if(rate>1.0f) select_timeline_rate=1.0f;
}

CTimelineView::CTimelineView(void)
{
	select_timeline_rate = 0;
	select_keyframe_rate = 0;
}

CTimelineView::~CTimelineView(void)
{

}
