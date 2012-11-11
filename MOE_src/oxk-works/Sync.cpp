#include "stdafx.h"
#include "Sync.h"
#include "resource.h"
#include "resrc1.h"

#include "./StartupCode.h"
#include "./SceneList.h"
#include "./LayoutFileManager.h"
#include "./CDemo.h"
#include "../klib/kfpu.h"

//#include <vector>
//using namespace std;
//vector<KSceneTime> SceneMusicTime;

CSoundWindow vSound;
CKSynthWindow vSynth;
enum {MUSIC_NONE=0, MUSIC_KS, MUSIC_CS};
unsigned char byMusicType;

extern CDemo demo;
extern BOOL isPlaying;

BOOL IsNumeric(char cChar, BOOL isFloating){
	BOOL isNumeric = (cChar>='0' && cChar<='9');
	return (isFloating) ? (isNumeric || cChar=='.') : isNumeric;
}

//
//KSynth
//
void CKSynthWindow::EnableControl(BOOL isEnable){
	if( isEnable ){
		//activate contorls
		if( byMusicType==MUSIC_KS ){
			EnableWindow(GetDlgItem(this->hDialog, IDC_KS_TRACK_CALC), TRUE);
			EnableWindow(GetDlgItem(this->hDialog, IDC_KS_DOCALC_AND_SET), TRUE);
			EnableWindow(GetDlgItem(this->hDialog, IDC_TIME_SET_ANSWER), TRUE);
		}else if( byMusicType==MUSIC_CS ){
			EnableWindow(GetDlgItem(this->hDialog, IDC_KS_TRACK_CALC), FALSE);
			EnableWindow(GetDlgItem(this->hDialog, IDC_KS_DOCALC_AND_SET), FALSE);
			EnableWindow(GetDlgItem(this->hDialog, IDC_TIME_SET_ANSWER), FALSE);
		}
		EnableWindow(GetDlgItem(this->hDialog, IDC_TIME_SET), TRUE);
	}else{
		if( byMusicType==MUSIC_KS ){
			EnableWindow(GetDlgItem(this->hDialog, IDC_KS_TRACK_CALC), FALSE);
			EnableWindow(GetDlgItem(this->hDialog, IDC_TIME_SET_ANSWER), FALSE);
			EnableWindow(GetDlgItem(this->hDialog, IDC_TIME_SET), FALSE);
		}else if( byMusicType==MUSIC_CS ){
			EnableWindow(GetDlgItem(this->hDialog, IDC_KS_TRACK_CALC), FALSE);
			EnableWindow(GetDlgItem(this->hDialog, IDC_TIME_SET_ANSWER), FALSE);
			EnableWindow(GetDlgItem(this->hDialog, IDC_TIME_SET), FALSE);
		}
		EnableWindow(GetDlgItem(this->hDialog, IDC_KS_DOCALC_AND_SET), FALSE);
	}
}

