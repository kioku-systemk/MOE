#include "stdafx.h"
#include "./OggVorbisLoader.h"
#pragma warning(disable:4244)

//#pragma comment(linker, "/nodefaultlib:libcmt")/* HACK!!! for temporal... fixme! */
#pragma comment(lib, "vorbis_static.lib")
#pragma comment(lib, "ogg_static.lib")
#pragma comment(lib, "vorbisfile_static.lib")

ov_callbacks OggVorbisLoader::m_cbOgg =
{
	OggVorbisLoader::callbackRead,
	OggVorbisLoader::callbackSeek,
	OggVorbisLoader::callbackClose,
	OggVorbisLoader::callbackTell
};

OggVorbisLoader::OggVorbisLoader(){
	m_fp = NULL;
	m_vi = NULL;
	ZeroMemory(&m_vf, sizeof(OggVorbis_File));
}

OggVorbisLoader::~OggVorbisLoader(){
	this->QueryFree();
}

int OggVorbisLoader::QueryLoadFile(const char* szFileName, void* pMemData, DWORD dwMembufferSize){
	int err = this->CSoundLoader::QueryLoadFile(szFileName);
	if(err!=CSL_E_OK) return err;

	m_dwDataLength = 0;
	m_dwCurrentDecodedPos = 0;

	//Set CSL_FILEINFO
	m_FileInfo->cbsize = sizeof(CSL_FILEINFO);
	lstrcpy(m_FileInfo->name, szFileName);

	if( lstrcmp(m_FileInfo->name, CSL_LOAD_MEMORYIMAGE)!=0 ) {
		HFILE hFile = (HFILE)CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		_ASSERT(hFile);
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

int OggVorbisLoader::QueryFree(){
	this->isLoaded = false;
	//OggVorbis
	SAFE_OVCLEAR(m_vf);
	SAFE_FPCLOSE(m_fp);
	ZeroMemory(&m_vf, sizeof(OggVorbis_File));

	//SAFE_GLOBALFREE(m_pData);
	return this->CSoundLoader::QueryFree();
}

int OggVorbisLoader::DecodeOggVorbis(void** pdata, DWORD dwPlusPointer, DWORD dwSizeToRead, int nQualifyBytes){
	//if(!m_fp)	return CSL_E_NOTLOADED;

	int nSizeRead = 0;
	int current_section = 0;
	while(1){
		long ret=ov_read(&m_vf, (char*)*pdata + nSizeRead + dwPlusPointer, dwSizeToRead - nSizeRead, 0, nQualifyBytes, 1, &current_section);
		if(ret == 0){//EOF
			//SAFE_OVCLEAR(m_vf);
			//SAFE_FPCLOSE(m_fp);
			break;
		}else if(ret<0){
			//SAFE_OVCLEAR(m_vf);
			//SAFE_FPCLOSE(m_fp);
			//return CSL_E_UNEXP;
			break;
		}else{
			nSizeRead += ret;
		}
	}
	return CSL_E_OK;
}

//OGG�t�@�C�������[�h����
int OggVorbisLoader::ReadWaveData(WAVEFORMATEX* wfx, void** pdata, QWORD dwFrom, QWORD dwSizeToRead, bool isLoopWave)
{
	static char szBefore[MAX_PATH];
	_ASSERT(m_FileInfo->name);
	if(pdata!=NULL) SAFE_GLOBALFREE(*pdata);

	////����A����񂩁H
	//if(!m_fp && lstrcmp(m_FileInfo->name, szBefore)!=0){
	//	SAFE_OVCLEAR(m_vf);
	//	SAFE_FPCLOSE(m_fp);
	//}

	int nQualify = 16; //�ʎq���r�b�g
	if(!isLoaded){//�ŏ��̈�񂵂����Ȃ������i�X�g���[�~���O�p�̏��u�j
		if( lstrcmp(m_FileInfo->name, CSL_LOAD_MEMORYIMAGE)==0 ){
			ov_open_callbacks( this, &m_vf, NULL, 0, m_cbOgg);
		}else{
			lstrcpy(szBefore, m_FileInfo->name);
			m_fp = fopen(m_FileInfo->name, "rb");
			if(!m_fp) return CSL_E_UNEXP;//_ASSERT(m_fp);
			
			if(ov_open(m_fp, &m_vf, NULL, 0)<0){
				SAFE_FPCLOSE(m_fp);
				return CSL_E_UNEXP;
			}
		}

		_ASSERT(wfx);
		m_vi=ov_info(&m_vf,-1);
		if( !m_vi ){
		}else{
			int nQualify = 16; //�ʎq���r�b�g

			m_dwDataLength = m_vi->channels * m_vi->rate * ov_time_total( &m_vf,-1 ) * nQualify/8; //�f�R�[�h�����Ƃ��̃T�C�Y

			ZeroMemory( wfx, sizeof(WAVEFORMATEX) ); 
			wfx->wFormatTag      = WAVE_FORMAT_PCM; 
			wfx->nChannels       = m_vi->channels;
			wfx->nSamplesPerSec  = m_vi->rate;
			wfx->wBitsPerSample  = nQualify;
			wfx->nBlockAlign     = wfx->wBitsPerSample / 8 * wfx->nChannels;
			wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign;
		}
	}
	if(pdata){
		int ret = 0;
		if(dwSizeToRead<=0){//�t�@�C���S�̂�ǂݍ���
			(*pdata) = (LPBYTE)GlobalAlloc(GPTR, m_dwDataLength * sizeof(BYTE));
			_ASSERT(*pdata);

			ret = this->DecodeOggVorbis(pdata, 0, m_dwDataLength, nQualify/8);
			SAFE_OVCLEAR(m_vf);
			SAFE_FPCLOSE(m_fp);

			m_dwCurrentDecodedPos += dwSizeToRead;
		}else{
			(*pdata) = (LPBYTE)GlobalAlloc(GPTR, dwSizeToRead * sizeof(BYTE));
			_ASSERT(*pdata);

			//�J�n�ʒu���w�肳��Ă����, �f�[�^�̈悩��̃I�t�Z�b�g��Start�Ƃ���.
			//�w�肳��Ă��Ȃ����, ����܂Ői�񂾃J�[�\���ʒu����ǂݍ��݂��J�n����
			if(dwFrom>0){
				//�o�C�g������T���v�����ɕϊ�
				dwFrom = ((double)dwFrom/(double)m_wfx.nAvgBytesPerSec) * (double)m_wfx.nSamplesPerSec;
				ov_pcm_seek(&m_vf, dwFrom);
			}

			//���݈ʒu���烊�N�G�X�g�T�C�Y��ǂނƃI�[�o�[����悤�Ȃ�A���b�v�A���E���h����B
			DWORD dwNowCursor = (DWORD)ov_pcm_tell(&m_vf);
			//�T���v��������o�C�g���ɕϊ�����
			dwNowCursor = dwNowCursor*m_wfx.wBitsPerSample/8*m_wfx.nChannels;
			if(m_dwDataLength < (dwNowCursor + dwSizeToRead)){//���b�v�A���E���h����H
				if( !isLoopWave ){
					if( dwNowCursor>=m_dwDataLength ){
						FillMemory((BYTE*)*pdata, dwSizeToRead, (m_wfx.wBitsPerSample==8) ? 128:0);
						return CSL_E_OUTOFRANGE;
					}
				}

				DWORD dwBeforeWrapAround = m_dwDataLength-dwNowCursor; 
				//�Ƃ肠�����A�Ō�܂œǂ�
				ret = this->DecodeOggVorbis(pdata, 0, dwBeforeWrapAround, nQualify/8);
				m_dwCurrentDecodedPos += dwBeforeWrapAround;

				if(isLoopWave){//�c���������𖳉��Ŗ��߂邩�ǂ���
					ov_pcm_seek(&m_vf, 0);	//���߂�
					ret = this->DecodeOggVorbis(pdata, dwBeforeWrapAround, dwSizeToRead - dwBeforeWrapAround, nQualify/8);
					
					m_dwCurrentDecodedPos =	 0;
					m_dwCurrentDecodedPos += dwSizeToRead - dwBeforeWrapAround;
				}else{
					FillMemory((BYTE*)*pdata+dwBeforeWrapAround, dwSizeToRead - dwBeforeWrapAround, (m_wfx.wBitsPerSample==8) ? 128:0);
					return CSL_N_FIN;
				}
			}else{ //���b�v�A���E���h���Ȃ������ꍇ
				ret = this->DecodeOggVorbis(pdata, 0, dwSizeToRead, nQualify/8);
				m_dwCurrentDecodedPos += dwSizeToRead;
			}//�K�v�ȃf�[�^�͂܂�����̂ŁA�t�@�C���͕��Ȃ�
		}
		if(ret!=CSL_E_OK) return ret;
	}
	return CSL_E_OK;
}

size_t OggVorbisLoader::callbackRead(void* ptr, size_t size, size_t nmemb, void* datasource)
{
	OggVorbisLoader* p = (OggVorbisLoader*)datasource;
	if(!p) return -1;
	if( p->m_FileInfo->dwMemoryCursor >= p->m_FileInfo->fsize ){
		return 0; //EOF
	}
	
	__int64 dwSizeToRead = size*nmemb;
	if( p->m_FileInfo->dwMemoryCursor+dwSizeToRead >= p->m_FileInfo->fsize ){
		dwSizeToRead = p->m_FileInfo->fsize - p->m_FileInfo->dwMemoryCursor;
		CopyMemory(ptr, ((unsigned char*)p->m_FileInfo->pMemBuffer) + p->m_FileInfo->dwMemoryCursor, dwSizeToRead);
		p->m_FileInfo->dwMemoryCursor = p->m_FileInfo->fsize;//+= dwSize;
		return dwSizeToRead / size;
	}
	
	CopyMemory(ptr, ((unsigned char*)p->m_FileInfo->pMemBuffer) + p->m_FileInfo->dwMemoryCursor, dwSizeToRead);
	p->m_FileInfo->dwMemoryCursor += dwSizeToRead;
	return nmemb;
}

int OggVorbisLoader::callbackSeek(void* datasource, ogg_int64_t offset, int whence)
{
	OggVorbisLoader* p = (OggVorbisLoader*)datasource;
	if(!p) return -1;

    //if (offset < 0) {
    //    return -1;
    //}
	__int64 Newpos;
    switch (whence) {
        case SEEK_SET:
            Newpos = offset;
            break;
        case SEEK_CUR:
            Newpos = p->m_FileInfo->dwMemoryCursor + offset;
            break;
        case SEEK_END:
			Newpos = p->m_FileInfo->fsize + offset;
            break;
        default:
            return -1;
    }
    if (Newpos < 0) return -1;
    p->m_FileInfo->dwMemoryCursor = Newpos;

	return 0;
}

int OggVorbisLoader::callbackClose(void* datasource)
{
	OggVorbisLoader* p = (OggVorbisLoader*)datasource;
	if(!p) return -1;

	return 0;
}

long OggVorbisLoader::callbackTell(void* datasource)
{
	OggVorbisLoader* p = (OggVorbisLoader*)datasource;
	if(!p) return -1;

	return p->m_FileInfo->dwMemoryCursor;
}

int OggVorbisLoader::SetWavePointerPos(DWORD dwPos_byte, int method)
{
	if( isLoaded ){
	//if(m_vf.vi!=NULL){
		m_dwCurrentDecodedPos = dwPos_byte;
		//�o�C�g������T���v�����ɕϊ�
		DWORD dwa = (dwPos_byte / (double)m_wfx.nAvgBytesPerSec) * m_wfx.nSamplesPerSec;
		if( ov_pcm_seek(&m_vf, (__int64)dwa)!=0 ){
			FatalAppExit(0, "");
		}
	//}
	}
	return CSL_E_OK;
}

int OggVorbisLoader::SetWavePointerMS(DWORD dwMS, int method){
	if( isLoaded ){
		m_dwCurrentDecodedPos = dwMS/1000.0 * (double)m_wfx.nAvgBytesPerSec;
		ov_pcm_seek(&m_vf, dwMS * 1000 * m_wfx.nSamplesPerSec);
	}
	return CSL_E_OK;
}