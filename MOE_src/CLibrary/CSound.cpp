//v090
#include "stdafx.h"
#include "./CSound.h"

extern "C" const GUID CLSID_DirectSound      = { 0x47D4D946, 0x62E8, 0x11CF, 0x93, 0xBC, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 };
extern "C" const GUID IID_IDirectSound       = { 0x279AFA83, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60 };
extern "C" const GUID IID_IDirectSoundNotify = { 0xB0210783, 0x89CD, 0x11D0, 0xAF, 0x08, 0x00, 0xA0, 0xC9, 0x25, 0xCD, 0x16 };

//static
#if DIRECTSOUND_VERSION >= 0x0800
IDirectSound8* CSound::m_pSoundObject = NULL;
#else
IDirectSound* CSound::m_pSoundObject = NULL;
#endif
IDirectSoundBuffer* CSound::m_pPrimaryBuffer = NULL;
int CSound::m_nRef = 0;
HMODULE CSound::m_hDLL = NULL;

void CSound::QueryInitialize(){
	m_pSecondaryBuffer = NULL;
	
	//バッファの複製用
	m_nDuplicateLimit = 0;
	m_ppDuplicatedBuffer = NULL;

	m_Loader = NULL;

	m_isLoop		= FALSE;

	m_dwOneSplittedBufferSize = 0;
	m_pNotifyHandle = NULL;
	m_pSoundNotify = NULL;
	m_dwBufferLengthSec = 3;
	m_dwNotificationNum = 12;
	m_hThread = NULL;
	m_pDsbnotify = NULL;
	m_isStreamFile = FALSE;
	//m_dwPlayProgress = 0;

	m_isAllowRapidAccess = true;

	//InitializeCriticalSection(&m_csWriteBuffer);
	m_hThreadMessageDispatchEvent = NULL;

	m_nRef++;
}

CSound::CSound(){
	this->QueryInitialize();
	//m_nRef++;
	//this->QueryInitialize();
}

CSound::CSound(HWND hWnd, DWORD dwCoopLevel){
	//m_nRef++;
	this->QueryInitialize();
	//m_nRef++;
	//this->QueryInitialize();
	this->Initialize(hWnd, dwCoopLevel);
}

CSound::~CSound(){
	this->UnInitialize();
}

void CSound::MessageBox(const char* format, ...)
{
	//char* str = new char[1024];
	char str[4096];
	va_list ap;
	va_start(ap,format);
	wvsprintf(str,format,ap);
	va_end(ap);
	::MessageBox(NULL,str,"Message",MB_OK);
	//delete str;
}

int CSound::Initialize(HWND hWnd, DWORD dwCoopLevel)
{
	if(m_pSoundObject){
		return CS_E_NOCANDO;
	}

	if( !m_nRef ){
		this->QueryInitialize();
	}

	m_hDLL = LoadLibrary(TEXT("DSound.dll"));
#if DIRECTSOUND_VERSION >= 0x0800
	typedef HRESULT (WINAPI *pDirectSoundCreate)(LPCGUID, LPDIRECTSOUND8*, LPUNKNOWN);
	pDirectSoundCreate FDirectSoundCreate = (pDirectSoundCreate)GetProcAddress(m_hDLL, TEXT("DirectSoundCreate8"));
#else
	typedef HRESULT (WINAPI *pDirectSoundCreate)(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN);
	pDirectSoundCreate FDirectSoundCreate = (pDirectSoundCreate)GetProcAddress(m_hDLL, TEXT("DirectSoundCreate"));
#endif

	//DirectSoundオブジェクトの作成
	if(FAILED(FDirectSoundCreate(NULL, &m_pSoundObject, NULL)))
	{
		_ASSERT(0);
		return CS_E_NULL_OBJECT;
	}

	if(FAILED(m_pSoundObject->SetCooperativeLevel(hWnd, dwCoopLevel)))	//協調レベルを設定
	{
		_ASSERT(0);
		return CS_E_NOCANDO;
	}

	if(CS_E_OK!=this->SetPrimaryBufferWaveFormat(2, 44100, 16)){
		_ASSERT(0);
		return CS_E_NOCANDO;
	}
	return CS_E_OK;
}

