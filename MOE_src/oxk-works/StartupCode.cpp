#pragma warning(disable:4244)//型変換の警告排除

#include "stdafx.h"
#include "StartupCode.h"
#include "CloneWindow.h"
#include "SceneList.h"
#include "SceneObjectList.h"
#include "ObjectList.h"
#include "LayouterWindow.h"
#include "LayoutFileManager.h"
#include "./kCtrl/timelineview.h"
#include "CDemo.h"
#include "Sync.h"

CWindow win;
CWindowGL wingl;
extern char g_szLayoutName[1024];
extern CTreeView vClone;
extern CListView vSceneList;
extern CListView vSceneObjectList;
extern CListView vObjectList;
CTimelineView vTimeline;
extern CDemo demo;

//Sync
extern CSoundWindow vSound;
extern CKSynthWindow vSynth;

//ファイル更新確認用(外部で編集されたことを検出するものではない
static DWORD dwLastSaveTime = 0;
static DWORD dwCurrentTime = 0;

void Initialize();
void OnDraw(HDC hDC);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int cmain();
void VerifyOnExit();
void VerifyBeforeOpenFile();
LRESULT CALLBACK HWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HGLWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

TCHAR* g_szFileName;

long nCurrentMode = LAYOUTER;//Layouter
extern BOOL isPlaying;

void CreateNewFrame(float frame_time)
{
	long sc = GetSelectedScene();
	if(sc!=-1){
		long sco = GetSelectedSceneObject();
		if( sco != -1 )
			demo.scene[sc].sceneobj[sco].CreateKeyFrame(frame_time);
	}
}

void PlayMusic(float Offset){
	vSound.Play(Offset);
}
void StopMusic(){
	vSound.Stop();
}

float GetNearestTimelineSelection(float fSourcePoint){
	long sc = GetSelectedScene();
	long sco = GetSelectedSceneObject();

	float dmin=10.0f;
	float tmin=-1.0f;
	if(sc!=-1 && sco!=-1){
		vector<float>::iterator tit,teit=demo.scene[sc].sceneobj[sco].anim.anim_time.end();
		for(tit=demo.scene[sc].sceneobj[sco].anim.anim_time.begin(); tit!=teit; tit++){
			float dd = (*tit) - fSourcePoint;
			if(dmin>(dd*dd)){
				dmin = dd*dd;
				tmin = (*tit);
			}
		}
	}
	return tmin;
}