void CKSynthWindow::CreateSynthWindow(int x, int y, int width, int height, const TCHAR* szTitle, HWND hParentWnd){
	m_hParentWnd = hParentWnd;
	hDialog = CreateDialogA(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_SYNTH_WINDOW), NULL, (DLGPROC)CKSynthWindow::DlgProc);
	ShowWindow(hDialog, TRUE);
	//UpdateWindow(hDialog);
	m_hWnd = hDialog;
	SetProp(hDialog, szCKSynthWindow, this);
}
LRESULT CALLBACK CKSynthWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	return DefWindowProc(hWnd, msg, wParam, lParam);
};
LRESULT CALLBACK CKSynthWindow::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	CKSynthWindow* that = (CKSynthWindow*)GetProp(hWnd, szCKSynthWindow);
	switch(msg)
	{
		case WM_USER+100:
		{
			char szTrackListBuffer[1024];
			if(0==demo.ksl->GetKSynthTrackList(szTrackListBuffer, sizeof(szTrackListBuffer))){
				that->szTrack = szTrackListBuffer;
				SetDlgItemText(hWnd, IDC_KS_TRACKINFO, szTrackListBuffer);
			}else{
				
			}
			that->fTotalMusicTimeSec = demo.ksl->GetSoundTime();//pKSD->GetKSynthTrackLength(that->szTrack.c_str());

			that->CalcAndShowTotalMusicTime();
			that->CalcAndShowLeftTime();
			that->CalcAndShowTotalSceneTime();
		}break;
		case WM_USER+101:
		{
			that->fTotalMusicTimeSec = demo.ksl->GetSoundTime();
			that->CalcAndShowTotalMusicTime();
		}break;
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_TIME_SET:{
				long sn = GetSelectedScene();
				if(sn!=-1){
					if(isPlaying){
						
					}else{
						char szFloat[64];
						GetDlgItemText(hWnd, IDC_KS_ANSWER, szFloat, sizeof(szFloat));
						//GetDlgItemText(hWnd, IDC_TIME_SCENE, szFloat, sizeof(szFloat));
                        float fNewST = StrToFloat(szFloat);
						fNewST = kfAbs(fNewST);
						fNewST *= 1000.0f;//入力は秒数だから
						demo.scene[sn].fscene_time = fNewST;
						UpdateSceneTime(fNewST);

						//that->CalcAndShowTrackString();
					}
				}
			}break;
			//case IDC_TIME_SET_ANSWER:
			//{
			//	if(isPlaying){
			//	
			//	}else{
			//		char szFloat[64];
			//		GetDlgItemText(hWnd, IDC_KS_ANSWER, szFloat, sizeof(szFloat));
			//		SetDlgItemText(hWnd, IDC_TIME_SCENE, szFloat);
			//		PushButton(hWnd, IDC_TIME_SET);//ボタンを押す
			//	}
			//}break;
			//case IDC_KS_DOCALC:{
			//	//BOOL isKS2Sec = IsDlgButtonChecked(hWnd, IDC_KS_KSTOSEC);
			//	//if(isKS2Sec){
			//		char szTrack[512];
			//		GetDlgItemText(hWnd, IDC_KS_TRACK_CALC, szTrack, sizeof(szTrack));
			//		
			//		float fTotal = ksd.GetKSynthTrackLength(szTrack);

			//		char szFloat[64];
			//		sprintf(szFloat, "%.3f", fTotal);
			//		SetDlgItemText(hWnd, IDC_KS_ANSWER, szFloat);

			//	//}
			//	//}else{
			//	//	char szSec[64];
			//	//	GetDlgItemText(hWnd, IDC_KS_ANSWER, szSec, sizeof(szSec));

			//	//	float fSceneTime = StrToFloat(szSec);
			//	//	fSceneTime = kabs(fSceneTime);
			//	//	fSceneTime *= 1000.0f;//sec->msec
			//	//	

			//	//}
			//	break;
		 //  }
			case IDC_KS_DOCALC_AND_SET:
			{
				long sn = GetSelectedScene();
				if(sn!=-1){
					if(isPlaying){
					}else{
						char szTrack[512];
						GetDlgItemText(hWnd, IDC_KS_TRACK_CALC, szTrack, sizeof(szTrack));
						
						float fTotal = demo.ksl->GetKSynthTrackLength(szTrack);

						char szFloat[64];
						sprintf(szFloat, "%.3f", fTotal);
						SetDlgItemText(hWnd, IDC_KS_ANSWER, szFloat);
						
						demo.scene[sn].scene_track = szTrack;

						PushButton(hWnd, IDC_TIME_SET);//ボタンを押す
					}
				}
			}
			//case IDC_KS_LOAD_H:
			//{
			//	char* szFile = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
			//	if(GetOpenFileNameSingle(hWnd, "h", szFile, FALSE))
			//	{
			//		char szTrackString[512];
			//		ReadKSynthHeader(szFile, that->dwKSMusicTime, szTrackString);
			//		that->szTrack = szTrackString;
			//		SetDlgItemText(hWnd, IDC_KS_TRACKINFO, that->szTrack.c_str());
			//	}
			//	GlobalFree(szFile);

			//	int i;
			//	int tlen = lstrlen(that->szTrack.c_str())+1;
			//	float* fTotal = &that->fTotalMusicTimeSec;
			//	*fTotal= 0;
			//	//DWORD dwHoge = 0;
			//	for(i=0; i<tlen; i++){
			//		*fTotal += that->dwKSMusicTime[that->szTrack.c_str()[i]]/1000.0f;//msec->sec;
			//		//dwHoge += that->dwKSMusicTime[that->szTrack.c_str()[i]];
			//	}

			//	char szFloat[64];
			//	sprintf(szFloat, "%.3f", *fTotal);
			//	SetDlgItemText(hWnd, IDC_TIME_TOTAL_MUSIC, szFloat);

			//	//char sza[64];
			//	//sprintf(sza, "%d", dwHoge);
			//	//MessageBox(NULL, sza, 0, 0);
			//}break;
			}
		}
	}
	return FALSE;
}
DWORD CKSynthWindow::GetMusicTime(char cMusic){
	return demo.ksl->GetKSynthCharToTime(cMusic);//this->dwKSMusicTime[cMusic];
}
void CKSynthWindow::CalcAndShowTrackString(){
	//demo.ReadyDemo();
	//long sn = GetSelectedScene();
	//if(sn!=-1){
	//	char szTimeTrack[1024*100];
	//	ksd.GetKSynthTrackStringFromSec(sn==0 ? 0 : demo.scene_endtime[sn-1]/1000.0f, demo.scene[sn].fscene_time/1000.0f, szTimeTrack, sizeof(szTimeTrack));//sec->msec
	//	SetDlgItemText(hDialog, IDC_KS_TRACK_CALC, szTimeTrack);
	//}
}
void CKSynthWindow::CalcAndShowLeftTime(){
	this->CalcAndShowTotalSceneTime();
	char szTimeLeft[64];
	if( byMusicType==MUSIC_KS ){
		sprintf(szTimeLeft, "%.3f", demo.ksl->GetSoundTime() - this->fTotalSceneSec/1000.0f);
	}else if( byMusicType==MUSIC_CS ){
		sprintf(szTimeLeft, "%.3f", demo.cs->GetDuration()/1000.0f - this->fTotalSceneSec/1000.0f);
	}
	SetDlgItemText(hDialog, IDC_TIME_LEFT, szTimeLeft);
}
void CKSynthWindow::CalcAndShowTotalSceneTime(){
	char szFloatTime[64];
	int i;
	float total_time = 0.0f;
	long num = demo.scene.size();//demo.scene.end() - demo.scene.begin();//demo.scene.size();
	for(i=0; i<num; i++){
		total_time += demo.scene[i].fscene_time;
	}
	sprintf(szFloatTime, "%.3f", total_time/1000.00f);//msec->sec
	SetDlgItemText(hDialog, IDC_TIME_TOTAL, szFloatTime);
	this->fTotalSceneSec = total_time;
}
void CKSynthWindow::CalcAndShowTotalMusicTime(){
	if( byMusicType==MUSIC_KS ){
		this->fTotalMusicTimeSec = demo.ksl->GetSoundTime();
	}else if( byMusicType==MUSIC_CS ){
		this->fTotalMusicTimeSec = demo.cs->GetDuration()/1000.0f;
	}
	char szFloat[64];
	sprintf(szFloat, "%.3f", fTotalMusicTimeSec);
	SetDlgItemText(hDialog, IDC_TIME_TOTAL_MUSIC, szFloat);
}
void CKSynthWindow::ShowSceneTime(){
	long sn = GetSelectedScene();
	if(sn!=-1){
		char szFloat[64];
		sprintf(szFloat, "%.3f", demo.scene[sn].fscene_time/1000.0f);
		SetDlgItemText(hDialog, IDC_KS_ANSWER, szFloat);
		SetDlgItemText(hDialog, IDC_TIME_CURRENT, szFloat);
	}
}
void CKSynthWindow::ShowSceneTrack(){
	long sn = GetSelectedScene();
	if(sn!=-1){
		SetDlgItemText(hDialog, IDC_KS_TRACK_CALC, demo.scene[sn].scene_track.c_str());
		SetDlgItemText(hDialog, IDC_TIME_SCENE_TRACK, demo.scene[sn].scene_track.c_str());
	}
}
void CKSynthWindow::ShowTrackString(){
	char str[1024] = {'\0'};
	demo.ksl->GetKSynthTrackList(str, sizeof(str));
	szTrack = str;
	SetDlgItemText(hDialog, IDC_KS_TRACKINFO, szTrack.c_str());
}
void CKSynthWindow::Refresh(){
#ifdef NOT64K
	byMusicType = demo.ksl->GetKSynthPtr() ? MUSIC_KS : demo.cs->IsLoaded() ? MUSIC_CS : MUSIC_NONE;
#else
	byMusicType = demo.ksl->GetKSynthPtr() ? MUSIC_KS : MUSIC_NONE;
#endif
	if( byMusicType==MUSIC_KS ){
		//CheckDlgButton(hDialog, IDC_KS_KSTOSEC, BST_CHECKED);
		////CheckDlgButton(hDialog, IDC_KS_KSTOSEC, BST_CHECKED);
		SetDlgItemText(hDialog, IDC_KS_TRACK_CALC, "");
		SetDlgItemText(hDialog, IDC_KS_ANSWER, "");

		this->ShowSceneTime();
		//this->CalcAndShowTotalSceneTime();
		this->CalcAndShowTotalMusicTime();
		this->CalcAndShowLeftTime();
		this->ShowSceneTrack();
		this->ShowTrackString();
	}else if( byMusicType==MUSIC_CS ){
		//CheckDlgButton(hDialog, IDC_KS_KSTOSEC, BST_CHECKED);
		////CheckDlgButton(hDialog, IDC_KS_KSTOSEC, BST_CHECKED);
		SetDlgItemText(hDialog, IDC_KS_TRACK_CALC, "");
		SetDlgItemText(hDialog, IDC_KS_ANSWER, "");

		this->ShowSceneTime();
		//this->CalcAndShowTotalSceneTime();
		this->CalcAndShowTotalMusicTime();
		this->CalcAndShowLeftTime();
		this->ShowSceneTrack();
		this->ShowTrackString();
	}
}


