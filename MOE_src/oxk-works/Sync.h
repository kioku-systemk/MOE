#ifndef SYNC_H
#define SYNC_H
#include <windows.h>
#include "./common.h"
#include <iostream>
#include "../klib/ks2lib.h"

float StrToFloat(const char* szFloat);
BOOL ReadKSynthHeader(const char* szFileName, DWORD dwKSMusicTime[256], char* szTrackString);
void PushButton(HWND hWnd, int nId);
BOOL LoadKSF(const char * filename, KSynth* ks, char* szRecvBuf, int bufsize);
BOOL IsNumeric(char cChar, BOOL isFloating);

#define szCKSynthWindow  "CKSynthWindowPtr"
#define szCTimeWindow	 "CTimeWindowPtr"
#define szCSoundWindow	 "CSoundWindowPtr"

class CKSynthWindow : public CWindowBase{
private:
	HWND m_hWnd;
	HWND m_hParentWnd;
	HWND m_hFileName;
	HWND m_hStaticTitle;
	HWND m_hStaticLength;
	HWND m_hLoadButton;

	HWND hDialog;

	float fTotalMusicTimeSec;
	float fTotalSceneSec;
	std::string szTrack;
	DWORD dwKSMusicTime[256];
public:
	CKSynthWindow(){
		this->Clear();
	};
	virtual ~CKSynthWindow(){};
public:
	void Hide(){ RemoveProp(hDialog, szCKSynthWindow); DestroyWindow(hDialog); hDialog = NULL; m_hWnd = NULL; }
	void Show(){ if(hDialog) return; this->CreateSynthWindow(0, 0, 0, 0, "", m_hParentWnd); }
	HWND GetHolderWnd(){ return hDialog; }
	void CreateSynthWindow(int x, int y, int width, int height, const TCHAR* szTitle, HWND hParentWnd);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void SetTrackString(const char* szTrackString){
		szTrack = szTrackString;
	}
	const char* GetTrackString() {return szTrack.c_str();}
	DWORD GetMusicTime(char cMusic);

	void Clear(){
		fTotalMusicTimeSec = 0.0f;
		fTotalSceneSec = 0.0f;
		szTrack.clear();
		ZeroMemory(dwKSMusicTime, sizeof(dwKSMusicTime));
	}
	void Refresh();

	void CalcAndShowTotalMusicTime();
	void CalcAndShowTotalSceneTime();
	void CalcAndShowLeftTime();
	void CalcAndShowTrackString();
	void ShowSceneTime();
	void ShowSceneTrack();
	void ShowTrackString();
	void EnableControl(BOOL isEnable);
};

class CSoundWindow : public CWindowBase{
private:
	HWND m_hWnd;
	HWND m_hParentWnd;
	HWND m_hFileName;
	HWND m_hStaticTitle;
	HWND m_hStaticLength;
	HWND m_hLoadButton;

	HWND hDialog;

	DWORD m_dwMusicLength;
public:
	CSoundWindow(){
		this->Clear();
	}
	virtual ~CSoundWindow(){
		Clear();
	};
public:
	void Hide(){ RemoveProp(hDialog, szCSoundWindow); DestroyWindow(hDialog); hDialog = NULL; m_hWnd = NULL; }
	void Show(){ if(hDialog) return; this->CreateSoundWindow(0, 0, 0, 0, "", m_hParentWnd); }
	HWND GetHolderWnd(){ return hDialog; }
	void CreateSoundWindow(int x, int y, int width, int height, const TCHAR* szTitle, HWND hParentWnd);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void Play(float StartSec);
	void Stop();
	BOOL isKS1ModeChecked(){
		return ((IsDlgButtonChecked(hDialog, IDC_SOUND_KS1_MODE)==BST_CHECKED));
	}
	BOOL isMuteChecked(){
		return ((IsDlgButtonChecked(hDialog, IDC_SOUND_MUTE_CHECK)==BST_CHECKED));
	}

	void Clear(){
		m_dwMusicLength = 0;
	}
	void Refresh();
	void EnableControl(BOOL isEnable);
};