extern void KeyInput();
extern int keydata[256];
void TimelineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HPEN hRedPen = CreatePen(PS_SOLID, 1, RGB(255,0,0));
	static HPEN hBluePen = CreatePen(PS_SOLID, 1, RGB(0,0,255));
	static HPEN hGreenPen = CreatePen(PS_SOLID, 1, RGB(0,255,0));
	static HPEN hDashRedPen = CreatePen(PS_DOT, 1, RGB(255,0,0));
	static HPEN hDashBluePen = CreatePen(PS_DOT, 1, RGB(0,0,255));
	static HPEN hDashGreenPen = CreatePen(PS_DOT, 1, RGB(0,255,0));

	//SetFocus(hWnd);
	if(msg==WM_KEYDOWN || msg==WM_KEYUP)
		CallWindowProc((WNDPROC)GetWindowLong(hGLWnd, GWL_WNDPROC), hGLWnd, msg, wParam, lParam);

	static int mouse_lbtn=0, mouse_rbtn=0;
	if(msg==WM_LBUTTONDOWN) mouse_lbtn=1;
	if(msg==WM_LBUTTONUP)   mouse_lbtn=0;
	if(msg==WM_RBUTTONDOWN) mouse_rbtn=1;
	if(msg==WM_RBUTTONUP){
		
		RECT rt;
		GetClientRect(vTimeline.GetHWnd(),&rt);
		mouse_rbtn=0;
		long mx = LOWORD(lParam);
		float msx = (mx-40)/(float)(rt.right-50);
		if(msx>1.0f) msx = 1.0f;
		if(msx<0.0f) msx = 0.0f;

		//一番近くのキーフレームを選択する
		vTimeline.select_keyframe_rate = GetNearestTimelineSelection(msx);
	}
	if(msg==WM_MBUTTONDOWN){
		float current_rate = vTimeline.GetSelectRate();//白い線のRate
		long sc = GetSelectedScene();
		long sco = GetSelectedSceneObject();
		float keyframe_rate = vTimeline.select_keyframe_rate;//赤い線のRate
		vector<KCloneData> cln;
		if(sc!=-1 && sco!=-1){
			if(&demo.scene[sc].sceneobj[sco]!=NULL){
				KModelEdit* mdl = demo.scene[sc].sceneobj[sco].model;
				if(mdl){
					int nObj = mdl->GetCloneAllocNum();
					
					int i;
					for(i=0; i<nObj; i++){
						KCloneData bt = demo.scene[sc].sceneobj[sco].anim.GetBoneTrans(i, current_rate, demo.scene[sc].sceneobj[sco].interpolate);
						cln.push_back(bt);
					}

					for(i=0; i<nObj; i++){
						long keyframe_skt = GetFindAnimNumber(keyframe_rate, &(demo.scene[sc].sceneobj[sco].anim));
						KCloneData* bt = &(demo.scene[sc].sceneobj[sco].anim.anim[i][keyframe_skt]);
						*bt = cln[i];
					}
				}
			}
		}
	}

	if(nCurrentMode==LAYOUTER){
		if((mouse_lbtn==1)&&(mouse_rbtn==1)){
			mouse_rbtn=0;
			//新規フレーム
			CreateNewFrame(vTimeline.GetSelectRate());
		}
		if(msg==WM_RBUTTONDBLCLK){//削除
			long sc = GetSelectedScene();
			long sco = GetSelectedSceneObject();
			float rt = vTimeline.select_keyframe_rate;
			if((sc!=-1)&&(sco!=-1)){
				if(&demo.scene[sc].sceneobj[sco]!=NULL){
					demo.scene[sc].sceneobj[sco].DeleteKeyFrame(rt);
				}
			}
		}
	}

	BOOL isShiftPressing = ((GetAsyncKeyState(VK_SHIFT)&0x8000) == 0x8000);
	BOOL isCtrlPressing = ((GetAsyncKeyState(VK_CONTROL)&0x8000) == 0x8000);
	if(msg==WM_MOUSEWHEEL){
		if(nCurrentMode==TIMELINER && isPlaying) return;

		long sc = GetSelectedScene();
		long sco = GetSelectedSceneObject();
		float rt = vTimeline.select_keyframe_rate;
		if(sc!=-1 && sco!=-1){
			if(&demo.scene[sc].sceneobj[sco]!=NULL){
				if((rt==0.0f)||(rt==1.0f)) return;

				float fNewTimeRate;
				if((int)wParam>0){//UP     ...unsignedだから，intにキャストしないとマイナスが算出できない
					fNewTimeRate = (isShiftPressing) ? rt+0.001f : rt+0.01f;
					fNewTimeRate = (isCtrlPressing) ? rt+0.1f : fNewTimeRate;
					if(fNewTimeRate>=1.0f) fNewTimeRate = 0.999f;
				}else{//DOWN
					fNewTimeRate = (isShiftPressing) ? rt-0.001f : rt-0.01f;
					fNewTimeRate = (isCtrlPressing) ? rt-0.1f : fNewTimeRate;
					if(fNewTimeRate<=0.0f) fNewTimeRate = 0.001f;
				}

				vector<float>::iterator tit,teit=demo.scene[sc].sceneobj[sco].anim.anim_time.end();
				for(tit=demo.scene[sc].sceneobj[sco].anim.anim_time.begin(); tit!=teit; tit++){
					if(*tit == rt){
						*tit = fNewTimeRate;
						break;
					}
				}
				vTimeline.select_keyframe_rate = fNewTimeRate;

				sort(demo.scene[sc].sceneobj[sco].anim.anim_time.begin(), demo.scene[sc].sceneobj[sco].anim.anim_time.end());
				//tit,teit=demo.scene[sc].sceneobj[sco].anim.anim_time.end();//STLすげぇぇぇぇぇぇぇぇぇ！！！！！！
				//for(tit=demo.scene[sc].sceneobj[sco].anim.anim_time.begin(); tit!=teit; tit++){
				//	char sz[64];
				//	sprintf(sz, "%f\n", *tit);
				//	OutputDebugString(sz);
				//}
			}
		}
	}

	Render();//InvalidateRect(wingl.CGethWnd(), NULL, FALSE);
	
	RECT rt;
	GetClientRect(vTimeline.GetHWnd(),&rt);
	HDC hdc = vTimeline.GetBackDC();

	//draw frame marking
	if(demo.scene.size()>0){
		long sc = GetSelectedScene();
		long sco = GetSelectedSceneObject();
		if(sc!=-1 && sco!=-1){
			
			vector<float>::iterator tit,teit=demo.scene[sc].sceneobj[sco].anim.anim_time.end();
			
			//Marker
			SelectBrush(hdc,GetStockBrush(WHITE_BRUSH));
			for(tit=demo.scene[sc].sceneobj[sco].anim.anim_time.begin(); tit!=teit; tit++){
				long w = (long)((*tit)*(rt.right-50) + 40);
				long h = (long)((rt.bottom*0.5f));
				Rectangle(hdc,w-3,h-3,w+3,h+3);
			}

			long son = CloneGetSelectedItem();
			if(son!=-1){
				//Speed Line
				HPEN oldPen;
				if(GetModifyMode() == MODIFY_TRANSLATION)
					oldPen = SelectPen(hdc,hDashRedPen);
				else
					oldPen = SelectPen(hdc,hRedPen);

				MoveToEx(hdc, 40, (rt.bottom*0.5f), NULL);
				const float difRate = 0.01f;
				for(float atim = difRate; atim <= 1.0f; atim += difRate){
					KCloneData kcd1 = demo.scene[sc].sceneobj[sco].anim.GetBoneTrans(son, atim - difRate, demo.scene[sc].sceneobj[sco].interpolate);
					KCloneData kcd2 = demo.scene[sc].sceneobj[sco].anim.GetBoneTrans(son, atim, demo.scene[sc].sceneobj[sco].interpolate);
					if(GetModifyMode() == MODIFY_TRANSLATION)
						LineTo(hdc, atim*(rt.right-50) + 40, (kcd1.pos.x - kcd2.pos.x) * 100.0f + (rt.bottom*0.5f));
					else if(GetModifyMode() == MODIFY_ROTATION)
						LineTo(hdc, atim*(rt.right-50) + 40, ( -kcd1.rot.x / 360.0f + 1.0f ) * (rt.bottom*0.5f));
					else if(GetModifyMode() == MODIFY_SCALING)
						LineTo(hdc, atim*(rt.right-50) + 40, ( -kcd1.scale.x*0.1f + 1.0f ) * (rt.bottom*0.5f));
					else if(GetModifyMode() == MODIFY_ALPHA)
						LineTo(hdc, atim*(rt.right-50) + 40, ( -kcd1.alpha + 1.0f ) * (rt.bottom*0.5f));
				}
				if(GetModifyMode() == MODIFY_TRANSLATION)
					SelectPen(hdc,hDashGreenPen);
				else
					SelectPen(hdc,hGreenPen);
				MoveToEx(hdc, 40, (rt.bottom*0.5f), NULL);
				for(float atim = difRate; atim < 1.0f; atim += difRate){
					KCloneData kcd1 = demo.scene[sc].sceneobj[sco].anim.GetBoneTrans(son, atim - difRate, demo.scene[sc].sceneobj[sco].interpolate);
					KCloneData kcd2 = demo.scene[sc].sceneobj[sco].anim.GetBoneTrans(son, atim, demo.scene[sc].sceneobj[sco].interpolate); 
					if(GetModifyMode() == MODIFY_TRANSLATION)
						LineTo(hdc, atim*(rt.right-50) + 40, (kcd1.pos.y - kcd2.pos.y) * 100.0f + (rt.bottom*0.5f));
					else if(GetModifyMode() == MODIFY_ROTATION)
						LineTo(hdc, atim*(rt.right-50) + 40, ( -kcd1.rot.y / 360.0f + 1.0f ) * (rt.bottom*0.5f));
					else if(GetModifyMode() == MODIFY_SCALING)
						LineTo(hdc, atim*(rt.right-50) + 40, ( -kcd1.scale.y*0.1f + 1.0f ) * (rt.bottom*0.5f));
				}
				if(GetModifyMode() == MODIFY_TRANSLATION)
					SelectPen(hdc,hDashBluePen);
				else
					SelectPen(hdc,hBluePen);
				MoveToEx(hdc, 40, (rt.bottom*0.5f), NULL);
				for(float atim = difRate; atim < 1.0f; atim += difRate){
					KCloneData kcd1 = demo.scene[sc].sceneobj[sco].anim.GetBoneTrans(son, atim - difRate, demo.scene[sc].sceneobj[sco].interpolate);
					KCloneData kcd2 = demo.scene[sc].sceneobj[sco].anim.GetBoneTrans(son, atim, demo.scene[sc].sceneobj[sco].interpolate); 
					if(GetModifyMode() == MODIFY_TRANSLATION)
						LineTo(hdc, atim*(rt.right-50) + 40, (kcd1.pos.z - kcd2.pos.z) * 100.0f + (rt.bottom*0.5f));
					else if(GetModifyMode() == MODIFY_ROTATION)
						LineTo(hdc, atim*(rt.right-50) + 40, ( -kcd1.rot.z / 360.0f + 1.0f ) * (rt.bottom*0.5f));
					else if(GetModifyMode() == MODIFY_SCALING)
						LineTo(hdc, atim*(rt.right-50) + 40, ( -kcd1.scale.z*0.1f + 1.0f ) * (rt.bottom*0.5f));
				}

				SelectPen(hdc,oldPen);
			}
		}
	}

	HDC hWndDC = GetDC(vTimeline.GetHWnd());
	BitBlt(hWndDC, 0,0,rt.right,rt.bottom, hdc, 0,0, SRCCOPY);
	ReleaseDC(vTimeline.GetHWnd(), hWndDC);
}

