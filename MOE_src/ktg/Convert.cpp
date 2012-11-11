#include "stdafx.h"
#include "common.h"
#include "Convert.h"

extern CWindowGL win;
extern KTextureEdit ktex;
extern HWND g_hDlg;
UINT tex_name = NULL;
char g_szFinalTexture[TEXT_BUFFER_SIZE];
int g_ErrReason;
int g_ErrSymbol = -1;
enum g_eErr{E_NO_ERR, E_LESS_ARGUMENT, E_OVER_ARGUMENT, E_WRONG_NUMBER_OF_ARGUMENT, E_NOT_FOUND_SYMBOL, E_NOT_FOUND_OPENED_ARC, E_NOT_FOUND_CLOSED_ARC, E_UNEXPECTED_EOS, E_INCORRECT_FUNCCALL};

//�l��ǂݍ���ł���
BOOL GetValueStr(char** ppChar, char* pRecv, DWORD nRecvSize)
{
	ZeroMemory(pRecv, nRecvSize);

	bool isText = false;
	//ppChar�̃T�C�Y��4�o�C�g�Ȃ̂�,���ʂ�++���Ă��܂��ƕςȏꏊ���w��.
	for(int i=0;; i++, *ppChar+=sizeof(char)*sizeof('a'))
	{
		if(**ppChar=='"'){
			isText = !isText;
			if(*(*ppChar-sizeof(char)*sizeof('a'))=='\\' && *(*ppChar-(sizeof(char)*sizeof('a'))*2)!='\\' && !isText)	isText = !isText;
		}
		if(!isText){
			//','�������邩, �I�[��')'���������炻���܂ł̏��𕶎���ŕԋp����.
			if(**ppChar==','){
				pRecv[i] = '\0';
				
				if(*(*ppChar+sizeof(char)*sizeof('a'))=='\0'){
					g_ErrReason = E_NOT_FOUND_CLOSED_ARC;
					//SAFE_FREE(*ppRecv);
					return FALSE; //�I�[��)�ŏI����Ă��Ȃ�.
				}
				if(i==0){
					g_ErrReason = E_WRONG_NUMBER_OF_ARGUMENT;
					//SAFE_FREE(*ppRecv);
					return FALSE; //(,�����,,�̏ꍇ
				}
				
				*ppChar+=sizeof(char)*sizeof('a'); //���̕������w���Ă���.
				break;
			}else if((**ppChar==')')){
				if((*(*ppChar+sizeof(char)*sizeof('a'))=='\0')){ //)\0�̏ꍇ, �܂萳��Ɋ֐����Ăяo����Ă���ꍇ.
					pRecv[i] = '\0';
					if(i==0){
						g_ErrReason = E_LESS_ARGUMENT;
						//SAFE_FREE(*ppRecv);
						return FALSE; //()��,)�̏ꍇ
					}
					
					*ppChar+=sizeof(char)*sizeof('a'); //���̕������w���Ă���.
					break;
				}else{ //))�Ƃ�.
					g_ErrReason = E_INCORRECT_FUNCCALL;
					//SAFE_FREE(*ppRecv);
					return FALSE;
				}
			}else if(**ppChar=='\0'){
				////")\0"�͈ꌩ���킾���A���̊֐����Ă΂��̂̓f�[�^�ǂݍ��݂̗v�����������Ƃ��ł����āA
				////���Ȃ킿�����ɓ���Ƃ������Ƃ͋L�q����Ă���f�[�^�������֐����v������f�[�^���������Ƃ������Ƃ��Ӗ�����̂ŁA
				////�G���[�����ɂ��ׂ��ł���B
				if((*(*ppChar-sizeof(char)*sizeof('a'))==')')){ //")\0���ǂ����`�F�b�N����"
					g_ErrReason = E_LESS_ARGUMENT;
				}else{
					g_ErrReason = E_NOT_FOUND_CLOSED_ARC;	//")\0"�ł͂Ȃ��Ƃ������Ƃ�, )��������O�ɏI�[���������ꂽ�Ƃ�������.
				}
				//SAFE_FREE(*ppRecv);
				return FALSE;
			}
		}
		if(i>=PARSER_BUFFER_LENGTH){
			g_ErrReason = E_UNEXPECTED_EOS;
			//SAFE_FREE(*ppRecv);
			return FALSE;
		}
		pRecv[i] = **ppChar; //�������
	}
	return TRUE;
	//return *ppRecv;
	//static char* string = NULL;
	//if(string)
	//{
	//	GlobalFree(string);
	//	string = NULL;
	//}
	//string = (char*)GlobalAlloc(GPTR, sizeof(char) * 256);

	////ppChar�̃T�C�Y��4�o�C�g�Ȃ̂�,���ʂ�++���Ă��܂��ƕςȏꏊ���w��.
	//for(int i=0;; i++, *ppChar+=sizeof(char)*sizeof('a'))
	//{
	//	//','�������邩, �I�[��')'���������炻���܂ł̏��𕶎���ŕԋp����.
	//	if(**ppChar==',' || (**ppChar==')' && *(*ppChar+sizeof(char)*sizeof('a'))=='\0'))
	//	{
	//		string[i] = '\0';
	//		if(i==0) return NULL; //()�̏ꍇ
	//		*ppChar+=sizeof(char)*sizeof('a'); //���̕������w���Ă���.
	//		break;
	//	}
	//	string[i] = **ppChar; //�������
	//}
	//return string;
}