//
//SoundWindow
//

void CSoundWindow::EnableControl(BOOL isEnable){
	if( isEnable ){
		//activate contorls
		//if( byMusicType==MUSIC_KS ){
		//	EnableWindow(GetDlgItem(this->hDialog, IDC_SOUND_VOLUME_SLIDER), TRUE);
		//}else if( byMusicType==MUSIC_CS ){
		//	EnableWindow(GetDlgItem(this->hDialog, IDC_SOUND_VOLUME_SLIDER), FALSE);
		//}
		EnableWindow(GetDlgItem(this->hDialog, IDC_SOUND_LOAD), TRUE);
		EnableWindow(GetDlgItem(this->hDialog, IDC_SOUND_MUTE_CHECK), TRUE);
	}else{
		//if( byMusicType==MUSIC_KS ){
		//	EnableWindow(GetDlgItem(this->hDialog, IDC_SOUND_VOLUME_SLIDER), TRUE);//再生中でもボリュームコントロールはできる
		//}else if( byMusicType==MUSIC_CS ){
		//	EnableWindow(GetDlgItem(this->hDialog, IDC_SOUND_VOLUME_SLIDER), FALSE);
		//}
		EnableWindow(GetDlgItem(this->hDialog, IDC_SOUND_LOAD), FALSE);
		EnableWindow(GetDlgItem(this->hDialog, IDC_SOUND_MUTE_CHECK), FALSE);	
	}
}
void CSoundWindow::CreateSoundWindow(int x, int y, int width, int height, const TCHAR* szTitle, HWND hParentWnd){
	m_hParentWnd = hParentWnd;
	hDialog = CreateDialogA(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_SOUND_WINDOW), NULL, (DLGPROC)CSoundWindow::DlgProc);
	ShowWindow(hDialog, TRUE);
	//UpdateWindow(hDialog);
	m_hWnd = hDialog;
	SetProp(hDialog, szCSoundWindow, this);

	//if(CSound::Initialize(hDialog)==false){
	//	MessageBox(0, "DirectSound init error!!", 0, MB_OK);
	//	return;
	//}
}
LRESULT CALLBACK CSoundWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	return DefWindowProc(hWnd, msg, wParam, lParam);
};
LRESULT CALLBACK CSoundWindow::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	CSoundWindow* that = (CSoundWindow*)GetProp(hWnd, szCSoundWindow);
	switch(msg)
	{
		case WM_HSCROLL:
		{
			char szBuf[64];
			HWND hSlider = GetDlgItem(hWnd, IDC_SOUND_VOLUME_SLIDER);
			int nPos = (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);

			float fPos = (nPos/1000.0f);
			if( byMusicType==MUSIC_KS ){
				KSynth* ks = demo.ksl->GetKSynthPtr();
				ks->vol = fPos;
			}else if( byMusicType==MUSIC_CS ){
				if( fPos > 1.0f ){
					fPos = 1.0f;
				}
				demo.cs->SetVolume(CS_VOLUME_FIX_SET( -(CS_VOLUME_RANGE - fPos*CS_VOLUME_RANGE) ));
			}

			//sprintf(szBuf, "%d", CS_VOLUME_FIX_SET( -(CS_VOLUME_RANGE - fPos*CS_VOLUME_RANGE) ));
			sprintf(szBuf, "%.4f", fPos);
			SetDlgItemText(hWnd, IDC_SOUND_VOLUME_EDIT, szBuf);
			break;
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			//case IDC_EXPORT_SCENETIIME:
			//{
			//	char* szFile = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
			//	if(GetSaveFileNameSingle(that->m_hWnd, "h", szFile, FALSE))
			//	{
			//		demo.ReadyDemo();
			//		FILE* fp = fopen(szFile, "wt");
			//			fprintf(fp, "/* System K SceneTime defnition file. */\n");
			//			fprintf(fp, "/* Exported by Oxygenz. */\n");
			//			fprintf(fp, "/* Ver = 0.1 */\n");
			//			fprintf(fp, "\n\n");
			//			fprintf(fp, "#define SCENE_MAX\t%d+1\n", demo.scene.size());
			//			fprintf(fp, "DWORD SCENE[SCENE_MAX];\n");
			//			fprintf(fp, "\n\n");
			//			fprintf(fp, "void demoInit()\n");
			//			fprintf(fp, "{\n");
			//			fprintf(fp, "\tSCENE[%d] = %d;\n", 0, 0);
			//			unsigned int i;
			//			for(i=0; i<demo.scene.size(); i++){
			//				if(i==0){
			//					fprintf(fp, "\tSCENE[%d] = %d;\n", i+1, (DWORD)demo.scene[i].fscene_time);
			//				}else{
			//					fprintf(fp, "\tSCENE[%d] = SCENE[%d] + %d;\n", i+1, i+1-1, (DWORD)demo.scene[i].fscene_time);
			//				}
			//				//fprintf(fp, "\tSCENE[%d] = %d;", i, demo.scene_endtime[i]);
			//			}
			//			fprintf(fp, "\n");
			//			fprintf(fp, "}\n");
			//		fclose(fp);
			//	}
			//	GlobalFree(szFile);
			//	break;
			//}
			case IDC_SOUND_LOAD:{
				char* szFile = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024);
				if(GetOpenFileNameSingle(that->m_hWnd, "*", szFile, FALSE))
				{
					if(demo.LoadMusic(szFile)){	
						vSynth.Refresh();
						if( demo.ksl->GetKSynthPtr() ){
							that->m_dwMusicLength = demo.ksl->GetSoundTime()*1000.0f;
							byMusicType = MUSIC_KS;
						}
#ifdef NOT64K
						else{
							that->m_dwMusicLength = demo.cs->GetDuration();
							byMusicType = MUSIC_CS;
						}
#endif						
						vSound.Refresh();
					}else{
						/* ここで状態変更しなくてもいいでしょ。すでにロードされてるのがだめになっちゃうし。 */
						//byMusicType = MUSIC_NONE;
						//that->m_dwMusicLength = 0;
						MessageBox(NULL, "COULDN'T OPEN A MUSIC FILE.", 0, MB_SYSTEMMODAL);
					}
				}
				GlobalFree(szFile);
			}break;
			}
		}
	}
	return FALSE;
}