void UpdateSceneTime(float fscene_time){
	if(demo.scene.size()<=0){//nop
	}else{
		demo.ReadyDemo();//end_scenetimeを更新する必要がある
		vSynth.Refresh();
	}
}

void UpdateTimeline(){
	//vTimeline.SetSelectRateIndirect(0.0f);
	vTimeline.select_keyframe_rate = GetNearestTimelineSelection(vTimeline.GetSelectRate());
	SendMessage(vTimeline.GetHWnd(), WM_USER+1000, 0, 0);
	//vTimeline.ForceDraw();
}

void OnDraw(HDC hDC)
{
	Render();
}

void ChangeWindowStyle(HWND hWnd, DWORD dwStyle)
{
	if(!hWnd) return;
//	SetParent(hWnd, hMainWnd);
	if(dwStyle>0)
	{
		SetWindowLong(hWnd, GWL_STYLE, dwStyle);
		SetWindowPos(hWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	}
	InvalidateRect(hMainWnd, NULL, TRUE);
}

void RefreshWindows()
{
	if(nCurrentMode==LAYOUTER){
		HWND hWndList[6] = { vSceneList.GetHolderWnd(), wingl.CGethWnd(), vSceneObjectList.GetHolderWnd(),
							vObjectList.GetHolderWnd() ,   vTimeline.GetHWnd(), vClone.GetHolderWnd()};
		//float real_ratio[6][4] = { {0.0f, 0.0f, 0.1f, 0.5f}, {0.1f, 0.0f, 0.7f, 0.7125f}, {0.8f, 0.0f, 0.2f, 0.5f},
		//					       {0.0f, 0.5f, 0.1f, 0.5f}, {0.2f, 0.8f, 0.5f, 0.15f}, {0.8f, 0.5f, 0.2f, 0.5f}};
		float real_ratio[6][4] = {  {0.0f, 0.8f, 0.15f, 1-0.8f},
									{0.0f, 0.0f, 0.78125f, 0.8f},
									{0.78125f, 0.2f, 1.0f-0.78125f, 0.3f},
								    {0.78125f,0.0f, 1.0f-0.78125f, 0.2f},
									{0.15f, 0.8f, 0.78125f-0.15f, 0.2f},
									{0.78125f, 0.5f, 1.0f-0.78125f, 0.5f}
								 };

		RECT rect;
		GetClientRect(hMainWnd, &rect);
		
		//上段
		//SetWindowPos(hWndList[0], 0, 0, 0, rect.right*ratio[0][0], rect.bottom*ratio[0][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[0], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[0], 0, rect.right*real_ratio[0][0], rect.bottom*real_ratio[0][1], rect.right*real_ratio[0][2], rect.bottom*real_ratio[0][3], SWP_NOZORDER);

		//SetWindowPos(hWndList[1], 0, rect.right*ratio[0][0], 0, rect.right*ratio[1][0], rect.right*ratio[1][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[1], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[1], 0, rect.right*real_ratio[1][0], rect.bottom*real_ratio[1][1], rect.right*real_ratio[1][2], rect.bottom*real_ratio[1][3], SWP_NOZORDER);

		//SetWindowPos(hWndList[2], 0, rect.right*ratio[0][0]+rect.right*ratio[1][0], 0, rect.right*ratio[2][0], rect.right*ratio[2][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[2], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[2], 0, rect.right*real_ratio[2][0], rect.bottom*real_ratio[2][1], rect.right*real_ratio[2][2], rect.bottom*real_ratio[2][3], SWP_NOZORDER);
		//RECT t;
		//GetClientRect(hWndList[2], &t);
		//SetWindowPos(vObjectList.GetListWnd() , 0, 0, 0, t.right, t.bottom, SWP_NOZORDER|SWP_NOMOVE);

		//下段
		//SetWindowPos(hWndList[3], 0, 0, rect.bottom*ratio[3][1], rect.right*ratio[3][0], rect.bottom*ratio[3][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[3], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[3], 0, rect.right*real_ratio[3][0], rect.bottom*real_ratio[3][1], rect.right*real_ratio[3][2], rect.bottom*real_ratio[3][3], SWP_NOZORDER);

		//SetWindowPos(hWndList[4], 0, rect.right*ratio[3][0], rect.bottom*ratio[4][1], rect.right*ratio[4][0], rect.bottom*ratio[4][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[4], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[4], 0, rect.right*real_ratio[4][0], rect.bottom*real_ratio[4][1], rect.right*real_ratio[4][2], rect.bottom*real_ratio[4][3], SWP_NOZORDER);

		//SetWindowPos(hWndList[5], 0, rect.right*ratio[3][0]+rect.right*ratio[4][0], rect.bottom*ratio[5][1], rect.right*ratio[5][0], rect.bottom*ratio[5][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[5], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[5], 0, rect.right*real_ratio[5][0], rect.bottom*real_ratio[5][1], rect.right*real_ratio[5][2], rect.bottom*real_ratio[5][3], SWP_NOZORDER);
	}else{
		HWND hWndList[6] = { vSceneList.GetHolderWnd(), wingl.CGethWnd(), vSceneObjectList.GetHolderWnd(),
							vSynth.GetHolderWnd() ,   vTimeline.GetHWnd(), vSound.GetHolderWnd()};

		//x,y, width, height
		float real_ratio[6][4] = {	{0.0f, 0.8f, 0.15f, 1-0.8f},
									{0.0f, 0.0f, 0.78125f, 0.8f},
									{0.78125f, 0.40f, 1.0f-0.78125f, 0.30f},//SceneObjectList
									//{0.78125f, 0.40f, 1.0f-0.78125f, 0.40f},//Time
									{0.78125f, 0.0f, 1.0f-0.78125f, 0.40f},//Synth
									{0.15f, 0.8f, 0.78125f-0.15f, 0.2f},
									{0.78125f, real_ratio[2][3] + real_ratio[3][3], 1.0f-0.78125f, 1.00f - real_ratio[5][1]}//Sound
								 };

		RECT rect;
		GetClientRect(hMainWnd, &rect);

		//上段
		//SetWindowPos(hWndList[0], 0, 0, 0, rect.right*ratio[0][0], rect.bottom*ratio[0][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[0], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[0], 0, rect.right*real_ratio[0][0], rect.bottom*real_ratio[0][1], rect.right*real_ratio[0][2], rect.bottom*real_ratio[0][3], SWP_NOZORDER);

		//SetWindowPos(hWndList[1], 0, rect.right*ratio[0][0], 0, rect.right*ratio[1][0], rect.right*ratio[1][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[1], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[1], 0, rect.right*real_ratio[1][0], rect.bottom*real_ratio[1][1], rect.right*real_ratio[1][2], rect.bottom*real_ratio[1][3], SWP_NOZORDER);

		//SetWindowPos(hWndList[2], 0, rect.right*ratio[0][0]+rect.right*ratio[1][0], 0, rect.right*ratio[2][0], rect.right*ratio[2][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[2], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[2], 0, rect.right*real_ratio[2][0], rect.bottom*real_ratio[2][1], rect.right*real_ratio[2][2], rect.bottom*real_ratio[2][3], SWP_NOZORDER);
		//RECT t;
		//GetClientRect(hWndList[2], &t);
		//SetWindowPos(vObjectList.GetListWnd() , 0, 0, 0, t.right, t.bottom, SWP_NOZORDER|SWP_NOMOVE);

		//下段
		//SetWindowPos(hWndList[3], 0, 0, rect.bottom*ratio[3][1], rect.right*ratio[3][0], rect.bottom*ratio[3][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[3], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[3], 0, rect.right*real_ratio[3][0], rect.bottom*real_ratio[3][1], rect.right*real_ratio[3][2], rect.bottom*real_ratio[3][3], SWP_NOZORDER);

		//SetWindowPos(hWndList[4], 0, rect.right*ratio[3][0], rect.bottom*ratio[4][1], rect.right*ratio[4][0], rect.bottom*ratio[4][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[4], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[4], 0, rect.right*real_ratio[4][0], rect.bottom*real_ratio[4][1], rect.right*real_ratio[4][2], rect.bottom*real_ratio[4][3], SWP_NOZORDER);

		//SetWindowPos(hWndList[5], 0, rect.right*ratio[3][0]+rect.right*ratio[4][0], rect.bottom*ratio[5][1], rect.right*ratio[5][0], rect.bottom*ratio[5][1], SWP_NOZORDER);
		ChangeWindowStyle(hWndList[5], WS_POPUP|WS_VISIBLE);
		SetWindowPos(hWndList[5], 0, rect.right*real_ratio[5][0], rect.bottom*real_ratio[5][1], rect.right*real_ratio[5][2], rect.bottom*real_ratio[5][3], SWP_NOZORDER);
	}
}


WNDPROC CloneOldWndProc,
		SceneListOldWndProc,
		ObjectListOldWndProc,
		SceneObjectListOldWndProc;

LRESULT CALLBACK SceneListWndHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	CallWindowProc(SceneListHookProc, hWnd, msg, wParam, lParam);
	return CallWindowProc(SceneListOldWndProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK SceneObjectListWndHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	CallWindowProc(SceneObjectListHookProc, hWnd, msg, wParam, lParam);
	return CallWindowProc(SceneObjectListOldWndProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK ObjectListWndHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	CallWindowProc(ObjectListHookProc, hWnd, msg, wParam, lParam);
	return CallWindowProc(ObjectListOldWndProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK CloneWndHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	CallWindowProc(CloneHookProc, hWnd, msg, wParam, lParam);
	return CallWindowProc(CloneOldWndProc, hWnd, msg, wParam, lParam);
}

void Initialize(int width, int height)
{
	RECT rect;
	rect.right = width;
	rect.bottom = height;
	
	/* for Layouter*/
	float ratio[6][2] = { {0.2f, 0.5f}, {0.6f, 0.4f}, {0.2f, 0.5f},
						  {0.2f, 0.5f}, {0.6f, 0.6f}, {0.2f, 0.5f}};
	vSceneList.CreateListView(0, 0, rect.right*ratio[0][0], rect.bottom*ratio[0][1], "SceneList", hMainWnd);
	DWORD dwStyle = LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyle(vSceneList.GetListWnd(), dwStyle);
	vSceneList.SetCallbackFunctions(SceneListNameEdit, SceneListPopup, SceneListCommand, SceneListKey, SceneListMouse, SceneListSelectedItemNotify);
	vSceneList.SetHookProcedure(SceneListHookProc);
	vSceneList.AddColumn("Scene", LVSCW_AUTOSIZE, 0);


	wingl.CCreateWindow(rect.right*ratio[0][0], 0, rect.right*ratio[1][0], rect.bottom*ratio[1][1], "OpenGL");
	wingl.CSetCallbackFunctions(LayouterKeyEvent, LayouterMouseEvent, OnDraw);//, OnIdle);
	SetParent(wingl.CGethWnd(), hMainWnd);

	vSceneObjectList.CreateListView(rect.right*ratio[0][0] + rect.right*ratio[1][0], 0, rect.right*ratio[2][0], rect.bottom*ratio[2][1], "SceneObjectList", hMainWnd);
	dwStyle = LVS_EX_GRIDLINES;
	//dwStyle = LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyle(vSceneObjectList.GetListWnd(), dwStyle);
	vSceneObjectList.SetCallbackFunctions(SceneObjectListNameEdit, SceneObjectListPopup, SceneObjectListCommand, SceneObjectListKey, SceneObjectListMouse, SceneObjectListSelectedItemNotify);
	vSceneObjectList.SetHookProcedure(SceneObjectListHookProc);
	vSceneObjectList.AddColumn("camera_trans", LVSCW_AUTOSIZE, 2);
	vSceneObjectList.AddColumn("interpolate", LVSCW_AUTOSIZE, 1);
	vSceneObjectList.AddColumn("Scene Object", LVSCW_AUTOSIZE, 0);


	vObjectList.CreateListView(0, rect.bottom*ratio[3][1], rect.right*ratio[3][0], rect.bottom*ratio[3][1], "ObjectList", hMainWnd);
	dwStyle = LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyle(vObjectList.GetListWnd(), dwStyle);
	vObjectList.SetCallbackFunctions(ObjectListNameEdit, ObjectListPopup, ObjectListCommand, ObjectListKey, ObjectListMouse, ObjectListSelectedItemNotify);
	vObjectList.SetHookProcedure(ObjectListHookProc);
	vObjectList.AddColumn("Object", LVSCW_AUTOSIZE, 0);

	vTimeline.Create(hMainWnd, rect.right*ratio[3][0], rect.bottom*ratio[4][1], rect.right*ratio[4][0], rect.bottom*ratio[4][1], "Timeline");
	vTimeline.SetWndProcCallback(TimelineProc);


	vClone.CreateTreeView(rect.right*ratio[3][0]+rect.right*ratio[4][0], rect.bottom*ratio[5][1], rect.right*ratio[5][0], rect.bottom*ratio[5][1], "Clone", hMainWnd);
	vClone.SetCallbackFunctions(CloneNameEdit, NULL, NULL, CloneKey, CloneMouse, CloneSelectedItemNotify); 
	vClone.SetHookProcedure(CloneHookProc);

	LONG oldStyle = GetWindowLong(vClone.GetTreeWnd(), GWL_STYLE);
	oldStyle |= TVS_CHECKBOXES;
	SetWindowLong(vClone.GetTreeWnd(), GWL_STYLE, oldStyle);


	SceneObjectListOldWndProc = (WNDPROC)SetWindowLong(vSceneObjectList.GetListWnd(), GWL_WNDPROC, (LONG)SceneObjectListWndHookProc);
	ObjectListOldWndProc = (WNDPROC)SetWindowLong(vObjectList.GetListWnd(), GWL_WNDPROC, (LONG)ObjectListWndHookProc);
	SceneListOldWndProc = (WNDPROC)SetWindowLong(vSceneList.GetListWnd(), GWL_WNDPROC, (LONG)SceneListWndHookProc);
	CloneOldWndProc = (WNDPROC)SetWindowLong(vClone.GetTreeWnd(), GWL_WNDPROC, (LONG)CloneWndHookProc);

	/* for Timeliner*/
	vSound.CreateSoundWindow(rect.right*ratio[3][0]+rect.right*ratio[4][0], rect.bottom*ratio[5][1], rect.right*ratio[5][0], rect.bottom*ratio[5][1], "Sound", hMainWnd);
	SetParent(vSound.GetHolderWnd(), hMainWnd);

	vSynth.CreateSynthWindow(rect.right*ratio[0][0] + rect.right*ratio[1][0], 0, rect.right*ratio[2][0], rect.bottom*ratio[2][1], "Synth", hMainWnd);
	SetParent(vSynth.GetHolderWnd(), hMainWnd);
	//ksd.SetNotifyCallback(vSynth.KSynthLoadNotify);

	RefreshWindows();

	//const float lratio [2] = { 19.0f, 50.0f };
	//const float rratio[2] = { 19.0f, 50.0f };
	//const float cratio = { 100.0f - lratio[0] - rratio[0] };
	//float w[3], h[3];


	////左
	//w[0] = width /100.0*lratio[0];
	//h[0] = height/100.0*lratio[1];
	//vSceneList.CreateListView(0, 0, w[0], h[0], "SceneList", hMainWnd);
	//DWORD dwStyle = LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES;
	//ListView_SetExtendedListViewStyle(vSceneList.GetListWnd(), dwStyle);
	//vSceneList.SetCallbackFunctions(SceneListNameEdit, NULL, SceneListCommand, SceneListKey, SceneListMouse, SceneListSelectedItemNotify);
	//vSceneList.SetHookProcedure(SceneListHookProc);
	//vSceneList.AddColumn("Scene", w[0], 0);
	//ChangeWindowStyle(vSceneList.GetHolderWnd(), WS_POPUP|WS_VISIBLE);

	//vObjectList.CreateListView(0, h[0], w[0], h[0], "ObjectList", hMainWnd);
	//dwStyle = LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES;
	//ListView_SetExtendedListViewStyle(vObjectList.GetListWnd(), dwStyle);
	//vObjectList.SetCallbackFunctions(ObjectListNameEdit, ObjectListPopup, ObjectListCommand, ObjectListKey, ObjectListMouse, ObjectListSelectedItemNotify);
	//vObjectList.SetHookProcedure(ObjectListHookProc);
	//vObjectList.AddColumn("Object", w[0], 0);
	//ChangeWindowStyle(vObjectList.GetHolderWnd(), WS_POPUP|WS_VISIBLE);

	////真ん中
	//w[1] = width /100.0 * 60.0;
	//h[1] = height/100.0 * 60.0;
	//wingl.CCreateWindow(w[0]+(width/100.0*1.0f), 0, w[1], h[1], "OpenGL");
	//wingl.CSetCallbackFunctions(LayouterKeyEvent, LayouterMouseEvent, OnDraw);
	//SetParent(wingl.CGethWnd(), hMainWnd);
	//ChangeWindowStyle(wingl.CGethWnd(), WS_POPUP|WS_VISIBLE);

	//vTimeline.Create(hMainWnd, w[0]+(width/100.0*1.0f), h[1]+(height/100.0f*5.0f), w[1], height/100.0*35.0f-50, "Timeline");
	//vTimeline.SetWndProcCallback(TimelineProc);

	////右
	//w[2] = width/100.0*rratio[0];
	//h[2] = height/100.0*rratio[1];
	//vSceneObjectList.CreateListView(w[0]+(width/100.0*cratio), 0, w[2], h[2], "SceneObjectList", hMainWnd);
	//dwStyle = LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES;
	//ListView_SetExtendedListViewStyle(vSceneObjectList.GetListWnd(), dwStyle);
	//vSceneObjectList.SetCallbackFunctions(SceneObjectListNameEdit, NULL, SceneObjectListCommand, SceneObjectListKey, SceneObjectListMouse, SceneObjectListSelectedItemNotify);
	//vSceneObjectList.SetHookProcedure(SceneObjectListHookProc);
	//vSceneObjectList.AddColumn("Scene Object", w[2], 0);
	//ChangeWindowStyle(vSceneObjectList.GetHolderWnd(), WS_POPUP|WS_VISIBLE);

	//vClone.CreateTreeView(w[0]+(width/100.0*cratio), h[2], w[2], h[2], "Clone", hMainWnd);
	//vClone.SetCallbackFunctions(CloneNameEdit, NULL, NULL, CloneKey, CloneMouse, CloneSelectedItemNotify); 
	//vClone.SetHookProcedure(CloneHookProc);
	//LONG oldStyle = GetWindowLong(vClone.GetTreeWnd(), GWL_STYLE);
	//oldStyle |= TVS_CHECKBOXES;
	//SetWindowLong(vClone.GetTreeWnd(), GWL_STYLE, oldStyle);
	//ChangeWindowStyle(vClone.GetHolderWnd(), WS_POPUP|WS_VISIBLE);


	//HMENU hMenu = GetMenu(hMainWnd);
	//HMENU hSub  = GetSubMenu(hMenu, 1);
	//EnableMenuItem(hSub, 0, MF_BYPOSITION|MF_GRAYED);
	//EnableMenuItem(hSub, 1, MF_BYPOSITION|MF_GRAYED);
}

void FreeSynthWindow(){
	vSynth.Clear();
}

void FreeSoundWindow(){
	vSound.Clear();
}

void RefreshSynthWindow(){
	vSynth.Refresh();
}

void RefreshSoundWindow(){
	vSound.Refresh();
}

void FreeAll()
{
	ResetUndo();
	demo.scene.clear();
	for(int i=0; i<256; i++)
		demo.FreePrimitive(i);
	
	FreeSceneList();
	FreeSceneObjectList();
	FreeObjectList();
	FreeCloneWindow();

	FreeSynthWindow();
	FreeSoundWindow();
	//RefreshAllView();
}

void RefreshAllView()
{
	RefreshSceneList();
	RefreshObjectList();
	RefreshSceneObjectList();

	RefreshSynthWindow();
	RefreshSoundWindow();
	//long i = GetSelectedScene();
	//if(i<0) return;
	//long n = GetSelectedObjectNum();
	//KModelEdit* mdl = demo.obj[n];
	//if(!mdl) return;
	//RefreshCloneTree(mdl, NULL, NULL);
}

void VerifyOnExit()
{
	if(MessageBox(hMainWnd, "Exit?", "Exit", MB_YESNO|MB_ICONQUESTION|MB_SYSTEMMODAL)==IDYES){
		SetParent(vClone.GetHolderWnd(), NULL);
		SetParent(vSceneList.GetHolderWnd(), NULL);
		SetParent(vObjectList.GetHolderWnd(), NULL);
		SetParent(vSceneObjectList.GetHolderWnd(), NULL);

		PostQuitMessage(0);
	}
}

LRESULT CALLBACK HGLWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_MOUSEMOVE:
		{
			if(GetForegroundWindow()!=hWnd) SetFocus(hMainWnd);
			SetFocus(hWnd);
			//POINT pt;
			//pt.x = LOWORD(lParam);
			//pt.y = HIWORD(lParam);
			//
			////ClientToScreen(hMainWnd, &pt);
			//HWND hTWnd = NULL;
			//if((hTWnd = ChildWindowFromPoint(hMainWnd, pt)))
			//{
			//	SetFocus(hTWnd);
			//}
			break;
		}
		case WM_DESTROY:
		case WM_CLOSE:
		{
			VerifyOnExit();
			return FALSE;
		}
		//case WM_DROPFILES:
		//{
		//	HDROP hDrop;
		//	char szFilename[1024] = {0};
		//	hDrop = (HDROP)wParam;
		//	DragQueryFile(hDrop, 0, szFilename, 1024);
		//	DragFinish(hDrop);

		//	if(MessageBox(hMainWnd, "Open a dropped file?", szFilename, MB_YESNO|MB_ICONQUESTION|MB_SYSTEMMODAL)==IDYES){
		//		lstrcpy(g_szFileName, szFilename);
		//		FreeAll();
		//		demo.Load(szFilename);
		//		RefreshAllView();
		//	}
		//	break;
		//}
		//case WM_KEYDOWN:
		//{
		//	static BOOL isPlaying = FALSE:
		//	if(!isPlaying){
		//		vSound.Play(0);
		//	}else{
		//		vSound.Stop();
		//	}
		//}
	}
	return TRUE;
}

LRESULT CALLBACK HWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		//case WM_ACTIVE:
		//break;
		case WM_KEYDOWN:
		case WM_KEYUP:
			CallWindowProc((WNDPROC)GetWindowLong(hGLWnd, GWL_WNDPROC), hGLWnd, msg, wParam, lParam);
			return TRUE;//オリジナルのメッセージ処理を行わないとLayouterWindow.cppが機能しない
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case ID_FILE_NEW_LAYOUT:
				{
					//long nUndoBuffer = GetUndoBufferNum();
					//if(dwLastSaveTime){
					//	if(MBQ("This document has been modified. Save?")){
					//		CallWindowProc((WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC), hWnd, WM_COMMAND, ID_FILE_SAVE_OW, 0);
					//	}
					//}
					FreeAll();
					RefreshAllView();
					break;
				}
				case ID_FILE_OPEN_LAYOUT:
				{
					char* szFile = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
						if(GetOpenFileNameSingle(hGLWnd, "kdf", szFile, FALSE))
						{
							lstrcpy(g_szFileName, szFile);
							FreeAll();
							demo.Load(szFile);
							RefreshAllView();
						}
					GlobalFree(szFile);
					break;
				}
				case ID_FILE_SAVE_OW:
				{
					if(g_szFileName[0] == '\0')
					{
						char* szFile = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
							if(GetSaveFileNameSingle(hGLWnd, "kdf", szFile, FALSE))
							{
								lstrcpy(g_szFileName, szFile);
							}else return TRUE;
						GlobalFree(szFile);
					}
					
					demo.Save(g_szFileName);
					RefreshAllView();
					break;
				}
				case ID_FILE_SAVE_AS:
				{
					char* szFile = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
						if(GetSaveFileNameSingle(hGLWnd, "kdf", szFile, FALSE))
						{
							lstrcpy(g_szFileName, szFile);
							demo.Save(szFile);
							RefreshAllView();
						}
					GlobalFree(szFile);
					break;
				}
				case ID_FILE_EXPORT:
				{
					char* szFile = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
						if(GetSaveFileNameSingle(hGLWnd, "kdb", szFile, FALSE))
						{
							//lstrcpy(g_szFileName, szFile);
							demo.BinarySave(szFile);
							MessageBox(hWnd, "Export done", "", MB_ICONINFORMATION|MB_OK|MB_TOPMOST);
							RefreshAllView();
						}
					GlobalFree(szFile);
					break;
				}
				case ID_FILE_EXIT:
				{
					VerifyOnExit();	
					break;
				}
				case ID_EDIT_UNDO:
				{
					Undo();	
					break;
				}
				case ID_EDIT_REDO:
				{
					Redo();	
					break;
				}
				case ID_MODE_LAYOUTER:
				{
					StopPlaying();

					nCurrentMode = LAYOUTER;
					HMENU hMenu = GetMenu(hWnd);
					HMENU hSub = GetSubMenu(hMenu, 2);
					CheckMenuItem(hSub, 1, MF_BYPOSITION|MF_UNCHECKED);
					CheckMenuItem(hSub, 0, MF_BYPOSITION|MF_CHECKED);

				
					//vSound.Hide();
					//vSynth.Hide();
					ShowWindow(vSound.GetHolderWnd(), FALSE);
					//ShowWindow(vTime.GetHolderWnd(), FALSE);
					ShowWindow(vSynth.GetHolderWnd(), FALSE);

					//ShowWindow(vScene.GetHolderWnd(), TRUE);
					ShowWindow(vClone.GetHolderWnd(), TRUE);
					ShowWindow(vObjectList.GetHolderWnd(), TRUE);
					ShowWindow(vSceneObjectList.GetHolderWnd(), TRUE);

					RefreshWindows();
					break;
				}
				case ID_MODE_TIMELINER:
				{
					StopPlaying();

					nCurrentMode = TIMELINER;
					HMENU hMenu = GetMenu(hWnd);
					HMENU hSub = GetSubMenu(hMenu, 2);
					CheckMenuItem(hSub, 0, MF_BYPOSITION|MF_UNCHECKED);
					CheckMenuItem(hSub, 1, MF_BYPOSITION|MF_CHECKED);

					//ShowWindow(vScene.GetHolderWnd(), FALSE;
					ShowWindow(vClone.GetHolderWnd(), FALSE);
					//ShowWindow(vSceneObjectList.GetHolderWnd(), FALSE);
					ShowWindow(vObjectList.GetHolderWnd(), FALSE);

					//vSound.Show();
					//vSynth.Show();
					ShowWindow(vSound.GetHolderWnd(), TRUE);
					//ShowWindow(vTime.GetHolderWnd(), TRUE);
					ShowWindow(vSynth.GetHolderWnd(), TRUE);

					RefreshWindows();
					break;
				}
				case ID_FRAME_000:
				case ID_FRAME_025:
				case ID_FRAME_050:
				case ID_FRAME_075:
				case ID_FRAME_100:
				{
					float mul = (LOWORD(wParam) - ID_FRAME_000) * 0.25f;
					UpdateFrameTransparency(mul);
					break;
				}
			}
			return FALSE;
		}
		case WM_DESTROY:
		case WM_CLOSE:
		{
			VerifyOnExit();
			return FALSE;
		}
		case WM_SIZE:
		{
			RefreshWindows();
			return FALSE;
		}
	}
	return TRUE;
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