char* szRemoveCommentBuffer = NULL;
TCHAR* szTextureFormula = NULL;
TCHAR* szFinalTexture = NULL;
char* swap_buf = NULL;
char* num_str = NULL;
void InitConverter(){
	szTextureFormula = (TCHAR*)GlobalAlloc(GPTR, sizeof(TCHAR) * TEXT_BUFFER_SIZE);
	szFinalTexture = (TCHAR*)GlobalAlloc(GPTR, sizeof(TCHAR) * TEXT_BUFFER_SIZE);
	swap_buf = (char*)GlobalAlloc(GPTR, sizeof(char) * TEXT_BUFFER_SIZE);
	num_str = (char*)GlobalAlloc(GPTR, sizeof(char) * PARSER_BUFFER_LENGTH);
	szRemoveCommentBuffer = (TCHAR*)GlobalAlloc(GPTR, sizeof(TCHAR) * TEXT_BUFFER_SIZE);
}

void DeInitConverter(){
	GlobalFree(szTextureFormula);
	GlobalFree(szFinalTexture);
	GlobalFree(swap_buf);
	GlobalFree(num_str);
	GlobalFree(szRemoveCommentBuffer);
}

//�e�N�X�`���X�V
int UpdateTexture(const char* szTexture)
{
	if(szTexture==NULL || szTexture[0]=='\0'){
		g_ErrReason = E_UNEXPECTED_EOS;
		g_ErrSymbol = -1;
		return FALSE;
	}

	ZeroMemory(szTextureFormula, TEXT_BUFFER_SIZE);
	lstrcpy(szTextureFormula, szTexture); //���ڒl��������킯�ɂ͂����Ȃ�(�����\�L�ɕϊ����Ă��܂��ƃR�����g�Ȃǂ����ׂď����邩��)
		//-------------------------------------------------------------------------
		//C++�`���̃R�����g���폜
		RemoveCPPCommentFromString(szTextureFormula);
		//C�`���̃R�����g���폜
		RemoveCCommentFromString(szTextureFormula);
		//' '&&'\t'���폜
		RemoveCharFromString(szTextureFormula, ' ' , '"');
		RemoveCharFromString(szTextureFormula, '\t', '"');
		RemoveCaridgeReturnFromString(szTextureFormula);
		////'\r\n'���폜
		//RemoveCharFromString(szTextureFormula, '\r');
		//RemoveCharFromString(szTextureFormula, '\n');
		//-------------------------------------------------------------------------
        //�����œ����`���ɕϊ����Ă��
		//���ׂĂ̊֐��͏����ŊǗ������.
		//�Ǘ�������'A'����n�܂�, 'z'���I�[�Ƃ���
		//�G���R�[�h�́A�ȉ��̂Ƃ���
		//(����,����1,����2;����....)

		if(tex_name!=0){
			//glDeleteTextures(1, &tex_name);
			tex_name = 0;
		}
	ZeroMemory(szFinalTexture , TEXT_BUFFER_SIZE);
	for(int i=-1; szTextureFormula[0]!='\0';) //szTextureFormula�ɂ�1�s�ɐ��`���ꂽ�e�L�X�g�f�[�^���i�[����Ă���
	{
		//���̃p�[�X�|�C���g��T��(TOKEN)
		char* p = &szTextureFormula[0];
		int parse_point = 0;
		bool isText = false;
		while(1)
		{
			if(*p=='"')
			{
				isText = !isText;
				//avoid "\"" problem
				if(*(p-1)=='\\' && *(p-2)!='\\' && !isText)	isText = !isText;
			}
			if(*p == '\0'){
				g_ErrReason = E_UNEXPECTED_EOS;
				g_ErrSymbol = -1;
				//GlobalFree(szFinalTexture);
				//GlobalFree(szTextureFormula);
				return FALSE;
			}
			if(!isText && *p==TOKEN) break; //�e�L�X�g����:�ɂ͔������Ȃ�����
			p++;
			parse_point++;
		}
		szTextureFormula[parse_point] = '\0';	//�߂Ă���

		//�ǂ�Ńp�[�X���ׂ����T������
		int found_index = -1;
		int j;
		for(j=0; j<parser_num; j++)
		{
			char* cp = NULL;
			if(0 == strnicmp(szTextureFormula, parser[j], lstrlen(parser[j])))
			{
				found_index = j;
				break;
			}else if(0 == strnicmp(szTextureFormula, "texturesize", lstrlen("texturesize"))){
				found_index = -2;//�T�C�Y�w��q�͒ʏ�̏����ł͂Ȃ��B
				break;			
			}
		}
		if(found_index==-1){
			g_ErrReason = E_NOT_FOUND_SYMBOL;
			g_ErrSymbol = -1;
			//GlobalFree(szFinalTexture);
			//GlobalFree(szTextureFormula);
			return FALSE; //������������
		}

		//���̎��_�ŁA�֐��̖��O�͊m�肵�Ă���
		szFinalTexture[++i] = found_index+'A'; //����ŁA�������Z�[�u����
		szFinalTexture[++i] = ','; //�J���}��}��(�֐����ƈ����̋�ʂ̂���)

		if(found_index>=0){
			//�f�[�^�ǂݎ�菀��
			char* c=szTextureFormula;
			c+=lstrlen(parser[found_index]);
			if(*c=='(') c++;		//'('��j��
			else{
				g_ErrReason = E_NOT_FOUND_OPENED_ARC;
				g_ErrSymbol = found_index;
				//GlobalFree(szFinalTexture);
				//GlobalFree(szTextureFormula);
				return FALSE;		//'('���Ȃ�.���@���.
			}

			if(lstrlen(parser_val_type[found_index])!=0 && //����������
				(*c)==')'){
					g_ErrReason = E_LESS_ARGUMENT;
					g_ErrSymbol = found_index;
					//GlobalFree(szFinalTexture);
					//GlobalFree(szTextureFormula);
					return FALSE; //�������Ȃ���������.
			}

			char* test = c;
			//�����̐����������Ȃ����`�F�b�N
			for(j=0; ; j++) //lstrlen()�����ł���
			{
				if(parser_val_type[found_index][0]=='v') break;
				//char* dummy = NULL;
				//GetValueStr(&test, &dummy);
				//if(dummy==NULL) break;
				//SAFE_FREE(dummy);
				BOOL isOK = GetValueStr(&test, num_str, (DWORD)GlobalSize(num_str));
				if(!isOK){
					break;
				}

				if(j>=lstrlen(parser_val_type[found_index])){ //��������
					g_ErrReason = E_OVER_ARGUMENT;
					g_ErrSymbol = found_index;
					//GlobalFree(szFinalTexture);
					//GlobalFree(szTextureFormula);
					return FALSE;
				}
			}

			if(parser_val_type[found_index][0]=='v'){ //�ϒ��̏ꍇ
				//char* num_str = NULL;
				BOOL isOK = GetValueStr(&c, num_str, (DWORD)GlobalSize(num_str));
				if(!isOK){
					g_ErrSymbol = found_index;
					//GlobalFree(szFinalTexture);
					//GlobalFree(szTextureFormula);
					return FALSE;
				}
				int cnt=StrToInt(num_str);
				int k;
				for(k=0,i++;k<lstrlen(num_str);k++,i++)
				{
					szFinalTexture[i] = num_str[k];
				}
				szFinalTexture[i] = ',';
				//SAFE_FREE(num_str);

				for(j=0; j<cnt*(parser_val_type[found_index][1]-'0'); j++) //�������X�g��2�Ԗڂ��ǂݍ���
				{
					BOOL isOK = GetValueStr(&c, num_str, (DWORD)GlobalSize(num_str));
					if(!isOK){
						g_ErrSymbol = found_index;
						//GlobalFree(szFinalTexture);
						//GlobalFree(szTextureFormula);
						return FALSE;
					}
					//istack[j+1]=StrToInt(num_str);
					for(k=0,i++;k<lstrlen(num_str);k++,i++)
					{
						szFinalTexture[i] = num_str[k];
					}
					szFinalTexture[i] = ',';
					//SAFE_FREE(num_str);
				}
			}else{
				for(j=0; j<lstrlen(parser_val_type[found_index]); j++) //lstrlen()�����ł���
				{
					BOOL isOK = GetValueStr(&c, num_str, (DWORD)GlobalSize(num_str));
					if(!isOK){
						g_ErrSymbol = found_index;
						//GlobalFree(szFinalTexture);
						//GlobalFree(szTextureFormula);
						return FALSE;
					}

					if(lstrcmpi(parser[found_index], "changechannel")==0 || lstrcmpi(parser[found_index], "channel")==0){//CONSTANT
						DWORD dwChannel = 0;
						if(StrStrI(num_str, "R"))	dwChannel |=KT_R;
						if(StrStrI(num_str, "G"))	dwChannel |=KT_G;
						if(StrStrI(num_str, "B"))	dwChannel |=KT_B;
						if(StrStrI(num_str, "A"))	dwChannel |=KT_A;
						if((dwChannel/10)!=0) szFinalTexture[++i] = (TCHAR)(dwChannel/10)+'0';
						szFinalTexture[++i] = (TCHAR)(dwChannel%10)+'0';
						szFinalTexture[++i] = ',';
					}else{
						int k;
						for(k=0,i++;k<lstrlen(num_str);k++,i++)	szFinalTexture[i] = num_str[k];
						szFinalTexture[i] = ',';
					}
					//SAFE_FREE(num_str);
				}
			}
		}else if(found_index<=-2){
			//�f�[�^�ǂݎ�菀��
			char* c=szTextureFormula;
			c+=lstrlen("texturesize");
			if(*c=='(') c++;		//'('��j��
			else{
				g_ErrReason = E_NOT_FOUND_OPENED_ARC;
				g_ErrSymbol = found_index;
				return FALSE;		//'('���Ȃ�.���@���.
			}

			if(lstrlen("ii")!=0 && //����������
				(*c)==')'){
					g_ErrReason = E_LESS_ARGUMENT;
					g_ErrSymbol = found_index;
					return FALSE; //�������Ȃ���������.
			}

			char* test = c;
			//�����̐����������Ȃ����`�F�b�N
			for(j=0; ; j++) //lstrlen()�����ł���
			{
				BOOL isOK = GetValueStr(&test, num_str, (DWORD)GlobalSize(num_str));
				if(!isOK){
					break;
				}

				if(j>=lstrlen("ii")){ //��������
					g_ErrReason = E_OVER_ARGUMENT;
					g_ErrSymbol = found_index;
					return FALSE;
				}
			}

			for(j=0; j<lstrlen("ii"); j++)
			{
				BOOL isOK = GetValueStr(&c, num_str, (DWORD)GlobalSize(num_str));
				if(!isOK){
					g_ErrSymbol = found_index;
					return FALSE;
				}
				int k;
				for(k=0,i++;k<lstrlen(num_str);k++,i++)	szFinalTexture[i] = num_str[k];
				szFinalTexture[i] = ',';
			}			
		}
		//szFinalTexture[i] = '\0';
		szFinalTexture[i] = ';';

		//���̃��[�v�̂��߂�,�g�p����������������
		szTextureFormula[parse_point] = TOKEN;
		int limit = lstrlen(szTextureFormula)+1;
		ZeroMemory(swap_buf, TEXT_BUFFER_SIZE);
		lstrcpy(swap_buf, szTextureFormula);
		for(j=0; j<limit - parse_point; j++)
		{
			szTextureFormula[j] = swap_buf[j+(parse_point+1)];
		}
		//SAFE_FREE(swap_buf);
		//MessageBox(NULL, szTextureFormula, 0, 0);
	}
	//�_�C�A���O�ւ̕\��
	int nFinalTextureSize = lstrlen(szFinalTexture)+1;
	char string[1024];
	wsprintf(string, "text length = %ld bytes. file length = %ld bytes.\n(includes:'\\0', excluded:'\\r\\n',' ','\\t',\"//comments\")", lstrlen(szTexture)+1, nFinalTextureSize);
	SetDlgItemText(g_hDlg, IDC_BYTES, string);
	if(nFinalTextureSize>=1024*64){
		MessageBox(NULL, "UNEXPCTED CODE EXECUTED.\nTECH. INFO: Convert.cpp::UpdateTexture()::CheckingFinalTextureSize", "FIXME!", MB_OK|MB_ICONERROR);
	}

    //win.CMessageBox(szFinalTexture);
	lstrcpy(g_szFinalTexture, szFinalTexture);
	glDeleteTextures(1, &tex_name);
	ktex.SetTextureSize(256, 256);
	ktex.GenerateTextureIndirect(&tex_name, szFinalTexture);

	//SAFE_FREE(szFinalTexture);
	//SAFE_FREE(szTextureFormula);
	g_ErrReason = 0;
	g_ErrSymbol = -1;

	InvalidateRect(hMainWnd, NULL, FALSE);
	return TRUE;
}