void CSoundWindow::Refresh(){
	CheckDlgButton(hDialog, IDC_SOUND_KS1_MODE, BST_UNCHECKED);
	HWND hSlider = GetDlgItem(hDialog, IDC_SOUND_VOLUME_SLIDER);

	SetDlgItemText(hDialog, IDC_SOUND_FILENAME, demo.music_file.c_str());
	float fVol = 0.0f;
	this->m_dwMusicLength = 0;

	if( byMusicType==MUSIC_KS ){
		fVol = demo.ksl->GetKSynthPtr()->vol;
		this->m_dwMusicLength = demo.ksl->GetSoundTime()*1000.0f;
		SendMessage(hSlider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 2000));
	}else if( byMusicType==MUSIC_CS ){
		fVol = (CS_VOLUME_RANGE + demo.cs->GetVolume())/(float)CS_VOLUME_RANGE;
		this->m_dwMusicLength = demo.cs->GetDuration();
		SendMessage(hSlider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 1000));
	}//else
	{
		char szBuf[64];
		sprintf(szBuf, "%.4f", fVol);
		SetDlgItemText(hDialog, IDC_SOUND_VOLUME_EDIT, szBuf);

		SendMessage(GetDlgItem(hDialog, IDC_SOUND_VOLUME_SLIDER), TBM_SETPOS, TRUE, fVol*1000.0f);
	}
}