int	cmain()
{
//#ifndef _DEBUG
//	__try
//#endif
//	{
		//ReadKSynthHeader("jkla3.h");
		setlocale( LC_ALL, "Japanese" );//setlocale for multibyte->widechar functions. reqired by vs2005.

		g_szFileName = (TCHAR*)GlobalAlloc(GPTR, sizeof(TCHAR)*1024);
		const int width = 1024;
		const int height = 768;
		win.CCreateWindow(width, height, "", "LAYOUTVIEW");
		win.CSetWindowText(hMainWnd, "Oxygenz  produced by SystemK 2005  ver.%4.2f (%s %s)", APP_VER, __DATE__, __TIME__);
		win.LoadAccelerators("MAINACCEL");
		HANDLE hIcon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
		SetClassLong(hMainWnd, GCL_HICON, (LONG)hIcon);
	//	ShowCursor(TRUE);
		win.CSetCallbackFunctions(NULL, NULL, NULL, OnIdle);
		//win.CSetCallbackFunctions(KeyEvent, MouseEvent);//, OnDraw);
		win.CSetHookProcedure(HWndProc);
		wingl.CSetHookProcedure(HGLWndProc);

		//編集スクリーンに関しては, WindowsからのWM_PAINTを発生させない.
		//ValidateRect(hMainWhd, NULL);
		ValidateRect(hGLWnd, NULL);

		DragAcceptFiles(hGLWnd, TRUE);

		Initialize(width, height);
		SendMessage(hMainWnd, WM_COMMAND, ID_MODE_LAYOUTER+nCurrentMode, 0);

		InitLayouterWindow();

		//int nArgc;
		//WCHAR** wszCmdLine = CommandLineToArgvW(GetCommandLineW(), &nArgc);
		//int length;
		//char** szArgv = (char**)GlobalAlloc(GPTR, sizeof(char*) * nArgc);
		//int i;
		//for(i=0; i<nArgc; i++)
		//{
		//	if((length = ::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, NULL, 0, NULL, NULL))==0) continue;
		//	szArgv[i] = (char*)GlobalAlloc(GPTR, sizeof(char) * length);
		//	::WideCharToMultiByte(CP_ACP, 0, wszCmdLine[i], -1, szArgv[i], length, NULL, NULL);
		//}

		//if(nArgc>1){
		//	const char* kdf_szHeader = "KScene Demo File";
		//	char* header = (char*)GlobalAlloc(GPTR, sizeof(char) * (lstrlen(kdf_szHeader)+1));
		//		FILE* fp = fopen(szArgv[1] ,"rb");
		//			for(i=0; i<lstrlen(kdf_szHeader); i++) header[i] = fgetc(fp);
		//		fclose(fp);

		//		if(lstrcmp(header, kdf_szHeader)==0){
		//			lstrcpy(g_szFileName, szArgv[1]);
		//			demo.Load(szArgv[1]);
		//		}else{
		//			MessageBox(NULL, "This is not a KDF file!", 0, MB_SYSTEMMODAL);
		//			return FALSE;
		//		}
		//	GlobalFree(header);
		//}
		RefreshAllView();

		win.CMessageLoop();
		GlobalFree(g_szFileName);
	//}
//#ifndef _DEBUG
//	__except(EXCEPTION_EXECUTE_HANDLER)
//	{
//		__try
//		{
//			//MessageBox(NULL, "すみません。例外処理が発生しました。現在の情報をできるだけ吐き出します。", "c.r.v.", MB_SYSTEMMODAL);
//			char szPath[256] = {'\0'};
//			GetCurrentDirectory(sizeof(szPath), szPath);
//			//GetTempPath(sizeof(szPath), szPath);
//			PathRemoveBackslash(szPath);
//			lstrcat(szPath, "\\dump.kdf");
//			//if(demo.Save(szPath)==false){
//			//	MessageBox(NULL, "CDemo::Save()がfalseを返しました。\nデータは保存されませんでした。\nこの後すぐ，例外処理を恣意的に発生させます。", "c.r.v.", MB_SYSTEMMODAL);
//			//	throw;
//			//}
//			//char szMes[1024];
//			///wsprintf(szMes, "最新の情報を保存している最中に例外は発生しませんでした。\nただし，保存されたデータが破損している可能性がありますので，使用する際は十分に気をつけてください。\nデータは下記の場所に保存されました。\n%s\n\nこのメッセージボックスを閉じると，アプリケーションを再起動します。",szPath);
//			//MessageBox(NULL, szMes, "c.r.v.", MB_SYSTEMMODAL);
//			RebootMe();
//		}
//		__except(EXCEPTION_EXECUTE_HANDLER)
//		{
//			//MessageBox(NULL, "申し訳ございません。最新の情報を保存している際に例外が発生しました。すべての作業データは失われます。", "c.r.v.", MB_SYSTEMMODAL); 
//		}
//	}
//#endif
	return 0;
}