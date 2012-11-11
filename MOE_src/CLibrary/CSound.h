//###			DirectSound Library 		 ###
//###	Code, Design : c.r.v. 2004-2005		 ###
/*
	version:
		0.1 - blog���J��
		0.5 - box
		0.6 - box add :: GetFrequency(), 'virtual'
		0.7 - ���������v���O���}�̐ӔC��.
		      ������������.
			  �f���v���P�[�g�Ή�.
		0.71- �w���ŃR���p�C���ł���悤��.
		      �x���}�V������,�f���v���P�[�g�Ɏ��Ԃ������邱�Ƃ����o.
			  �g�p�������팸������,������ӂ�����.
			  ���\�b�h���ύX.GetPosition -> GetCurrentPosision
		0.72- Wave�ǂݍ��ݕ��������P.������.
		0.73- Wave�ǂݍ��ݍ������ɔ����o�O�C��.
		0.74- �v���C�}���o�b�t�@�̊Ǘ�������ύX.�g�p�������팸.
		0.75- �t�[���v���[�t�R����.
		0.76- �f�X�g���N�^�̃o�O�C��
		0.77- �^�ϊ��p�̃I�y���[�^.�������ɉ߂���, ����ύX����\��������.
		0.78- ���\�b�h�����낢��ύX. ��n�����v���O���}�̐ӔC��.�ꕔ�ϐ����ύX.
		0.79- �t�[���v���[�t������Ƒ���.
		0.80- ���b�v�A���E���h�Ή�::WAVE�t�@�C���ǂݍ���
		0.81- WriteDataToBuffer()�̈�����ύX.�X�g���[�~���O�Ή���.
		0.82- Stop()����������.
		0.83- �����o�b�t�@�Đ����ɁA��Ƀ��C���o�b�t�@�̍Đ��ʒu���O�ɂȂ�����C��
		0.84- �T�E���h�t�@�C���̓W�J�͊O���N���X�ōs�����Ƃɂ����̂ŁA�����������B
		0.90- CSoundLoader�Ƃ̘A�g���������B
				
	date:
		-0.82 05/31/2005
		0.83- Oct.21.2005

	notice:
		mp3�͓������̂��߃T�|�[�g���Ȃ��B
		DirectShow�̃f�R�[�_�ɓ����Ă�邱�Ƃ��ł��邯�ǁA��i�𔄂�Ƌ������̂ŋp���B
		�ǁ[���Ă�mp3����Ȃ��ጙ�Ȑl�͏���Ƀf�R�[�_�ɓ����ē���ꂽWAVE��n�����ق��̃��C�u�������g���Ă��������B
*/
#ifndef CSOUND_H_DEFINED
#define CSOUND_H_DEFINED

#include "CSoundLoader.h"
#include <windows.h>

#define CS_LIMITLOADONMEMORY 1024*1024*1	//��������ɓW�J����ő�e��(1mB)�B����𒴂���ƃX�g���[�~���O�Đ��B
#define CS_NOSOUND		-10000
#define CS_VOLUME_DB(csdB__)   ((csdB__)*100)
#define CS_VOLUME_MAX        0
#define CS_VOLUME_MIN	-10000
#define CS_VOLUME_RANGE  10000
#define CS_VOLUME_RANGE_REAL 10000
#define CS_VOLUME_FIX_SET(csVolumeToSet__) (LONG)(csVolumeToSet__/**(CS_VOLUME_RANGE_REAL/CS_VOLUME_RANGE)*/)/*CS_VOLUME_RANGE_REAL-CS_VOLUME_RANGE*/
#define CS_VOLUME_FIX_GET(csVolumeToGet__) (LONG)(csVolumeToGet__/*/(CS_VOLUME_RANGE_REAL/CS_VOLUME_RANGE)*/)
#define CS_PAN_RIGHT	 10000
#define CS_PAN_LEFT		-10000

#define CSL_MSG_SEEK			WM_USER+43+1
#define CSL_MSG_GETCURPOS_BYTE  WM_USER+43+2
#define CSL_MSG_STOP			WM_USER+43+3
#define CSL_MSG_SEEK_AND_PLAY   WM_USER+43+4
#define CSL_MSG_PAUSE			WM_USER+43+5

enum {CS_E_NOTIMPL, CS_E_OK, CS_E_UNEXP, CS_E_NOTLOADED, CS_E_NOTFOUND, CS_E_NULL_OBJECT, CS_E_NULL_PRIMARY, CS_E_NULL_SECONDARY, CS_E_NULL, CS_E_NOMEM, CS_E_NOCANDO};