//�֐��� : RemoveCPPCommentFromString()
//�@�\   : �w�肳�ꂽ�����񂩂�, C++�X�^�C���̃R�����g�̍s���폜���đO���ɂ߂Ă���.
int RemoveCPPCommentFromString(char* szSrc, int* ctrf)
{
	ZeroMemory(szRemoveCommentBuffer, GlobalSize(szRemoveCommentBuffer));
	//int diff=0;
	////char str[128] = {0};
	////char a[5096];
	////lstrcpy(a, szSrc);
	//for(int i=0; i<lstrlen(szSrc); i++)
	//{
	//	for(;;)
	//	{
	//		if(szSrc[i+diff]=='/' && szSrc[i+diff+1]=='/')// || szSrc[i+diff-1]=='/' && szSrc[i+diff]=='/')
	//		{
	//			for(;;)
	//			{
	//				//�ʏ�̍s������
	//				if(szSrc[i+diff]=='\r' && szSrc[i+diff+1]=='\n')
	//				{
	//					diff+=2;
	//					break;
	//				}
	//				//�e�L�X�g���P�s�����Ȃ������ꍇ
	//				else if(szSrc[i+diff] == '\0') break;
	//				else diff++;	//�|�C���^��i�߂�.
	//			}
	//		}else break;
	//	}
	//	//str[i] = szSrc[i+diff];
	//	szSrc[i] = szSrc[i+diff];
	//}
	int diff=0;
	////char str[128] = {0};
	////char a[5096];
	////lstrcpy(a, szSrc);
	//char* szRemoveCommentBuffer = (char*)GlobalAlloc(GPTR, sizeof(char)*(lstrlen(szSrc)+1));
	////lstrcpy(szRemoveCommentBuffer, szSrc);
	bool isText = false;
	int tmp;
	if(ctrf==NULL){
		ctrf = &tmp;
	}
	*ctrf = 0;
	for(int i=0; i<lstrlen(szSrc); i++)
	{
		for(;;)
		{
			if(szSrc[i+diff]=='"'){
				isText = !isText;
			}
			if(!isText){
				if(szSrc[i+diff]=='/' && szSrc[i+diff+1]=='/')//// || szSrc[i+diff-1]=='/' && szSrc[i+diff]=='/')
				{
					for(;;)
					{
						//�ʏ�̍s������
						if(szSrc[i+diff]=='\r' && szSrc[i+diff+1]=='\n')
						{
							*ctrf+=2;
							diff+=2;
							break;
						}
						//�e�L�X�g���P�s�����Ȃ������ꍇ
						else if(szSrc[i+diff] == '\0') break;
						else diff++;	//�|�C���^��i�߂�.
					}
				}else break;
			}else{
				break;//�e�L�X�g���̏ꍇ�̓|�C���^��i�߂�
			}
		}
		////str[i] = szSrc[i+diff];
		szRemoveCommentBuffer[i] = szSrc[i+diff];
	}
	lstrcpy(szSrc, szRemoveCommentBuffer);
	//GlobalFree(szRemoveCommentBuffer);
	return diff;
}