void CSound::CloseStreamThread(){
	//this->Stop();	//2回目以降の呼び出しのために停止
	if(m_hThread){
		PostThreadMessage(m_dwThreadId, WM_QUIT, 0, 0);
		WaitForSingleObject(m_hThread, INFINITE);

		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

//サウンドオブジェクトを破棄
int CSound::UnInitialize()
{
	this->CloseStreamThread();
	SAFE_GLOBALFREE(m_pDsbnotify);
	SAFE_CLOSEHANDLE(m_pNotifyHandle);
	SAFE_RELEASE(m_pSoundNotify);

	//セカンダリバッファとその複製を破棄
	SAFE_RELEASE(m_pSecondaryBuffer);
	for(int i=0; i<m_nDuplicateLimit; i++)
	{
		SAFE_RELEASE(m_ppDuplicatedBuffer[i]);
	}
	SAFE_GLOBALFREE(m_ppDuplicatedBuffer);
	SAFE_RELEASE(m_pPrimaryBuffer);

	SAFE_DELETE(m_Loader);

	//DeleteCriticalSection(&m_csWriteBuffer);

	SAFE_CLOSEHANDLE(m_hThreadMessageDispatchEvent);

	if(!--m_nRef){
		SAFE_RELEASE(m_pSoundObject);
		FreeLibrary(m_hDLL);
	}
	return CS_E_OK;
}

int CSound::GetPrimaryBuffer(IDirectSoundBuffer** buffer, DWORD dwFlags)
{
	if(!m_pSoundObject)	return CS_E_NULL_OBJECT;
	//プライマリバッファを取得してくる
	DSBUFFERDESC dsbdesc;
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = dwFlags;
	dsbdesc.dwBufferBytes = 0;
	dsbdesc.lpwfxFormat = NULL;

	if(FAILED(m_pSoundObject->CreateSoundBuffer(&dsbdesc, buffer, NULL)))
	{
		_ASSERT(0);
		return CS_E_NULL_PRIMARY;
	}
	return CS_E_OK;
}

bool CSound::CreateBuffer(IDirectSoundBuffer** buffer, DSBUFFERDESC* dsbdesc, WAVEFORMATEX* wfx)
{
	//SAFE_RELEASE((*buffer));

	if(wfx != NULL && dsbdesc->lpwfxFormat == NULL)
		dsbdesc->lpwfxFormat = wfx;

	if(FAILED(m_pSoundObject->CreateSoundBuffer(dsbdesc, buffer, NULL)))
		return false;
	
	return true;
}

void CSound::SetPan(LONG nPan){
	if(!m_pSecondaryBuffer) return;
	m_pSecondaryBuffer->SetPan(nPan);
}

void CSound::SetVolume(LONG nVolume){
	if(!m_pSecondaryBuffer) return;
	m_pSecondaryBuffer->SetVolume(nVolume);
}

void CSound::SetFrequency(DWORD nFrequency){
	if(!m_pSecondaryBuffer) return;
	m_pSecondaryBuffer->SetFrequency(nFrequency);
}

LONG CSound::GetVolume(){
	if(!m_pSecondaryBuffer) return 0;
	LONG nVol;
	m_pSecondaryBuffer->GetVolume(&nVol);
	return CS_VOLUME_FIX_GET(nVol);
}

LONG CSound::GetPan(){
	LONG nPan;
	m_pSecondaryBuffer->GetPan(&nPan);
	return nPan;
}

void CSound::SetMasterPanAndVolume(LONG nVol, LONG nPan)
{
	if(CS_E_OK==GetPrimaryBuffer(&m_pPrimaryBuffer, DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_PRIMARYBUFFER)){
		m_pPrimaryBuffer->SetVolume(nVol);
		m_pPrimaryBuffer->SetPan(nPan);
	}
	SAFE_RELEASE(m_pPrimaryBuffer);
}

int CSound::SetPrimaryBufferWaveFormat(WORD Channels, DWORD SamplesPerSec, WORD BitsPerSample)
{
	if(CS_E_OK!=GetPrimaryBuffer(&m_pPrimaryBuffer, DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_PRIMARYBUFFER)) return CS_E_NULL_PRIMARY;

	WAVEFORMATEX wfx;
	ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
	wfx.wFormatTag      = WAVE_FORMAT_PCM; 
	wfx.nChannels       = Channels;
	wfx.nSamplesPerSec  = SamplesPerSec;
	wfx.wBitsPerSample  = BitsPerSample;
	wfx.nBlockAlign     = wfx.wBitsPerSample / 8 * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	if(FAILED(m_pPrimaryBuffer->SetFormat(&wfx))) return CS_E_NOCANDO;

	SAFE_RELEASE(m_pPrimaryBuffer);
	return CS_E_OK;
}

int CSound::GetLoaderInterface(CSoundLoader** ppLoader, const char* szFileName, void* pMemData, DWORD dwMembufferSize){
	//*ppLoader = NULL;
	/* DO NOT FREE PPLOADER AT HERE! */
	//if( *ppLoader!=NULL ){
	//	SAFE_DELETE(*ppLoader);
	//}

	CSoundLoader* tmpLoader;
	tmpLoader = new WaveLoader();
	if(tmpLoader->IsLoadable(szFileName, pMemData, dwMembufferSize)){
		*ppLoader = tmpLoader;
		return CS_E_OK;
	}

	SAFE_DELETE(tmpLoader);
	tmpLoader = new OggVorbisLoader();
	if(tmpLoader->IsLoadable(szFileName, pMemData, dwMembufferSize)){
		*ppLoader = tmpLoader;
		return CS_E_OK;
	}

	SAFE_DELETE(tmpLoader);
	*ppLoader = NULL;
	return CS_E_UNEXP;
}

int CSound::Load(const char* szFileName, DSBUFFERDESC* pDsbdesc){
	return this->LoadInternal(szFileName, pDsbdesc);
}

////NEED TO REVIEW
//int CSound::LoadWaveFromMemory(void* pdata, WAVEFORMATEX* wfx, DWORD dwLength_byte, DSBUFFERDESC* pDsbdesc)
//{
//	if(!m_pPrimaryBuffer){		//プライマリバッファを取得する
//		if(CS_E_OK!=GetPrimaryBuffer(&m_pPrimaryBuffer, DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_PRIMARYBUFFER)) return CS_E_NULL_PRIMARY;
//	}
//	SAFE_RELEASE(m_pSecondaryBuffer);	//前のゴミを消しておく
//
//	if(NULL == pdata || NULL == wfx || 0 == dwLength_byte)	return CS_E_NULL;
//
//	DSBUFFERDESC dsbdesc;
//	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
//	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
//	dsbdesc.dwFlags =	DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME|
//						DSBCAPS_CTRLFREQUENCY|
//						DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_CTRLPOSITIONNOTIFY|
//						DSBCAPS_GLOBALFOCUS;
//	dsbdesc.dwBufferBytes = dwLength_byte;
//	dsbdesc.lpwfxFormat = wfx;
//
//	CopyMemory(&m_wfx, wfx, sizeof(WAVEFORMATEX));
//
//	if(!CreateBuffer(&m_pSecondaryBuffer, &dsbdesc, NULL))	return CS_E_NOCANDO;	//セカンダリバッファの作成
//	if(!WriteDataToBuffer(&m_pSecondaryBuffer, pdata, 0, dwLength_byte)) return CS_E_NOCANDO;	//データをバッファに書き込む
//	SAFE_RELEASE(m_pPrimaryBuffer);
//	return CS_E_OK;
//}

int CSound::LoadInternal(const char* szFileName, DSBUFFERDESC* pDsbdesc, void* pData, unsigned long dwDataSize){

	CSoundLoader* pLoader;
	char errmsg[512];
	if(CS_E_OK!=this->GetLoaderInterface(&pLoader, szFileName, pData, dwDataSize)){
		wsprintf(errmsg, "Sound::%sの読み取りインターフェイス取得に失敗.\nファイルが存在するかもしくは対応形式か確認して下さい.", szFileName);
		::MessageBox(NULL, errmsg, "", MB_ICONEXCLAMATION|MB_OK|MB_TOPMOST);
		//FatalAppExit(0, errmsg);
		return CS_E_NOTFOUND;
	}
	if(CSL_E_OK != pLoader->QueryLoadFile(szFileName, pData, dwDataSize)){
		wsprintf(errmsg, "Sound::%sの読み取りに失敗.", szFileName);
		::MessageBox(NULL, errmsg, "", MB_ICONEXCLAMATION|MB_OK|MB_TOPMOST);
		//FatalAppExit(0, errmsg);
		return CS_E_UNEXP;
	}

	//初期化
	this->AddRef();
	this->UnInitialize();

	m_Loader = pLoader;

	if(!m_pPrimaryBuffer){		//プライマリバッファを取得する
		if(CS_E_OK!=GetPrimaryBuffer(&m_pPrimaryBuffer, DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_PRIMARYBUFFER)) return CS_E_NULL_PRIMARY;
	}

	DSBUFFERDESC dsbdesc;
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);

	//全体の長さとWFXの取得
	DWORD dwDataLength = m_Loader->GetDecodedLength();
	m_Loader->GetWaveFormatEx(&m_wfx);
	if(dwDataLength >= CS_LIMITLOADONMEMORY){//展開したときのサイズが1MB以上だったらストリーミング再生]
		//スレッド処理
		this->CloseStreamThread();
		m_hThreadMessageDispatchEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hThread = CreateThread(NULL, 0, this->StreamThread, (void*)this, 0, &m_dwThreadId);		//スレッド生成
		WaitForSingleObject(m_hThreadMessageDispatchEvent, INFINITE);// スレッドメッセージキューが作成されるのを待つ

		m_isStreamFile = TRUE;

		//セカンダリバッファ
		{
			SAFE_RELEASE(m_pSecondaryBuffer);
			dsbdesc.dwFlags =	DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_CTRLPOSITIONNOTIFY|
								DSBCAPS_GLOBALFOCUS|DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME|
								DSBCAPS_CTRLFREQUENCY|DSBCAPS_LOCSOFTWARE;

			if(pDsbdesc){
				dsbdesc.dwFlags = pDsbdesc->dwFlags;
				dsbdesc.guid3DAlgorithm = dsbdesc.guid3DAlgorithm;
			}
			dsbdesc.lpwfxFormat = &m_wfx;
			DWORD dwSize = m_wfx.nAvgBytesPerSec * m_dwBufferLengthSec / m_dwNotificationNum;
			dwSize -= dwSize % m_wfx.nBlockAlign;
			dsbdesc.dwBufferBytes = dwSize * m_dwNotificationNum;
			if(!CreateBuffer(&m_pSecondaryBuffer, &dsbdesc, NULL))	return CS_E_NOCANDO;

			m_dwOneSplittedBufferSize = dwSize;//区切られたバッファの１つのサイズ（バッファ全体はこれ*m_dwNotificationNum
		}

		//通知インターフェイス
		{
			SAFE_RELEASE(m_pSoundNotify);
			if(FAILED(m_pSecondaryBuffer->QueryInterface(IID_IDirectSoundNotify, (void**)&m_pSoundNotify))){
				return CS_E_NOCANDO;
			}

			SAFE_GLOBALFREE(m_pDsbnotify);
			if(!(m_pDsbnotify = (DSBPOSITIONNOTIFY*)GlobalAlloc(GPTR, m_dwNotificationNum * sizeof(DSBPOSITIONNOTIFY)))){
				return CS_E_UNEXP;
			}

			m_pNotifyHandle = CreateEvent(NULL, FALSE, FALSE, NULL);	//通知ハンドルの作成
			for(DWORD i=0; i<m_dwNotificationNum; i++){
				m_pDsbnotify[i].dwOffset     = (m_dwOneSplittedBufferSize*i) + m_dwOneSplittedBufferSize - 1;
				m_pDsbnotify[i].hEventNotify = m_pNotifyHandle;
			}
			if(FAILED(m_pSoundNotify->SetNotificationPositions(m_dwNotificationNum, m_pDsbnotify))){
				SAFE_GLOBALFREE(m_pDsbnotify);
				SAFE_RELEASE(m_pSoundNotify);
				SAFE_CLOSEHANDLE(m_pNotifyHandle);
				return CS_E_NOCANDO;
			}
		}
	}else{
		m_isStreamFile = FALSE;

		void* pdata = NULL;
		if(CSL_E_OK != m_Loader->GetDecodedData(&pdata, 0, 0, FALSE)){
		}

		dsbdesc.dwFlags =	DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME|
							DSBCAPS_CTRLFREQUENCY|
							DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_CTRLPOSITIONNOTIFY|
							DSBCAPS_GLOBALFOCUS;
		if(pDsbdesc){
			dsbdesc.dwFlags = pDsbdesc->dwFlags;
			dsbdesc.guid3DAlgorithm = dsbdesc.guid3DAlgorithm;
		}
		dsbdesc.dwBufferBytes = dwDataLength;
		dsbdesc.lpwfxFormat = &m_wfx;

		SAFE_RELEASE(m_pSecondaryBuffer);
		if(!CreateBuffer(&m_pSecondaryBuffer, &dsbdesc, NULL)) return CS_E_NOCANDO;		//セカンダリバッファの作成
		if(!WriteDataToBuffer(&m_pSecondaryBuffer, pdata, 0, dwDataLength))	return CS_E_NOCANDO;		//データをバッファに書き込む
		
		SAFE_GLOBALFREE(pdata);
	}
	SAFE_RELEASE(m_pPrimaryBuffer);
	return CS_E_OK;
}

int CSound::LoadMemoryImage(void* pData, unsigned long dwDataSize){
	if(NULL == pData || 0 == dwDataSize)	return CS_E_NULL;
	return this->LoadInternal(CSL_LOAD_MEMORYIMAGE, NULL, pData, dwDataSize);
}

//waveデータをサウンドバッファにコピー
bool CSound::WriteDataToBuffer(IDirectSoundBuffer** buffer, void* data, DWORD dwOffset, DWORD dwWriteSize)
{
	//EnterCriticalSection(&m_csWriteBuffer);
		void* pmem[2];
		DWORD alloc_size[2];
		
		if(NULL == data || m_pSecondaryBuffer == NULL)	return false;

		if(DSERR_BUFFERLOST == (*buffer)->Lock(dwOffset, dwWriteSize, &pmem[0], &alloc_size[0], &pmem[1], &alloc_size[1], 0)){//サウンドバッファをロック
			(*buffer)->Restore();
			if(DS_OK != (*buffer)->Lock(dwOffset, dwWriteSize, &pmem[0], &alloc_size[0], &pmem[1], &alloc_size[1], 0))	return false;
		}

		//読み込んだサイズ分コピー
		if(pmem[0])	CopyMemory(pmem[0], data, alloc_size[0]);
		else		CopyMemory(pmem[1], data, alloc_size[1]);

		(*buffer)->Unlock(pmem[0], alloc_size[0], pmem[1], alloc_size[1]);	//バッファをアンロック
	//LeaveCriticalSection(&m_csWriteBuffer);
	return true;
}

//同一サウンド多重再生用コピー
void CSound::DuplicateBuffer(DWORD nDuplicate)
{
	if(!m_pSecondaryBuffer)	return;
	if(m_isStreamFile) return;

	if(m_ppDuplicatedBuffer!=NULL){	//後片付けを先にする
		for(int i=0; i<m_nDuplicateLimit; i++){
			SAFE_RELEASE(m_ppDuplicatedBuffer[i]);
		}
		SAFE_GLOBALFREE(m_ppDuplicatedBuffer);
	}

	if(nDuplicate > 0){
		m_ppDuplicatedBuffer = (IDirectSoundBuffer**)GlobalAlloc(GPTR, sizeof(IDirectSoundBuffer*) * nDuplicate);
		for(DWORD i=0; i<nDuplicate; i++){
			if(FAILED(m_pSoundObject->DuplicateSoundBuffer(m_pSecondaryBuffer, &m_ppDuplicatedBuffer[i]))){
				m_ppDuplicatedBuffer[i] = NULL;
			}
		}
	}else{
		if(m_ppDuplicatedBuffer!=NULL){
			for(int i=0; i<m_nDuplicateLimit; i++){
				SAFE_RELEASE(m_ppDuplicatedBuffer[i]);
			}
			SAFE_GLOBALFREE(m_ppDuplicatedBuffer);
		}
	}

	m_nDuplicateLimit = nDuplicate;	//デュプリケート数を保存
}

DWORD CSound::GetCurrentPosition()
{
	if(!m_pSecondaryBuffer)	return 0;
	if(m_isStreamFile){
		DWORD dwRet;
		PostThreadMessage(m_dwThreadId, CSL_MSG_GETCURPOS_BYTE, 0, (LPARAM)&dwRet);
		WaitForSingleObject(m_hThreadMessageDispatchEvent, INFINITE);
		//DWORD dwTmp = m_dwPlayProgress - (m_dwPlayProgress/(double)m_Loader->GetDecodedLength() * m_dwPlayProgress);
		return (DWORD)(dwRet / ((double)m_wfx.nAvgBytesPerSec/1000.0));
	}
	DWORD  now, now2;
	m_pSecondaryBuffer->GetCurrentPosition( &now, &now2 );
	return (DWORD)((double)now / (double)(m_wfx.nAvgBytesPerSec/1000.0));
}

DWORD CSound::GetFrequency()
{
	if(!m_pSecondaryBuffer)	return 0;
	DWORD  dwFreq;
	m_pSecondaryBuffer->GetFrequency(&dwFreq);
	return dwFreq;
}

int CSound::Play(DWORD position_ms, bool isLoop)
{
	if(!m_pSecondaryBuffer) return CS_E_NULL_SECONDARY;
	m_isLoop = isLoop;

	if(m_isStreamFile){
		//OpenStreamThread();
		//if(IsPlaying()){
		//PostThreadMessage(m_dwThreadId, CSL_MSG_SEEK_AND_PLAY, (DWORD)(position_ms * ((double)m_wfx.nAvgBytesPerSec/1000.0)), 0);
    PostThreadMessage(m_dwThreadId, CSL_MSG_SEEK_AND_PLAY, position_ms, 0);
		WaitForSingleObject(m_hThreadMessageDispatchEvent, INFINITE);
		//this->SetStreamCurosr((DWORD)(position_ms * ((double)m_wfx.nAvgBytesPerSec/1000.0)));
		//}
		//m_pSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
	}else{
		IDirectSoundBuffer* pBuffer = NULL;
		if(m_nDuplicateLimit > 0){	//セカンダリバッファのコピーを使う
			int id = GetInactiveBufferNo();//使用中でないバッファのインデックスを取得
			if(id == -1) return CS_E_NOCANDO;
			pBuffer = m_ppDuplicatedBuffer[id];
		}else{
			if( !m_isAllowRapidAccess ){
				DWORD dwStatus;
				m_pSecondaryBuffer->GetStatus( &dwStatus );
				bool isSecondaryPlaying = (dwStatus & DSBSTATUS_PLAYING);
				if( isSecondaryPlaying ){
					/* nop */
				}else{
					pBuffer = m_pSecondaryBuffer;
				}
			}else{
				pBuffer = m_pSecondaryBuffer;
			}
		}
		if( !pBuffer ){
			/* nop */
		}else{
			pBuffer->SetCurrentPosition( (DWORD)((double)position_ms * (double)(m_wfx.nAvgBytesPerSec/1000.0)) );
			pBuffer->Play(0, 0, isLoop ? DSBPLAY_LOOPING : 0);
			//m_pSecondaryBuffer->Play(0, 0, m_isStreamFile ? DSBPLAY_LOOPING : isLoop ? DSBPLAY_LOOPING : 0);
		}
	}
	return CS_E_OK;
}

//使用中でない複製バッファのIDを探す
int CSound::GetInactiveBufferNo()
{
	if(!m_ppDuplicatedBuffer) return -1;

	DWORD dwStatus;
	int max = m_nDuplicateLimit;
	for(int i=0; i<max; i++){
		if(m_ppDuplicatedBuffer[i]){
			m_ppDuplicatedBuffer[i]->GetStatus( &dwStatus );
			if(!(dwStatus & DSBSTATUS_PLAYING)){
				return i;
			}
		}
	}
	return -1;
}

int CSound::Stop()
{
	if(!m_pSecondaryBuffer) return CS_E_NULL_SECONDARY;
	if( m_isStreamFile ){
		PostThreadMessage(m_dwThreadId, CSL_MSG_STOP, 0, 0);
		WaitForSingleObject(m_hThreadMessageDispatchEvent, INFINITE);
	}else{
		m_pSecondaryBuffer->Stop();
		this->StopDuplicatedBuffer();	//まさか複製バッファの音だけ残しておきたいとか無いでしょ・・・
	}
	return CS_E_OK;
}

int CSound::StopDuplicatedBuffer(){
	if(!m_ppDuplicatedBuffer) return CS_E_NOCANDO;
	for(int i=0; i<m_nDuplicateLimit; i++){
		if(m_ppDuplicatedBuffer[i]){
			m_ppDuplicatedBuffer[i]->Stop();
			m_ppDuplicatedBuffer[i]->SetCurrentPosition(0);
		}
	}
	return CS_E_OK;
}

bool CSound::IsPlaying()
{
	if(!m_pSecondaryBuffer)	return 0;
	DWORD dwStatus;
	m_pSecondaryBuffer->GetStatus( &dwStatus );
	bool isSecondaryPlaying = (dwStatus & DSBSTATUS_PLAYING);
	return (isSecondaryPlaying || IsDuplicatedBufferPlaying());
}

bool CSound::IsDuplicatedBufferPlaying(){
	if(!m_ppDuplicatedBuffer) return false;

	DWORD dwStatus;
	for(int i=0; i<m_nDuplicateLimit; i++){
		if(m_ppDuplicatedBuffer[i]){
			m_ppDuplicatedBuffer[i]->GetStatus( &dwStatus );
			if(!(dwStatus & DSBSTATUS_PLAYING))	return true;
		}
	}
	return false;
}

int CSound::Reset(){
	if(!m_pSecondaryBuffer) return CS_E_NULL_SECONDARY;

	//if(m_isStreamFile){
	//	this->SetStreamCurosr(0);
	//}

	//m_pSecondaryBuffer->Stop();
	//m_pSecondaryBuffer->SetCurrentPosition(0);
	return CS_E_OK;
}

//int CSound::SetStreamCurosr(DWORD dwResumePosMs){
//	if(!m_pSecondaryBuffer) return CS_E_NULL_SECONDARY;
//	//m_dwPlayProgress = dwResumePosMs;
//	PostThreadMessage(m_dwThreadId, CSL_MSG_SEEK_AND_PLAY, dwResumePosMs, 0);
//
//	//m_pSecondaryBuffer->Stop();
//	//m_pSecondaryBuffer->SetCurrentPosition(0);
//
//	//m_Loader->SetWavePointerPos(dwResumePosMs, SEEK_SET);
//	//void* pdata = NULL;
//	//	m_Loader->GetDecodedData(&pdata, 0, m_dwOneSplittedBufferSize*m_dwNotificationNum, m_isLoop);
//	//	if(!WriteDataToBuffer(&m_pSecondaryBuffer, pdata, 0, m_dwOneSplittedBufferSize * m_dwNotificationNum))	return CS_E_NOMEM;
//	//SAFE_GLOBALFREE(pdata);
//
//	//this->CloseThread();
//	return CS_E_OK;
//}



void OutputDebugStringFormatted(const char* format, ...)
{
	char str[1024];
	va_list ap;
	va_start(ap,format);
	_vsnprintf(str, sizeof(str), format, ap);
	va_end(ap);

	::OutputDebugString(str);
}

//
//Stream
//
DWORD WINAPI CSound::StreamThread(LPVOID CSSPtr)
{
	CSound* pss = (CSound*)CSSPtr;

	//DWORD dwPlayDelta = 0;
	DWORD dwPlayProgress    = 0;
	DWORD dwNextWriteOffset = 0;
	DWORD dwLastPlayPos     = 0;
	BOOL  isFileFin         = FALSE;

  //! PostThreadMessageが失敗しないようにメッセージキューを作成する必要がある see MSDN::PostThreadMessage
	ResetEvent(pss->m_hThreadMessageDispatchEvent);
    MSG createQueueMsg;
    PeekMessage(&createQueueMsg, NULL, 0, 0, PM_NOREMOVE);
	SetEvent(pss->m_hThreadMessageDispatchEvent);//メッセージキュー作成完了

  while(TRUE){
		DWORD dwSignalPos = MsgWaitForMultipleObjects(1, &(HANDLE)(pss->m_pNotifyHandle), FALSE, INFINITE, QS_ALLEVENTS);
		if(dwSignalPos == WAIT_OBJECT_0 + 0){		//シグナルになった位置を算出
			//OutputDebugStringFormatted("SIGNAL  IN\n");

			DWORD  dwCurrentPlayPos;
			pss->m_pSecondaryBuffer->GetCurrentPosition(&dwCurrentPlayPos, NULL);
			//dwPlayDelta = dwCurrentPlayPos - dwLastPlayPos;

			//if(isFileFin){
			//	dwPlayProgress			= 0;
			//	dwLastPlayPos			= 0;
			//	dwNextWriteOffset		= 0;
			//	isFileFin				= FALSE;
			//	//PostThreadMessage(pss->m_dwThreadId, CSL_MSG_SEEK_AND_PLAY, 0, 0);
			//	//WaitForSingleObject(pss->m_hThreadMessageDispatchEvent, INFINITE);
			//	continue;
			//}

			void* pdata = NULL;
				DWORD dwCurrentDecodePos = pss->m_Loader->GetCurrentDecodedPos();

				int retdec = pss->m_Loader->GetDecodedData(&pdata, dwCurrentDecodePos, pss->m_dwOneSplittedBufferSize, pss->m_isLoop);
				if(retdec == CSL_N_FIN){
					//OutputDebugStringFormatted("file fin\n");
					isFileFin = TRUE;
				}else if(retdec == CSL_E_UNEXP){
					//OutputDebugStringFormatted("unexpected error while GetDecodedData\n");
					continue;
				}
				if(!(pss->WriteDataToBuffer(&(pss->m_pSecondaryBuffer), pdata, dwNextWriteOffset, pss->m_dwOneSplittedBufferSize))){
					//OutputDebugStringFormatted("unexpected error while WriteDataToBuffer\n");
					SAFE_GLOBALFREE(pdata);
					continue;
				}
			SAFE_GLOBALFREE(pdata);

			if( isFileFin ){
				dwPlayProgress			= 0;
				dwLastPlayPos			= 0;
				//dwNextWriteOffset		= 0;
				isFileFin				= FALSE;
			}else{
				dwPlayProgress += (( ((long)dwCurrentPlayPos - (long)dwLastPlayPos)) >= 0) ? ((long)dwCurrentPlayPos - (long)dwLastPlayPos) : dwCurrentPlayPos;
				dwLastPlayPos  = dwCurrentPlayPos;
			}

			dwNextWriteOffset += pss->m_dwOneSplittedBufferSize;
			dwNextWriteOffset %= (pss->m_dwOneSplittedBufferSize * pss->m_dwNotificationNum);
			//OutputDebugStringFormatted("SIGNAL OUT\n");
		}else if(WAIT_OBJECT_0 + 1){
			MSG msg;
			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
        switch( msg.message ){
					case WM_QUIT:
					{
						pss->m_pSecondaryBuffer->Stop();
						return 0;
					}break;
					case CSL_MSG_SEEK_AND_PLAY:
					case CSL_MSG_SEEK:
					{
						ResetEvent(pss->m_hThreadMessageDispatchEvent);
						//OutputDebugStringFormatted("CSL_MSG_SEEK_AND_PLAY\n");

						pss->m_pSecondaryBuffer->Stop();
						pss->m_pSecondaryBuffer->SetCurrentPosition(0);
						dwPlayProgress			= 0;
						dwLastPlayPos			= 0;
						dwNextWriteOffset		= 0;

						//アライメントをチェックする
						//QWORD dwTmp = (dwFrom>0) ? dwFrom+dwSizeToRead : dwSizeToRead;
						DWORD dwSizeToRead = (DWORD)(msg.wParam/1000.0 * (double)pss->m_wfx.nAvgBytesPerSec);

						//OutputDebugStringFormatted("User wants pointer to %dms, that means %dbytes\n", msg.wParam, dwSizeToRead);
						if(dwSizeToRead % pss->m_wfx.nBlockAlign){
							if((dwSizeToRead % pss->m_wfx.nBlockAlign) <= (DWORD)(pss->m_wfx.nBlockAlign / 2)){
								dwSizeToRead-=(dwSizeToRead % pss->m_wfx.nBlockAlign);
							}else{
								dwSizeToRead+=(pss->m_wfx.nBlockAlign - (dwSizeToRead % pss->m_wfx.nBlockAlign));
							}
							//OutputDebugStringFormatted("this must be aligned...%d\n", dwSizeToRead);
						}
						//dwSizeToRead = dwTmp;

						//pss->m_Loader->SetWavePointerMS(msg.wParam);
						pss->m_Loader->SetWavePointerPos(dwSizeToRead);
						//OutputDebugStringFormatted("Wave pointer pos set to %d\n", (msg.wParam * ((double)pss->m_wfx.nAvgBytesPerSec/1000.0)));

						void* pdata = NULL;
							int retdec = pss->m_Loader->GetDecodedData(&pdata, CSL_CONTINUE_CURSOR, pss->m_dwOneSplittedBufferSize*pss->m_dwNotificationNum, pss->m_isLoop);
							if(retdec == CSL_E_UNEXP){
								//OutputDebugStringFormatted("unexpected error while GetDecodedData\n");
								//continue;
							}else{
								//if(retdec == CSL_E_OUTOFRANGE){
								//	//nop
								//}else{
									//if(retdec == CSL_N_FIN){
									//	//OutputDebugStringFormatted("file fin\n");
									//	//isFileFin = TRUE;//TODO: ファイルの終端にたどり着いたときにどうするか
									//}
									if(!pss->WriteDataToBuffer(&pss->m_pSecondaryBuffer, pdata, 0, pss->m_dwOneSplittedBufferSize*pss->m_dwNotificationNum)){
										//OutputDebugStringFormatted("unexpected error while WriteDataToBuffer\n");
										SAFE_GLOBALFREE(pdata);
										//continue;
									}
									dwPlayProgress += pss->m_dwOneSplittedBufferSize;//FIXME ?:GetCurrentPositionがおかしいときはここを疑え
								//}
							}
						SAFE_GLOBALFREE(pdata);

						if( msg.message==CSL_MSG_SEEK_AND_PLAY ) pss->m_pSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

						SetEvent(pss->m_hThreadMessageDispatchEvent);
					}break;
					case CSL_MSG_GETCURPOS_BYTE:
					{
						ResetEvent(pss->m_hThreadMessageDispatchEvent);
						*((DWORD*)msg.lParam) = dwPlayProgress;
						SetEvent(pss->m_hThreadMessageDispatchEvent);
					}break;
					case CSL_MSG_PAUSE:
					case CSL_MSG_STOP:
					{
						//OutputDebugStringFormatted("CSL_MSG_STOP\n");
						ResetEvent(pss->m_hThreadMessageDispatchEvent);
						pss->m_pSecondaryBuffer->Stop();

						if( CSL_MSG_STOP ){
							pss->m_pSecondaryBuffer->SetCurrentPosition(0);

							dwPlayProgress			= 0;
							dwLastPlayPos			= 0;
							dwNextWriteOffset		= 0;
						}

						SetEvent(pss->m_hThreadMessageDispatchEvent);
					}break;
				}
			}
		}
	}
	return 0;
}

void CSound::AllowRapidAccess(bool isAllow){
	m_isAllowRapidAccess = isAllow;
}

DWORD CSound::GetDuration(){
	if( m_Loader ){
		return m_Loader->GetDecodedLengthMs();
	}
	return 0;
}