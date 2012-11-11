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
	//2��ڈȍ~�̌Ăяo���̂��߂ɒ�~
	CSound::Stop();

	//��U�A�X���b�h���~����
	m_isDestruct = true;
	//SetEvent(m_pNotifyHandle[0]);
	//WaitForSingleObject(m_hThread, INFINITE);
	TerminateThread(m_hThread, FALSE);

	void* pdata = NULL;
	////�r���Đ��̈ʒu���o�C�g�P�ʂŎZ�o
	m_dwPos_byte = (DWORD)((double)dwStartPos_ms * (double)(m_wfx.nAvgBytesPerSec/1000.0));
	//�T�E���h�|�C���^�̈ʒu�����炵��
	if(m_dwPos_ms > 0) CSound::SetWavePointerPos(m_dwPos_byte, SEEK_SET);
	//�o�b�t�@�̑O�������炩���ߖ��߂Ă���
	DWORD dwWriteSize = (m_wfx.nAvgBytesPerSec * (m_dwBufferLengthSec / 2));
	//WAVE�t�@�C������A�f�[�^���������������Ă���
	if(!(CSound::ReadWaveData(NULL, &pdata, NULL, NULL, 0, 0, dwWriteSize)))
		return;
	//�f�[�^���o�b�t�@�ɏ�������
	if(!(CSound::WriteDataToBuffer(&m_pSecondaryBuffer, pdata, 0, dwWriteSize)))
	{
		free(pdata);
		return;
	}
	free(pdata);
	//�������񂾕��A����Ƀ|�C���^��i�߂Ă���
	//if(m_dwPos_ms > 0)
	//	CSound::SetWavePointerPos(m_dwPos_byte+dwWriteSize, SEEK_SET);

	//�X���b�h����
	DWORD tid;
	m_hThread = CreateThread(NULL, 0, CSoundStream::StreamThread, this, 0, &tid);
}

void CSoundStream::Play(DWORD position_ms, bool isLoop)
{
    //�Đ��ʒu�����i�[
	m_dwPos_ms = position_ms;
    
	//�X���b�h�쐬
	InitThread(m_dwPos_ms);

	m_pSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
}

bool CSoundStream::LoadWaveFromFile(const char* szFile)
{
	if(!m_pPrimaryBuffer)
	{
		//�v���C�}���o�b�t�@���擾����
		if(!GetPrimaryBuffer(&m_pPrimaryBuffer, DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_PRIMARYBUFFER))
			return false;
	}
	//�O�̃S�~�������Ă���
	SAFE_RELEASE(m_pSecondaryBuffer);
	
	DWORD dwBufferSize = 0;

	//�ŏ��̃t�@�C���ǂݍ���
	if(!CSound::ReadWaveData(&m_wfx, NULL, &dwBufferSize, szFile))
	{
		return false;
	}

	//�o�b�t�@�̍쐬
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

	//�ʒm�n���h���̍쐬
	if(m_pNotifyHandle) free(m_pNotifyHandle);
	m_pNotifyHandle = (HANDLE*)malloc( 2 * sizeof(HANDLE));
	if(m_pNotifyHandle == NULL) return false;
	for(int i=0; i<2; i++)
	{
		m_pNotifyHandle[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	//�ʒm�C���^�[�t�F�C�X�̍쐬
	if(FAILED(m_pSecondaryBuffer->QueryInterface(IID_IDirectSoundNotify, (void**)&m_pSoundNotify))) return false;

	SAFE_FREE(m_pDsbnotify);
	m_pDsbnotify = (DSBPOSITIONNOTIFY*)malloc(2 * sizeof(DSBPOSITIONNOTIFY));
	if(m_pDsbnotify == NULL) return false;
	m_pDsbnotify[0].dwOffset		= 0;
	m_pDsbnotify[0].hEventNotify	= m_pNotifyHandle[0];
	//�Q�b�Ԃ̃o�b�t�@��p���Ă���̂ŁA�킩��₷���^�񒆂ɂ��Ă����B
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
		
		//�V�O�i���ɂȂ����ʒu���Z�o
		//dwSignalPos -= WAIT_OBJECT_0;

		//�o�b�t�@�ɑ΂��鏑�����݈ʒu���Z�o
		DWORD dwWritePosStart	= pss->m_pDsbnotify[(dwSignalPos==0) ? 1 : 0].dwOffset;
		fprintf(fp, "dwWritePosStart : %d\n", pss->m_pDsbnotify[(dwSignalPos==0) ? 1 : 0].dwOffset);
		
		DWORD dwWritePosEnd		= pss->m_pDsbnotify[(dwSignalPos==0) ? 0 : 1].dwOffset;
		fprintf(fp, "dwWritePosEnd   : %d\n", pss->m_pDsbnotify[(dwSignalPos==0) ? 0 : 1].dwOffset);
		
		LONG dwWriteSize = dwWritePosEnd - dwWritePosStart;
	 	
	 	//�o�b�t�@�̍Ō�̒ʒm�ʒu�������ꍇ�A�������ރT�C�Y�͑S�̂̃T�C�Y����̍��ŋ��߂�
	 	if(0 > dwWriteSize)
	 	{
			dwWriteSize += (pss->m_wfx.nAvgBytesPerSec * (pss->m_dwBufferLengthSec));
			fprintf(fp, "dwWriteSize<0\n");
		}

		fprintf(fp, "dwWriteSize   : %d\n", dwWriteSize);
		fprintf(fp, "-------------------------------\n");
		
		void* pdata = NULL;
		//WAVE�t�@�C������A�f�[�^���������������Ă���
		if(!(pss->CSound::ReadWaveData(NULL, &pdata, NULL, NULL, 0, 0, dwWriteSize)))
			return 0;
		//�f�[�^���o�b�t�@�ɏ�������
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