//�֐��� : RemoveCCommentFromString()
//�@�\   : �w�肳�ꂽ�����񂩂�, C�X�^�C���̃R�����g�͈̔͂��폜���đO���ɂ߂Ă���.
int RemoveCCommentFromString(char* szSrc, int* ctrf)
{
	ZeroMemory(szRemoveCommentBuffer, GlobalSize(szRemoveCommentBuffer));
	int diff=0;
	//char* szRemoveCommentBuffer = (char*)GlobalAlloc(GPTR, sizeof(char)*(lstrlen(szSrc)+1));
	bool isText = false;
	int tmp;
	if(ctrf==NULL){
		ctrf = &tmp;
	}
	*ctrf = 0;
	for(int i=0; i<lstrlen(szSrc); i++)
	{
		for(;;)
		{
			if(szSrc[i+diff]=='"'){
				isText = !isText;
			}
			if(!isText){
				if(szSrc[i+diff]=='/' && szSrc[i+diff+1]=='*')
				{
					for(;;)
					{
						//�ʏ�̍s������
						if(szSrc[i+diff]=='*' && szSrc[i+diff+1]=='/')
						{
							*ctrf+=2;
							diff+=2;
							break;
						}
						//�e�L�X�g���P�s�����Ȃ������ꍇ
						else if(szSrc[i+diff] == '\0') break;
						else diff++;	//�|�C���^��i�߂�.
					}
				}else break;
			}else{
				break;//�e�L�X�g���̏ꍇ�̓|�C���^��i�߂�
			}
		}
		szRemoveCommentBuffer[i] = szSrc[i+diff];
	}
	lstrcpy(szSrc, szRemoveCommentBuffer);
	//GlobalFree(szRemoveCommentBuffer);
	return diff;
}