class KSynthLoader{
private:
	KSynth* pKs;
	std::string m_szTrackString;
	DWORD dwKSMusicTime[256];
public:
	KSynthLoader(){
		ZeroMemory(dwKSMusicTime, sizeof(dwKSMusicTime));
		pKs = NULL;
		m_szTrackString.clear();
	}
	KSynthLoader(const KSynthLoader &ksl)
	{
		if(ksl.pKs){
			this->pKs = new KSynth();
			this->pKs->Init();
			CopyMemory(this->pKs, ksl.pKs, sizeof(KSynth));
			this->pKs->seektime = 0;
			this->pKs->seektrack = 0;
			this->pKs->mode = 0;
		}

		this->m_szTrackString = ksl.m_szTrackString;
		CopyMemory(this->dwKSMusicTime, ksl.dwKSMusicTime, sizeof(ksl.dwKSMusicTime));
	}
	KSynthLoader& operator=(const KSynthLoader &ksl)
	{
		if(ksl.pKs){
			this->pKs = new KSynth();
			this->pKs->Init();
			CopyMemory(this->pKs, ksl.pKs, sizeof(KSynth));
			this->pKs->seektime = 0;
			this->pKs->seektrack = 0;
			this->pKs->mode = 0;
		}
		
		this->m_szTrackString = ksl.m_szTrackString;
		CopyMemory(this->dwKSMusicTime, ksl.dwKSMusicTime, sizeof(ksl.dwKSMusicTime));
		return *this;
	}
	~KSynthLoader(){
		if(pKs){
			delete pKs;
			pKs = NULL;
		}
		m_szTrackString.clear();
	}


	int GetTNum(char ch);
	BOOL LoadKSF(const char * filename, char* szTrackBufRecv, int bufsize);

	float GetSoundTime(){//Sec
		if(!pKs) return 0.0f; 
		return this->GetKSynthTrackLength(this->m_szTrackString.c_str());
	}
	KSynth* GetKSynthPtr(){
		return pKs;
	}
	int GetKSynthTrackList(char* szRecv, size_t nBuffer){
		if(pKs==NULL) return -1;

		if(m_szTrackString.size() > nBuffer){
			return m_szTrackString.size();// - nBuffer;
		}
		
		lstrcpy(szRecv, m_szTrackString.c_str());
		return 0;
	}
	
	float GetKSynthTrackLength(const char* szTrack){//returns in SEC, '*'isAllowed
		if(pKs==NULL) return 0.0f;

		int i;
		float fTotal = 0.0f;
		int tlen = lstrlen(szTrack)+1;
		BOOL isPlus = FALSE;
		float fMultiply = 1.0f;
		char oldchar = 0;
		char c;

		for(i=0; i<tlen; i++){
			c = szTrack[i];
			if((c>='A'&&c<='~')){
			//if((c>='A'&&c<='Z')||(c>='['/*c>='^'*/&&c<='`')||(c>='a'&&c<='z')||(c>='{'&&c<='~')){
				oldchar = c;
				fTotal += this->GetKSynthCharToTime(c)*fMultiply;
				//char szbuf[64];
				//sprintf(szbuf, "%c:%.3f:%.3f\n", c, this->GetKSynthCharToTime(c), fMultiply);
				//OutputDebugString(szbuf);
			}else if(c=='*'){
				char szFloatingBuffer[1024] = {'\0'}; int p=0;
				do{
					c = szTrack[++i];
					szFloatingBuffer[p] = c; p++;
				}while(IsNumeric(c, TRUE));
				if(isPlus){
					fMultiply *= atof(szFloatingBuffer);
					isPlus = FALSE;
				}else{
					fMultiply = atof(szFloatingBuffer);
				}
				if(c=='*'){//次も掛け算だったら
					isPlus = TRUE;
				}i--;//読み飛ばしたぶんを戻す

				if(oldchar!=0 && !isPlus){
					fTotal -= this->GetKSynthCharToTime(oldchar);
					fTotal += this->GetKSynthCharToTime(oldchar)*fMultiply;
					oldchar = 0;
					fMultiply = 1.0f;
				}
			}else{
				//char szbuf[64];
				//sprintf(szbuf, "fixme:%c\n", c);
				//OutputDebugString(szbuf);
			}
		}
		//OutputDebugString("----------------\n");
		//int i;
		//float fTotal = 0.0f;
		//int tlen = lstrlen(szTrack)+1;
		//for(i=0; i<tlen; i++){
		//	fTotal += this->GetKSynthCharToTime(szTrack[i]);
		//}
		return fTotal;
	}

