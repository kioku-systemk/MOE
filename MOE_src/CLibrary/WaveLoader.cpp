#include "stdafx.h"
#include "./WaveLoader.h"
#pragma warning(disable:4244)

WaveLoader::WaveLoader(){
	m_hmmio = NULL;
	m_dwOffsetToWaveData = 0;
}

WaveLoader::~WaveLoader(){
	this->QueryFree();
}

int WaveLoader::QueryLoadFile(const char* szFileName, void* pMemData, DWORD dwMembufferSize){
	int err = this->CSoundLoader::QueryLoadFile(szFileName);
	if(err!=CSL_E_OK) return err;

	m_dwDataLength = 0;
	m_dwCurrentDecodedPos = 0;

	//Set CSL_FILEINFO
	m_FileInfo->cbsize = sizeof(CSL_FILEINFO);
	lstrcpy(m_FileInfo->name, szFileName);
	//char* str = (char*)GlobalAlloc(GPTR, lstrlen(szFileName)+1);
	//_ASSERT(str);
	//	lstrcpy(str, szFileName);
	//	PathRemoveFileSpec(str);
	//	lstrcpy(m_FileInfo->path, str);

	//	lstrcpy(str, szFileName);
	//	PathStripPath(str);
	//	lstrcpy(m_FileInfo->name, str);
	//  GlobalFree(str); 

	if( lstrcmp(m_FileInfo->name, CSL_LOAD_MEMORYIMAGE)!=0 ) {
		HFILE hFile = (HFILE)CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		//_ASSERT(hFile);
//#if _MSC_VER >= 1310
//		GetFileSizeEx((HANDLE)hFile, (PLARGE_INTEGER)&m_FileInfo->fsize);
//#else
		m_FileInfo->fsize = GetFileSize((HANDLE)hFile, NULL);
//#endif
		CloseHandle((HANDLE)hFile);
	}else{
		m_FileInfo->fsize = dwMembufferSize;
		m_FileInfo->pMemBuffer = pMemData;
	}
	
	if(CSL_E_OK != ReadWaveData(&m_wfx, NULL)){
		CSL_CleanUp(this);
		return CSL_E_BADFMT;
	}

	this->isLoaded = true;
	return CSL_E_OK;
}

int WaveLoader::QueryFree(){
	//WAVE
	m_dwOffsetToWaveData = 0;
	SAFE_MMIOCLOSE(m_hmmio);
	this->isLoaded = false;

	//SAFE_GLOBALFREE(m_pData);
	return this->CSoundLoader::QueryFree();
}