//�֐��� : RemoveCharFromString()
//�@�\   : �w�肳�ꂽ�����񂩂�, �w�肳�ꂽ�������폜���đO���ɂ߂Ă���.
//���l   : Exception�Ŏw�肳�ꂽ�����ň͂܂ꂽ��Ԃɂ����Ă�, Target�͖��������.
//         ���m�̃o�O�Ƃ���,"\"\""��""�Ƃ��Ă݂Ȃ���Ȃ����Ƃ���������(KTexture���őΏ�).
void RemoveCharFromString(char* szSrc, char Target, char Exception)
{
	ZeroMemory(szRemoveCommentBuffer, GlobalSize(szRemoveCommentBuffer));
	int len = lstrlen(szSrc)+1;
	int diff=0;
	bool isException = false;
	//char* szRemoveCommentBuffer = (char*)GlobalAlloc(GPTR, sizeof(char)*len);
	////lstrcpy(szRemoveCommentBuffer, szSrc);

	for(int i=0; i<len; i++)
	{
		if(Exception>=0 && szSrc[i+diff]==Exception){
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
	//GlobalFree(szRemoveCommentBuffer);
}

//�֐��� : RemoveCaridgeReturnFromString()
//�@�\   : �w�肳�ꂽ�����񂩂�, \r\n���폜.
//���_ : ���{���Ή�
int RemoveCaridgeReturnFromString(char* szSrc)
{
	ZeroMemory(szRemoveCommentBuffer, GlobalSize(szRemoveCommentBuffer));
	int len = lstrlen(szSrc)+1;
	int diff=0;
	//char* szRemoveCommentBuffer = (char*)GlobalAlloc(GPTR, sizeof(char)*len);
	////lstrcpy(szRemoveCommentBuffer, szSrc);

	for(int i=0; i<len; i++)
	{
		for(bool isEnd = false; isEnd != true; )
		{
			if(szSrc[i+diff+0]=='\r' && szSrc[i+diff+1]=='\n'){
				 diff+=2;
			}else if(szSrc[i+diff]=='\n' && szSrc[(i<0) ? 0:i-1+diff]!='\r'){
				 diff+=1;
			}else isEnd=true;
		}
		szRemoveCommentBuffer[i] = szSrc[i+diff];
	}
	lstrcpy(szSrc, szRemoveCommentBuffer);
	//GlobalFree(szRemoveCommentBuffer);
	return diff;
}

//void MakeEscapeSequenceFromString(char* szSrc)
//{
//	int len = lstrlen(szSrc)+1;
//	int diff=0;
//	//const char srcesc_seq[5][2] = { \\", "\t" };
//	//const char dstesc_seq[5]]   = { '\\',    };
//	char* tmp = (char*)GlobalAlloc(GPTR, sizeof(char)*len);
//	//lstrcpy(tmp, szSrc);
//
//	for(int i=0; i<len; i++)
//	{
//		if(szSrc[i+diff+0]=='\\'){
//			if(szSrc[i+diff+1]=='\\'){
//				tmp[i] = '\\';
//			}else if(szSrc[i+diff+1]=='t'){
//				tmp[i] = '\t';
//			}else if(szSrc[i+diff+1]=='n'){
//				tmp[i] = '\n';
//			}else if(szSrc[i+diff+1]=='"'){
//				tmp[i] = '"';
//			}
//			diff+=1;
//		}else{
//			tmp[i] = szSrc[i+diff];
//		}
//	}
//	lstrcpy(szSrc, tmp);
//	GlobalFree(tmp);
//}