	BOOL GetKSynthTrackStringFromSec(const float fBeginSecIn, const float fSearchSec, char* szRecv, const int sizebuf){
		//const char* szTrack = this->m_szTrackString.c_str();

		//BOOL isFoundDat = FALSE;
		//float fBeginSec = fBeginSecIn;
		//float fCurrentSum = 0.0f;
		//float fDestinationValue = fSearchSec;//秒  scene_end - scen_end[-1] 
		//float fBeginRatio = 0.0f;
		//ZeroMemory(szRecv, sizebuf);
		//int i, j, skipchar;
		//if(fSearchSec<=0.0f){
		//	if(sizebuf<lstrlen("")){
		//		isFoundDat = FALSE;
		//	}else{
		//		lstrcpy(szRecv, "");
		//		isFoundDat = TRUE;
		//	}
		//}else{
		//	for(skipchar=0, i=0; i<(signed)this->m_szTrackString.size(); i++){
		//		fCurrentSum += this->GetKSynthCharToTime(szTrack[i]);
		//		if(fCurrentSum<fBeginSec){
		//			//fCurrentSum = 0;
		//			skipchar++;
		//			continue;
		//		}else{
		//		}
		//		//szTimeTrack[i] = szTrack[i];
		//		//lstrcpyn(szTimeTrack, szTrack, i);
		//		if(fCurrentSum - fBeginSec >= fDestinationValue){
		//			if(sizebuf<i-skipchar){
		//				isFoundDat = FALSE; break;
		//			}

		//			float fSubtract = 0.0f;
		//			for(int k=0; k<skipchar; k++){
		//				fSubtract += this->GetKSynthCharToTime(szTrack[k]);
		//			}
		//			//fBeginSec -= fSubtract;
		//			fBeginRatio = (fBeginSec - fSubtract)/this->GetKSynthCharToTime(szTrack[skipchar]);//直前の文字のRate
		//			//fBeginRatio = 1.0f - (fBeginSec - fSubtract)/this->GetKSynthCharToTime(szTrack[skipchar]);
		//			//fBeginRatio = 1.0f - (fCurrentSum - fBeginSec)/this->GetKSynthCharToTime(szTrack[skipchar]);

		//			if((fBeginRatio>0.0f) && (1.0f - fBeginRatio)*this->GetKSynthCharToTime(szTrack[skipchar])>fDestinationValue){//直前の文字時間 - 直前の文字*直前の文字のRateをもってしてもなお，fSearchSecがそれより小さい場合
		//				//直前の文字時間残り% - (今までの文字時間合計 - 直前までの時間合計 - 求めたい時間)%
		//				float fRatio = (1.0f - fBeginRatio)  - (fCurrentSum - fBeginSec - fDestinationValue)/this->GetKSynthCharToTime(szTrack[skipchar]);
		//				if(fRatio<=0.0f || fRatio==1.0f){
		//				}else{
		//					char szFormula[64];
		//					sprintf(szFormula, "%c*%.6f", szTrack[skipchar], fRatio);
		//					//sprintf(szFormula, "%c*%.6f%c*%.6f", szTrack[skipchar], fBeginRatio, szTrack[skipchar], fRatio);
		//					if(sizebuf<lstrlen(szRecv) + lstrlen(szFormula)+1){
		//						isFoundDat = FALSE;
		//						break;//exit from for-loop.
		//					}
		//					lstrcat(szRecv, szFormula);
		//				}
		//			}else{
		//				int usedbuffer = 0;
		//				int skipcharaddon = 0;
  //                      if(fBeginSec>0.0f && fBeginRatio>0.0f){
		//					char szFormula[64];
		//					sprintf(szFormula, "%c*%.6f", szTrack[skipchar], 1.0f - fBeginRatio);
		//					if(sizebuf<i-skipchar + lstrlen(szFormula)+1){
		//						isFoundDat = FALSE;
		//						break;
		//					}

		//					//char szOneMinusBeginRatio[64] = {'\0'};
		//					//sprintf(szOneMinusBeginRatio, "%c*%.6f", szTrack[skipchar], fBeginRatio);
		//					lstrcpy(szRecv, szFormula);
		//					usedbuffer = lstrlen(szRecv);
		//					skipcharaddon = 1;

		//					float fRatio = 1.0f - (fCurrentSum - fBeginSec - fDestinationValue)/this->GetKSynthCharToTime(szTrack[i]);//最後の文字は割合で表示する
		//					if(fRatio<=0.0f || fRatio==1.0f){
		//					}else{
		//						char szFormula[64];
		//						sprintf(szFormula, "%c*%.6f", szTrack[i], fRatio);
		//						if(sizebuf<lstrlen(szRecv) + lstrlen(szFormula)+1){
		//							isFoundDat = FALSE;
		//							break;//exit from for-loop.
		//						}
		//						lstrcat(szRecv, szFormula);
		//					}
		//				}else{
		//					int p = 0;
		//					for(p=usedbuffer, j=skipchar; j<i; j++, p++){
		//					//for(p=usedbuffer, j=skipchar + skipcharaddon; j<i; j++, p++){
		//						szRecv[p] = szTrack[j];
		//					}
		//					float fRatio = 1.0f - (fCurrentSum - fBeginSec - fDestinationValue)/this->GetKSynthCharToTime(szTrack[i+skipchar]);//最後の文字は割合で表示する
		//					if(fRatio<=0.0f || fRatio==1.0f){
		//					}else{
		//						char szFormula[64];
		//						sprintf(szFormula, "%c*%.6f", szTrack[i+skipchar], fRatio);
		//						if(sizebuf<lstrlen(szRecv) + lstrlen(szFormula)+1){
		//							isFoundDat = FALSE;
		//							break;//exit from for-loop.
		//						}
		//						lstrcat(szRecv, szFormula);
		//					}
		//				}
		//			}

		//			isFoundDat = TRUE;
		//			break;
		//		}
		//	}
		//	if(fCurrentSum<fDestinationValue){
		//		if(sizebuf<(signed)this->m_szTrackString.size()){
		//			isFoundDat = FALSE;
		//		}
		//		lstrcpy(szRecv, this->m_szTrackString.c_str());
		//		isFoundDat = TRUE;
		//	}
		//}
		//return isFoundDat;
		return FALSE;
	}
	
