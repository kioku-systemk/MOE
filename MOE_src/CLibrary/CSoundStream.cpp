//v010
#include "./CSoundStream.h"

CSoundStream::CSoundStream()
{
	m_pNotifyHandle = NULL;
	m_pSoundNotify = NULL;
	m_isDestruct = false;
	m_dwBufferLengthSec = 2;
	m_hThread = NULL;
	m_pDsbnotify = NULL;
	m_dwPos_ms = m_dwPos_byte = 0;
}

CSoundStream::~CSoundStream()
{
	m_isDestruct = true;
	free(m_pDsbnotify);
	//WaitForSingleObject(m_hThread, INFINITE);
	CloseHandle(m_hThread);
	SAFE_FREE(m_pNotifyHandle);
	//SAFE_RELEASE(m_pSoundNotify);
}

void CSoundStream::InitThread(DWORD dwStartPos_ms)
{
	//2回目以降の呼び出しのために停止
	CSound::Stop();

	//一旦、スレッドを停止する
	m_isDestruct = true;
	//SetEvent(m_pNotifyHandle[0]);
	//WaitForSingleObject(m_hThread, INFINITE);
	TerminateThread(m_hThread, FALSE);

	void* pdata = NULL;
	////途中再生の位置をバイト単位で算出
	m_dwPos_byte = (DWORD)((double)dwStartPos_ms * (double)(m_wfx.nAvgBytesPerSec/1000.0));
	//サウンドポインタの位置をずらして
	if(m_dwPos_ms > 0) CSound::SetWavePointerPos(m_dwPos_byte, SEEK_SET);
	//バッファの前半をあらかじめ埋めておく
	DWORD dwWriteSize = (m_wfx.nAvgBytesPerSec * (m_dwBufferLengthSec / 2));
	//WAVEファイルから、データ部分を引っ張ってくる
	if(!(CSound::ReadWaveData(NULL, &pdata, NULL, NULL, 0, 0, dwWriteSize)))
		return;
	//データをバッファに書き込む
	if(!(CSound::WriteDataToBuffer(&m_pSecondaryBuffer, pdata, 0, dwWriteSize)))
	{
		free(pdata);
		return;
	}
	free(pdata);
	//書き込んだ分、さらにポインタを進めておく
	//if(m_dwPos_ms > 0)
	//	CSound::SetWavePointerPos(m_dwPos_byte+dwWriteSize, SEEK_SET);

	//スレッド生成
	DWORD tid;
	m_hThread = CreateThread(NULL, 0, CSoundStream::StreamThread, this, 0, &tid);
}

void CSoundStream::Play(DWORD position_ms, bool isLoop)
{
    //再生位置情報を格納
	m_dwPos_ms = position_ms;
    
	//スレッド作成
	InitThread(m_dwPos_ms);

	m_pSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
}