//WAVE�t�@�C�������[�h����
int WaveLoader::ReadWaveData(WAVEFORMATEX* wfx, void** pdata, QWORD dwFrom, QWORD dwSizeToRead, bool isLoopWave)
{
	static MMCKINFO parent, child;
	static char szBefore[MAX_PATH];
	_ASSERT(m_FileInfo->name);
	if(pdata!=NULL) SAFE_GLOBALFREE(*pdata);

	//if(m_hmmio!=NULL && lstrcmp(m_FileInfo->name, szBefore)!=0){//�t�@�C�������w�肳��Ă�����J���Ȃ���
	//	SAFE_MMIOCLOSE(m_hmmio);
	//}
	if( !isLoaded ){
		lstrcpy(szBefore, m_FileInfo->name);

		if( lstrcmp(m_FileInfo->name, CSL_LOAD_MEMORYIMAGE)==0 ){
//			if(!m_hmmio){//�ŏ��̈�񂵂����Ȃ������i�X�g���[�~���O�p�̏��u�j
			MMIOINFO mmioinfo;
			ZeroMemory(&mmioinfo, sizeof(MMIOINFO));
			mmioinfo.pchBuffer = (HPSTR)m_FileInfo->pMemBuffer;
			mmioinfo.fccIOProc = FOURCC_MEM;
			mmioinfo.cchBuffer = m_FileInfo->fsize;

			if(NULL == (m_hmmio = mmioOpen(NULL, &mmioinfo, MMIO_READ|MMIO_ALLOCBUF))){
				return CSL_E_UNEXP;
			}
		}else{
			if(NULL == (m_hmmio = mmioOpen((LPSTR)m_FileInfo->name, NULL, MMIO_READ|MMIO_ALLOCBUF))){
				return CSL_E_UNEXP;
			}
		}
		parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');//wave�t�@�C�����ǂ������ׂ�
		if(mmioDescend(m_hmmio, &parent, NULL, MMIO_FINDRIFF) != MMSYSERR_NOERROR){
			SAFE_MMIOCLOSE(m_hmmio);
			return CSL_E_UNEXP;
		}
		child.ckid = mmioFOURCC('f', 'm', 't', ' ');//fmt�`�����N�ֈړ�����
		if(mmioDescend(m_hmmio, &child, &parent, MMIO_FINDCHUNK) != MMSYSERR_NOERROR){
			SAFE_MMIOCLOSE(m_hmmio);
			return CSL_E_UNEXP;
		}

		_ASSERT(wfx);
		if(mmioRead(m_hmmio, (HPSTR)wfx, (LONG)child.cksize) != (LONG)child.cksize){//fmt�`�����N(WAVEFORMATEX)�ǂݎ��
			SAFE_MMIOCLOSE(m_hmmio);
			return CSL_E_UNEXP;
		}
		_ASSERT((wfx->wFormatTag==WAVE_FORMAT_PCM));
		mmioAscend(m_hmmio, &child, 0);//fmt�`�����N����o��
		child.ckid = mmioFOURCC('d', 'a', 't', 'a');//data�`�����N�Ɉړ�
		if(mmioDescend(m_hmmio, &child, &parent, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) {
			SAFE_MMIOCLOSE(m_hmmio);
			return CSL_E_UNEXP;
		}
		m_dwDataLength = child.cksize;//WAVE�̈�̃T�C�Y
		m_dwOffsetToWaveData = mmioSeek(m_hmmio, 0, SEEK_CUR);//�f�[�^�܂ł̈ʒu��ۑ����Ă���
	}
	if(pdata){
		if(dwSizeToRead<=0){//�t�@�C���S�̂�ǂݍ���
			(*pdata) = (LPBYTE)GlobalAlloc(GPTR, m_dwDataLength * sizeof(BYTE));
			_ASSERT(*pdata);

			if(mmioRead(m_hmmio, (HPSTR)*pdata, (LONG)m_dwDataLength) != (LONG)m_dwDataLength){
				GlobalFree(*pdata);
				SAFE_MMIOCLOSE(m_hmmio);
				return CSL_E_UNEXP;
			}
			SAFE_MMIOCLOSE(m_hmmio);	//�K�v�ȃf�[�^����������̂Ť�t�@�C�������
			m_dwCurrentDecodedPos += dwSizeToRead;
		}else{
			//DWORD dwInnerFinPos = dwFrom + dwSizeToRead;
			////�̈�T�C�Y�ȏゾ��������܂�悤�ɒl��␳
			//if(m_dwDataLength < dwInnerFinPos){
			//	dwSizeToRead -= dwInnerFinPos-m_dwDataLength;
			//}

			//�J�n�ʒu���w�肳��Ă����, �f�[�^�̈悩��̃I�t�Z�b�g��Start�Ƃ���.
			//�w�肳��Ă��Ȃ����, ����܂Ői�񂾃J�[�\���ʒu����ǂݍ��݂��J�n����
			if(dwFrom>=0) mmioSeek(m_hmmio, (LONG)(m_dwOffsetToWaveData + dwFrom), SEEK_SET);
			
			//�v���̈敪�̃������m��
			(*pdata) = (LPBYTE)GlobalAlloc(GPTR, (SIZE_T)(dwSizeToRead * sizeof(BYTE)));
			_ASSERT(*pdata);

			//���݈ʒu���烊�N�G�X�g�T�C�Y��ǂނƃI�[�o�[����悤�Ȃ�A���b�v�A���E���h����B
			DWORD dwNowCursor = mmioSeek(m_hmmio, 0, SEEK_CUR) - m_dwOffsetToWaveData;
			if(m_dwDataLength < (dwNowCursor + dwSizeToRead)){
				if( !isLoopWave ){
					if( dwNowCursor>=m_dwDataLength ){
						FillMemory((BYTE*)*pdata, dwSizeToRead, (m_wfx.wBitsPerSample==8) ? 128:0);
						return CSL_E_OUTOFRANGE;
					}
				}

				DWORD dwBeforeWrapAround = m_dwDataLength-dwNowCursor; 
				//�Ƃ肠�����A�Ō�܂œǂ�
				if(mmioRead(m_hmmio, (HPSTR)*pdata, (LONG)dwBeforeWrapAround) != (LONG)dwBeforeWrapAround){
					GlobalFree(*pdata);
					SAFE_MMIOCLOSE(m_hmmio);
					return CSL_E_UNEXP;
				}
				m_dwCurrentDecodedPos += dwBeforeWrapAround;

				if(isLoopWave){//�c���������𖳉��Ŗ��߂邩�ǂ���
					mmioSeek(m_hmmio, 0, SEEK_SET);
					mmioSeek(m_hmmio, m_dwOffsetToWaveData, SEEK_CUR);	//�|�C���^��WAVE�̈�n�_�ɖ߂�
					//���b�v�A���E���h����ǂ�
					if(mmioRead(m_hmmio, (HPSTR)*pdata+dwBeforeWrapAround, (LONG)(dwSizeToRead - dwBeforeWrapAround)) != (LONG)(dwSizeToRead - dwBeforeWrapAround)){
						GlobalFree(*pdata);
						SAFE_MMIOCLOSE(m_hmmio);
						return CSL_E_UNEXP;
					}
					m_dwCurrentDecodedPos =  0;
					m_dwCurrentDecodedPos += dwSizeToRead - dwBeforeWrapAround;
				}else{
					FillMemory((BYTE*)*pdata+dwBeforeWrapAround, dwSizeToRead - dwBeforeWrapAround, (m_wfx.wBitsPerSample==8) ? 128:0);
					return CSL_N_FIN;
				}
			}else{ //���b�v�A���E���h���Ȃ������ꍇ
				if(mmioRead(m_hmmio, (HPSTR)*pdata, (LONG)dwSizeToRead) != (LONG)dwSizeToRead){
					GlobalFree(*pdata);
					SAFE_MMIOCLOSE(m_hmmio);
					return CSL_E_UNEXP;
				}
				m_dwCurrentDecodedPos += dwSizeToRead;
			}//�K�v�ȃf�[�^�͂܂�����̂ŁA�t�@�C���͕��Ȃ�
		}
	}
	return CSL_E_OK;
}

int WaveLoader::SetWavePointerPos(DWORD dwPos_byte, int method)
{
	if( isLoaded ){
	//if(m_hmmio){
		m_dwCurrentDecodedPos = dwPos_byte;
		mmioSeek(m_hmmio, dwPos_byte+m_dwOffsetToWaveData, method);
		return CSL_E_OK;
	//}
	}
	return CSL_E_NOTLOADED;
}

int WaveLoader::SetWavePointerMS(DWORD dwMS, int method){
	if( isLoaded ){
		m_dwCurrentDecodedPos = dwMS/1000.0 * (double)m_wfx.nAvgBytesPerSec;
		mmioSeek(m_hmmio, (dwMS/1000.0 * (double)m_wfx.nAvgBytesPerSec) + m_dwOffsetToWaveData, method);
	}
	return CSL_E_OK;
}