	float GetKSynthCharToTime(const char cChar){//returns in SEC
		if(pKs==NULL) return 0.0f;
		if((cChar>='A'&&cChar<='~')){
		//if((cChar>='A'&&cChar<='Z')||(cChar>='['/*cChar>='^'*/&&cChar<='`')||(cChar>='a'&&cChar<='z')||(cChar>='{'&&cChar<='~')){
			return dwKSMusicTime[cChar]/1000.0f;//msec->sec;
		}
		return 0.0f;
	}

	void BuildKSynthTimeBuffer(){
		if(pKs==NULL) return;
		int i;
        for(i=0;i<TRACK_MAX; i++){
            int minchar=1.0e+5;
            int al;
            for(al=0; al<INST_SLOT_MAX; al++){
                if(pKs->Inst[i][al].sequence!=NULL){
                    if(minchar>lstrlen(pKs->Inst[i][al].sequence)){
                        minchar = lstrlen(pKs->Inst[i][al].sequence);
						//break;
                    }
                }
            }
			if(minchar!=1.0e+5){
				dwKSMusicTime[i+'A'] = (unsigned long)(minchar*(pKs->charTime[i])/44.100f);
				//char szbuf[16] = {'\0'};
				//sprintf(szbuf, "%c:%6d\n", i+'A', dwKSMusicTime[i+'A']);
				//OutputDebugString(szbuf);
			}
		}
		//OutputDebugString("--------------------------\n");
	}
};

KSynthLoader* GetKSDPtr();

#endif