class CSound
{
protected:
	static HMODULE m_hDLL;

#if DIRECTSOUND_VERSION >= 0x0800
	static IDirectSound8* m_pSoundObject;
#else
	static IDirectSound* m_pSoundObject;
#endif
	static IDirectSoundBuffer*	m_pPrimaryBuffer;
	static int m_nRef;
	IDirectSoundBuffer* m_pSecondaryBuffer;
	volatile bool m_isLoop;

	WAVEFORMATEX m_wfx;

	int m_nDuplicateLimit;
	IDirectSoundBuffer** m_ppDuplicatedBuffer;

	CSoundLoader*		m_Loader;
	
	volatile DWORD m_dwOneSplittedBufferSize;
	volatile DWORD m_dwNotificationNum;
	volatile DWORD m_dwBufferLengthSec;

	BOOL	m_isStreamFile;

	DWORD m_dwThreadId;
	HANDLE m_hThread;

	volatile HANDLE m_pNotifyHandle;
	IDirectSoundNotify* m_pSoundNotify;
	DSBPOSITIONNOTIFY* m_pDsbnotify;

	bool m_isAllowRapidAccess;

	//CRITICAL_SECTION m_csWriteBuffer;

	volatile HANDLE m_hThreadMessageDispatchEvent;

protected:
	static void	MessageBox(const char* format, ...);
	int			GetPrimaryBuffer(IDirectSoundBuffer** buffer, DWORD dwFlags);	//�v���C�}���o�b�t�@���擾���Ă���
	bool		CreateBuffer(IDirectSoundBuffer** buffer, DSBUFFERDESC* dsbdesc, WAVEFORMATEX* wfx);	//�o�b�t�@�쐬.
	bool		WriteDataToBuffer(IDirectSoundBuffer** buffer, void* data, DWORD dwOffset, DWORD dwWriteSize);	//�o�b�t�@��WAVE�f�[�^��]������.
	int			GetInactiveBufferNo();	//�g�p����Ă��Ȃ������o�b�t�@ID�����o��
	int			GetLoaderInterface(CSoundLoader** ppLoader, const char* szFileName, void* pMemData=NULL, DWORD dwMembufferSize = 0);
	static		DWORD WINAPI StreamThread(LPVOID CSSPtr);
	void		CloseStreamThread();
	//void		OpenStreamThread();
	void		QueryInitialize();
	int			LoadInternal(const char* szFileName, DSBUFFERDESC* pDsbdesc, void* pData = NULL, unsigned long dwDataSize = 0);


	bool IsDuplicatedBufferPlaying();
	int StopDuplicatedBuffer();
	int SetStreamCurosr(DWORD dwResumePosMs);

public:
	CSound();
	CSound(HWND hWnd, DWORD dwCoopLevel = DSSCL_PRIORITY);
	virtual ~CSound();
public:
	int AddRef(){ return ++m_nRef; }
	int Release(){ return --m_nRef; }

	int Initialize(HWND hWnd, DWORD dwCoopLevel = DSSCL_PRIORITY);	//������
	int UnInitialize();												//�j��

	int Load(const char* szFileName, DSBUFFERDESC* pDsbdesc = NULL);
	//int LoadWaveFromMemory(void* pdata, WAVEFORMATEX* wfx, DWORD dwLength_byte, DSBUFFERDESC* pDsbdesc = NULL);
	int LoadMemoryImage(void* pData, unsigned long dwDataSize);
	int Play(DWORD position_ms = 0, bool isLoop = false);
	int Stop();

	int Reset();
	bool IsPlaying();

	bool IsLoaded(){ return (m_Loader!=NULL); }

	DWORD GetDuration();
	DWORD GetCurrentPosition();	//�Đ��ʒu��ms�P�ʂŎ擾
	DWORD GetFrequency();		//���g����Hz�P�ʂŎ擾
	LONG  GetVolume();
	LONG  GetPan();

	void SetPan(LONG nPan);				//�p���l�̃����W -> ��:-10,000 �E:10,000
	void SetVolume(LONG nVolume);		//�{�����[���l�̃����W(1/100dB) -> ������:0 ����:-10,000 �����̓T�|�[�g����Ȃ�
	void SetFrequency(DWORD nFrequency);	//���g���l(Hz) -> ������:0
	void SetMasterPanAndVolume(LONG nVol = 0, LONG nPan = 0);

	//�v���C�}���o�b�t�@�i�o�̓o�b�t�@�j�̃t�H�[�}�b�g��ݒ肷��
	//�f�t�H���g -> 2Channels, 44.1kHz, 16bits
	int SetPrimaryBufferWaveFormat(WORD Channels, DWORD SamplesPerSec, WORD BitsPerSample);

	void DuplicateBuffer(DWORD nDuplicate);	//�f���v���P�[�g(FX�ƃX�g���[�~���O�ɂ͎g���Ȃ�)
	void		AllowRapidAccess(bool isAllow);}
;
#endif