void CSoundWindow::Play(float StartSec){
	if(StartSec>m_dwMusicLength){
		return;
	}else if(StartSec<0.0f){
		StartSec = 0.0f;
	}
	if(!this->isMuteChecked()){
		if( byMusicType==MUSIC_KS ){
			KSynth* ks = demo.ksl->GetKSynthPtr();
			//ks->Init();
			ks->seektrack = 0;
			ks->seektime = 0;
			ks->mode = 0;
			ks->ks1mode = 0;
			//ks->ks1mode = isKS1ModeChecked();
			ks->SetSeek(StartSec);
			ks->Play();
		}else if( byMusicType==MUSIC_CS ){
			demo.cs->Play(StartSec);
		}
	}
}
void CSoundWindow::Stop(){
	if( byMusicType==MUSIC_KS ){
		KSynth* ks = demo.ksl->GetKSynthPtr();
		ks->seektrack = 0;
		ks->seektime = 0;
		ks->mode=0;
	}else if( byMusicType==MUSIC_CS ){
		demo.cs->Stop();
		demo.cs->Reset();
	}
}


////
////Util
////
long RemoveCharFromString(char* szSrc, char Target, char Exception)
{
	char* szRemoveCommentBuffer = (char*)GlobalAlloc(GPTR, sizeof(char) * (lstrlen(szSrc) + 1));
//	ZeroMemory(szRemoveCommentBuffer, GlobalSize(szRemoveCommentBuffer));
	int len = lstrlen(szSrc)+1;
	int diff=0;
	bool isException = false;
	//char* szRemoveCommentBuffer = (char*)GlobalAlloc(GPTR, sizeof(char)*len);
	////lstrcpy(szRemoveCommentBuffer, szSrc);

	for(int i=0; i+diff<len; i++)
	{
		if(Exception>0 && szSrc[i+diff]==Exception){
			isException = !isException;
			if(szSrc[(i+diff<0) ? 0:i+diff-1]=='\\' && szSrc[(i+diff<0) ? 0:i+diff-2]!='\\' && !isException) isException = !isException; 
		}
		if(!isException)
		{
			for(bool isEnd = false; isEnd != true; )
			{
				if(szSrc[i+diff]==Target)
				{
					diff+=1;
				}else isEnd=true;
			}
		}
		szRemoveCommentBuffer[i] = szSrc[i+diff];
	}
	lstrcpy(szSrc, szRemoveCommentBuffer);
	GlobalFree(szRemoveCommentBuffer);
	return diff;
}
//
//long ReplaceCharInString(char* szSrc, char cSearch, char cReplace){
//	long tlen = lstrlen(szSrc) + 1;
//	int i, nRep = 0;
//	for(i=0; i<tlen; i++){
//		if(szSrc[i]==cSearch){
//			szSrc[i] = cReplace;
//			nRep++;
//		}
//	}
//	return nRep;
//}
//
//long ReplaceCRLFWithChar(char* szSrc, char crep){
//	long diff = 0;
//	diff += RemoveCharFromString(szSrc, '\n', 0);
//	diff += ReplaceCharInString(szSrc, '\r', crep);
//	//char* szRemoveCommentBuffer = (char*)GlobalAlloc(GPTR, sizeof(char) * 1024*64);
//	////ZeroMemory(szRemoveCommentBuffer, GlobalSize(szRemoveCommentBuffer));
//	//int len = lstrlen(szSrc)+1;
//	//int diff=0;
//
//	//for(int i=0; i+diff<len; i++)
//	//{
//	//	for(bool isEnd = false; isEnd != true; )//改行を検出
//	//	{
//	//		if(szSrc[i+diff+0]=='\r' && szSrc[i+diff+1]=='\n'){//CRLF
//	//			 diff+=2;
//	//		}else if(szSrc[i+diff]=='\n' && szSrc[(i<0) ? 0:i-1+diff]!='\r'){//LF
//	//			 diff+=1;
//	//		}else isEnd=true;
//	//	}
//	//	szRemoveCommentBuffer[i] = szSrc[i+diff];
//	//}
//	//lstrcpy(szSrc, szRemoveCommentBuffer);
//	//
//	//GlobalFree(szRemoveCommentBuffer);
//	return diff;
//}
//
//BOOL GetNextToken(char* szBuffer, char* szRecv, int size, int* pointer){
//	int tlen = lstrlen(szBuffer)+1;
//	int i, j;
//	for(i= *pointer, j=0; i<tlen; i++, j++){
//		if(	szBuffer[i]==' '
//			||
//			szBuffer[i]=='	'
//			||
//			szBuffer[i]=='\n'//LF単体の場合を想定。CRLFは想定外
//			||
//			//szBuffer[i]=='\r'//CR単体の場合を想定。CRLFは想定外
//			//||
//			szBuffer[i]=='\0'
//			||
//			szBuffer[i]=='"'
//			||
//			szBuffer[i]==','
//			)
//		{
//			szRecv[j] = '\0';
//			*pointer = i+1;
//			break;
//		}else{
//			if(i>size-1){
//				return FALSE;
//			}else{
//				szRecv[j] = szBuffer[i];
//			}
//		}
//		*pointer = i;
//	}
//	//*pointer = lstrlen(szBuffer);
//	return TRUE;
//}
//
////
////KSynth2が吐くヘッダを開いて、トラックとMUSIC_TIME_Xの数値を読みとる
////DWORD* dwTimeListは256以上の要素がないといけない.
////
//BOOL ReadKSynthHeader(const char* szFileName, DWORD dwKSMusicTime[256], char* szTrackString){
//	FILE* fp = fopen(szFileName, "rt");
//	if(!fp) return FALSE;
//	
//	char* szBuffer = NULL;
//	fseek(fp, 0, SEEK_END);
//	long fsize = ftell(fp);
//	szBuffer = (char*)GlobalAlloc(GPTR, sizeof(char) * fsize);
//	fseek(fp, 0, SEEK_SET);
//	fread(szBuffer, sizeof(char), fsize/sizeof(char), fp);
//	fclose(fp);
//
//	//ヘッダチェック
//	const char* szHeader[] = {
//		"/*\n	KSynth2 music data by KSynth2 c/cpp header Exporter ver 2.20\n	coded by kioku (Cyber K 2004)\n*/",
//		"/*\r\n	KSynth2 music data by KSynth2 c/cpp header Exporter ver 2.20\r\n	coded by kioku (Cyber K 2004)\r\n*/"
//	};
//	int i;
//	BOOL isCorrect = FALSE;
//	for(i=0; i<sizeof(szHeader)/sizeof(char*); i++){
//		if(0==strncmp(szHeader[i], szBuffer, lstrlen(szHeader[i]))){
//			isCorrect = TRUE;
//		}
//	}
//	if(!isCorrect){
//		GlobalFree(szBuffer);
//		return FALSE;
//	}
//
//
//	//データ読み取り
//	char* szSearched = (char*)GlobalAlloc(GPTR, GlobalSize(szBuffer));
//	lstrcpy(szSearched, szBuffer);
//
//	//ReplaceCRLFWithChar(szSearched, ' ');
//	RemoveCharFromString(szSearched, '\r', 0);//CRLF改行コードを使用している場合に備える
//	
//
//	char szMusicTimeId[256][32];	//MUSIC_TIME_?のリストを作る
//	int k;
//	for(k=0; k<256; k++){//size of ASCII
//		wsprintf(szMusicTimeId[k], "MUSIC_TIME_%c", k);
//	}
//
//	const char* szKSTrackId = "ks->SetTrack(";
//	//char* szTrackString = NULL;
//	//DWORD dwKSMusicTime[256] = { 0 };
//
//	int p = 0;
//	while(szSearched[p]!='\0'){
//		char token[1024*64];
//		if(GetNextToken(szSearched, token, sizeof(token), &p)){
//			for(k=0; k<256; k++){//MUSIC_TIME_?
//				if(0==lstrcmp(token, szMusicTimeId[k])){
//					do{
//						if(GetNextToken(szSearched, token, sizeof(token), &p)){
//							//nop.
//						}else{
//							MessageBox(NULL, "GetNextToken failed.", 0, MB_OK);
//						}
//					}while(token[0]=='\0');
//                    dwKSMusicTime[k] = StrToInt(token);
//					break;
//				}
//			}
//			if(0==lstrcmp(token, szKSTrackId)){//SetTrack
//				do{
//					if(GetNextToken(szSearched, token, sizeof(token), &p)){
//						//nop.
//					}else{
//						MessageBox(NULL, "GetNextToken failed.", 0, MB_OK);
//					}
//				}while(token[0]=='\0');
//				//szTrackString = (char*)GlobalAlloc(GPTR, (lstrlen(token)+1)*sizeof(char));
//				lstrcpy(szTrackString, token);
//			}
//		}else{
//			MessageBox(NULL, "GetNextToken failed.", 0, MB_OK);
//		}
//	}
//	//GlobalFree(szTrackString);
//
//	GlobalFree(szSearched);
//	GlobalFree(szBuffer);
//	return TRUE;
//}