bool CSoundStream::LoadWaveFromFile(const char* szFile)
{
	if(!m_pPrimaryBuffer)
	{
		//プライマリバッファを取得する
		if(!GetPrimaryBuffer(&m_pPrimaryBuffer, DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_PRIMARYBUFFER))
			return false;
	}
	//前のゴミを消しておく
	SAFE_RELEASE(m_pSecondaryBuffer);
	
	DWORD dwBufferSize = 0;

	//最初のファイル読み込み
	if(!CSound::ReadWaveData(&m_wfx, NULL, &dwBufferSize, szFile))
	{
		return false;
	}

	//バッファの作成
	DSBUFFERDESC dsbdesc;
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags =	DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_CTRLPOSITIONNOTIFY|
						DSBCAPS_GLOBALFOCUS|DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME|
						DSBCAPS_CTRLFREQUENCY;
	dsbdesc.lpwfxFormat = &m_wfx;
	dsbdesc.dwBufferBytes = m_wfx.nAvgBytesPerSec * m_dwBufferLengthSec;

	if(!CSound::CreateBuffer(&m_pSecondaryBuffer, &dsbdesc, NULL))
		return false;

	//通知ハンドルの作成
	if(m_pNotifyHandle) free(m_pNotifyHandle);
	m_pNotifyHandle = (HANDLE*)malloc( 2 * sizeof(HANDLE));
	if(m_pNotifyHandle == NULL) return false;
	for(int i=0; i<2; i++)
	{
		m_pNotifyHandle[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	//通知インターフェイスの作成
	if(FAILED(m_pSecondaryBuffer->QueryInterface(IID_IDirectSoundNotify, (void**)&m_pSoundNotify))) return false;

	SAFE_FREE(m_pDsbnotify);
	m_pDsbnotify = (DSBPOSITIONNOTIFY*)malloc(2 * sizeof(DSBPOSITIONNOTIFY));
	if(m_pDsbnotify == NULL) return false;
	m_pDsbnotify[0].dwOffset		= 0;
	m_pDsbnotify[0].hEventNotify	= m_pNotifyHandle[0];
	//２秒間のバッファを用いているので、わかりやすく真ん中にしておく。
	m_pDsbnotify[1].dwOffset		= m_wfx.nAvgBytesPerSec * (m_dwBufferLengthSec / 2);
	m_pDsbnotify[1].hEventNotify	= m_pNotifyHandle[1];

	if(FAILED(m_pSoundNotify->SetNotificationPositions(2, m_pDsbnotify)))
	{
		free(m_pDsbnotify);
		return false;
	}

	SAFE_RELEASE(m_pPrimaryBuffer);
	return true;
}

DWORD WINAPI CSoundStream::StreamThread(LPVOID CSSPtr)
{
	CSoundStream* pss = (CSoundStream*)CSSPtr;
	pss->m_isDestruct = false;

	FILE* fp = fopen("debug.txt", "wt");
	DWORD start = timeGetTime();
	fprintf(fp, "start : %d\n", start);
	fprintf(fp, "---------------\n");

	while(1)
	{
		DWORD dwSignalPos = WaitForMultipleObjects(2, pss->m_pNotifyHandle, FALSE, INFINITE);
		if(pss->m_isDestruct) return 0;
		fprintf(fp, "time offset : %d\n", timeGetTime() - start);
		
		//シグナルになった位置を算出
		//dwSignalPos -= WAIT_OBJECT_0;

		//バッファに対する書き込み位置を算出
		DWORD dwWritePosStart	= pss->m_pDsbnotify[(dwSignalPos==0) ? 1 : 0].dwOffset;
		fprintf(fp, "dwWritePosStart : %d\n", pss->m_pDsbnotify[(dwSignalPos==0) ? 1 : 0].dwOffset);
		
		DWORD dwWritePosEnd		= pss->m_pDsbnotify[(dwSignalPos==0) ? 0 : 1].dwOffset;
		fprintf(fp, "dwWritePosEnd   : %d\n", pss->m_pDsbnotify[(dwSignalPos==0) ? 0 : 1].dwOffset);
		
		LONG dwWriteSize = dwWritePosEnd - dwWritePosStart;
	 	
	 	//バッファの最後の通知位置だった場合、書き込むサイズは全体のサイズからの差で求める
	 	if(0 > dwWriteSize)
	 	{
			dwWriteSize += (pss->m_wfx.nAvgBytesPerSec * (pss->m_dwBufferLengthSec));
			fprintf(fp, "dwWriteSize<0\n");
		}

		fprintf(fp, "dwWriteSize   : %d\n", dwWriteSize);
		fprintf(fp, "-------------------------------\n");
		
		void* pdata = NULL;
		//WAVEファイルから、データ部分を引っ張ってくる
		if(!(pss->CSound::ReadWaveData(NULL, &pdata, NULL, NULL, 0, 0, dwWriteSize)))
			return 0;
		//データをバッファに書き込む
		if(!(pss->CSound::WriteDataToBuffer(&(pss->m_pSecondaryBuffer), pdata, dwWritePosStart, dwWriteSize)))
		{
			free(pdata);
			return 0;
		}
		free(pdata);
	}
	fclose(fp);
	return 0;
}