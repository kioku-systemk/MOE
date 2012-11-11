//##############################################//
//###		DirectSound Streaming Library	 ###//
//###		Code : corvette.raze.v(x*x)      ###//
//##############################################//
/*
	version:
		0.01 - for sysk ver.(unused)
		0.10 - 位置指定に対応. 取得は勘弁して・・・
				
	date:
		05/31/2005
*/
#ifndef CSOUNDSTREAM_H
#define CSOUNDSTREAM_H

#include "CSound.h"
#define SAFE_FREE(a) { if(a){ free(a); a=NULL; } }

class CSoundStream : public CSound
{
public:
	CSoundStream();
	virtual ~CSoundStream();
public:
	void Play(DWORD position_ms = 0, bool isLoop = false);
	bool LoadWaveFromFile(const char* szFile);
	bool LoadWaveFromMemory(){ return false; }

protected:
	//UnInitializeで.
	bool m_isDestruct;
	HANDLE* m_pNotifyHandle;
	IDirectSoundNotify* m_pSoundNotify;
	
	HANDLE m_hThread;
	DSBPOSITIONNOTIFY* m_pDsbnotify;
	DWORD m_dwBufferLengthSec;

	DWORD m_dwPos_byte;
	DWORD m_dwPos_ms;
protected:
	void InitThread(DWORD dwStartPos_ms);
	//管理スレッド
	static DWORD WINAPI StreamThread(LPVOID CSSPtr);
};
#endif