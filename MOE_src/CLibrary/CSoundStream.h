//##############################################//
//###		DirectSound Streaming Library	 ###//
//###		Code : corvette.raze.v(x*x)      ###//
//##############################################//
/*
	version:
		0.01 - for sysk ver.(unused)
		0.10 - �ʒu�w��ɑΉ�. �擾�͊��ق��āE�E�E
				
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
	//UnInitialize��.
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
	//�Ǘ��X���b�h
	static DWORD WINAPI StreamThread(LPVOID CSSPtr);
};
#endif