float StrToFloat(const char* szFloat){
	char* pRead = (char*)szFloat;
	if(!pRead || pRead[0]=='\0') return 0.0f;

	//整数部分の切り出し
	int iSection = StrToInt(szFloat);
	float ans = (float)iSection;
	unsigned short sign = 1;

	if(pRead[0] == '-'){
		pRead++;
		sign=-1;
	}
	while(*pRead != '.'){
		if('0' > *pRead || *pRead > '9')	return ans;
		//if('0' >= *pRead || *pRead >= '9')	return ans;
		pRead++;
	}
	float dig = 10.0f;
	while(dig<=100000000.0f){
		pRead++;
		if('0' <= *pRead && *pRead <= '9'){
			if(sign==1)
				ans += (*pRead - '0')/dig;
			else
				ans -= (*pRead - '0')/dig;
			dig*=10.0f;
		}else{
			break;
		}
	}
	return ans;
}

void PushButton(HWND hWnd, int nId){
	SendMessage(hWnd, WM_COMMAND, nId, 0);
}



//
//KSynthLoader
//
#include <stdio.h>
#define KSynthLoaderHEADER        "KS2DATA"
#define NOW_VER                  220

int KSynthLoader::GetTNum(char ch)
{
    int num=-1;
     if((ch>='A')&&(ch<='~')){
         num=ch-'A';//set track
     }
     if(num==-1){
        //char s[2];
        //s[0]=ch; s[1]='\0';
        //AnsiString err = "Num Error! >>";
        //err+=s;
        //ShowMessage(err);
     }
     return num;
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

BOOL KSynthLoader::LoadKSF(const char * filename, char* szTrackBufRecv, int bufsize)
{
	const int buffer_size = 1024*64*2;

    FILE* fp;
	{
		if((fp=fopen(filename,"rt"))==NULL){
		   // ShowMessage("can't open data!");
			return FALSE;
		}
		char buf[buffer_size] = {'\0'};
		fscanf(fp,"%s",buf);//header,ver
		if(lstrcmp(buf, KSynthLoaderHEADER)!=0){//check for KS header
			//ShowMessage("data error");
			fclose(fp);
			return FALSE;
		}
	}

	fseek(fp,0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* szModBuffer = (char*)GlobalAlloc(GPTR, sizeof(char) * fsize);// = { '\0' };
//		while(!IsEof(fp)){
		fread(szModBuffer,sizeof(char),fsize,fp);//track
	fclose(fp);
	if((fp=fopen(filename,"wt"))==NULL){
		return FALSE;
	}
//		}
		RemoveCharFromString(szModBuffer, '\r', 0);
		long newfsize = lstrlen(szModBuffer) + 1;
		fseek(fp, 0, SEEK_SET);
		fwrite(szModBuffer, sizeof(char), newfsize, fp);
	fclose(fp);
	GlobalFree(szModBuffer);

		
	if((fp=fopen(filename,"rt"))==NULL){
       // ShowMessage("can't open data!");
		return FALSE;
    }
	char buf[buffer_size] = {'\0'};
    fscanf(fp,"%s",buf);//header,ver
    if(lstrcmp(buf, KSynthLoaderHEADER)!=0){//check for KS header
        //ShowMessage("data error");
        fclose(fp);
		return FALSE;
    }

    fscanf(fp, "%s", buf);//ver
    long kver=atol(buf);
    if(kver>NOW_VER){
        //ShowMessage("This file is latest version. You must get latest KSynth");
        fclose(fp);
		return FALSE;
    }

    fscanf(fp,"%s",buf);//osc_num
    if(lstrcmp(buf,"INST")==0){//Inst trap
        //ShowMessage("This file is Instrument file. Check this file");
        fclose(fp);
		return FALSE;
    }

    int osc_n = atol(buf);
    if(osc_n>OSC_SLOT_MAX){
        //ShowMessage("OSC_SLOT is over max. KSynth Version missmatch!");
        fclose(fp);
		return FALSE;
    }

	this->pKs = new KSynth();
	KSynth* ks = pKs;
	ks->Init();
	try{
		int i;
		KOsc ko[OSC_SLOT_MAX];
		float fdelaytime = 0.0f;
		for(i=0; i<osc_n; i++){
			//ReadInst(p,Mixer1,i,fp);
			int tint=0;
			float temp=0.0f;
			fscanf(fp,"%f%f%f%d",&(ko[i].fmfrq),&(ko[i].fmendfrq),&(ko[i].fmamp),&tint);
			ko[i].fmtype = (unsigned char)tint;
			
			fscanf(fp,"%f%f%f%d",&(ko[i].wavefrq),&(ko[i].waveendfrq),&(ko[i].waveamp),&tint);
			ko[i].wavetype = (unsigned char)tint;
			
			fscanf(fp,"%d%f%f%f",&tint,&(ko[i].attacktime),&(ko[i].releaselevel),&(ko[i].releasetime));
			ko[i].algorithm = (unsigned char)tint;
			
			fscanf(fp,"%f%f%f%f",&(ko[i].delaylevel),&fdelaytime/*&(ko[i].delaytime)*/, &(ko[i].conductance), &(ko[i].inductance));
			
			fscanf(fp,"%f%f%f%f",&(ko[i].hipass), &(ko[i].wavefine), &(ko[i].fmdetune), &temp);//reserved
			ko[i].instvol = (127.0-temp)/127.0f;

			ks->Osc[i].SetOsc(ko[i].algorithm, ko[i].wavetype, ko[i].waveamp, ko[i].wavefrq, ko[i].waveendfrq, ko[i].wavefine, ko[i].fmtype, ko[i].fmamp, ko[i].fmfrq, ko[i].fmendfrq, ko[i].fmdetune, ko[i].attacktime, ko[i].releaselevel, ko[i].releasetime, ko[i].instvol);
			ks->Osc[i].SetFilter(fdelaytime/*ko[i].delaytime*/, ko[i].delaylevel, ko[i].conductance, ko[i].hipass, ko[i].inductance);
			//m->sInstVol[i]->Position = (long)temp;
		}
		
		fscanf(fp,"%s",buf);//track
		if(kver>=220){//トラックが2つ以上あるので最初のひとつ以外は読み飛ばし
			char* szThrow = (char*)GlobalAlloc(GPTR, sizeof(buf));
				fscanf(fp,"%s",szThrow);//読み捨て
				fscanf(fp,"%s",szThrow);//読み捨て
			GlobalFree(szThrow);
		}

		RemoveCharFromString(buf, '@', 0);
		if(bufsize>lstrlen(buf)+1){
			lstrcpy(szTrackBufRecv, buf);
		}
		this->m_szTrackString = buf;
		ks->SetTrack(buf);
		ks->ks1mode = 0;
		ks->seektime = 0;
		ks->seektrack = 0;
		ks->vol = 0.4000000f;

		int f=0;
		while(feof(fp)==0){
			fread(&buf[f],1,1,fp);//track
			f++;
		}
		fclose(fp);
		buf[f-1]='\0';

		char szBuffer[buffer_size], szSequence[buffer_size] = {'\0'};
		lstrcpy(szBuffer, buf);

		int j;
		int snum=0;
		int tracknum=0;
		int tlen = lstrlen(szBuffer)+1;
		for(i=0; szBuffer[i]!='\0'; i++){
			if(szBuffer[i]=='\n'){//seq.IsDelimiter("\r",i)){//separate
				FillMemory(szSequence, sizeof(szSequence), '\0');
				for(j=0; j<i; j++){//SubString(1,i)
					szSequence[j] = szBuffer[j+1];
				}

				char szTempBuffer[buffer_size] = {'\0'};
				int lim = lstrlen(szBuffer)+1;
				for(j=0; j<lim-i; j++){//
					szTempBuffer[j] = szBuffer[j+i];
				}
				CopyMemory(szBuffer, szTempBuffer, sizeof(szTempBuffer));

				//seq.Delete(1,i+1);//+1 is delete "\n"
				char cbuf[buffer_size] = {'\0'};
				lstrcpy(cbuf,szSequence);
				if(cbuf[0]=='#'){//Track
					tracknum=GetTNum(cbuf[1]);//set track
					if(tracknum==-1){
						throw "error";
					}
					int s=0;
					char bps[16] = {'\0'};
					while((cbuf[2+s]>='0')&&('9'>=cbuf[2+s])||(' '==cbuf[2+s])){
						bps[s]=cbuf[2+s];
						s++;
					}
					if(s==0){//error
						throw "error";
					}
					float bpm=(float)(atol(bps));
					if(tracknum=='J'){
						tracknum='J';
					}
					if(bpm!=0) ks->charTime[tracknum]=60.0f/bpm*44100.0f/2.0f;
					i=0;//reset
				}else if(cbuf[0]=='c'){//channel
					int chnum, instnum;
					chnum=GetTNum(cbuf[1]);//set channel
					if(chnum==-1){
						throw "error";
					}
					instnum=GetTNum(cbuf[3]);//set inst
					if(instnum==-1){
						throw "error";
					}
					cbuf[lstrlen(cbuf)-1]=(cbuf[lstrlen(cbuf)-1]=='\n' || cbuf[lstrlen(cbuf)-1]==';') ? '\0' : cbuf[lstrlen(cbuf)-1];//delete';'
					ks->SetSequence(tracknum,chnum,instnum,&cbuf[5]);
					snum++;
					i=0;//reset
				}else if(cbuf[0]=='/'){//comment
					i=0;
				}
			}
		}
	}
	catch (char* str){
		if(str!=NULL){
			delete pKs;
			pKs = NULL;
			return FALSE;
		}
	}
	this->BuildKSynthTimeBuffer();
    return TRUE;
}