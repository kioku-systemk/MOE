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

//値を読み込んでいく
BOOL GetValueStr(char** ppChar, char* pRecv, DWORD nRecvSize)
{
	ZeroMemory(pRecv, nRecvSize);

	bool isText = false;
	//ppCharのサイズは4バイトなので,普通に++してしまうと変な場所を指す.
	for(int i=0;; i++, *ppChar+=sizeof(char)*sizeof('a'))
	{
		if(**ppChar=='"'){
			isText = !isText;
			if(*(*ppChar-sizeof(char)*sizeof('a'))=='\\' && *(*ppChar-(sizeof(char)*sizeof('a'))*2)!='\\' && !isText)	isText = !isText;
		}
		if(!isText){
			//','を見つけるか, 終端の')'を見つけたらそこまでの情報を文字列で返却する.
			if(**ppChar==','){
				pRecv[i] = '\0';
				
				if(*(*ppChar+sizeof(char)*sizeof('a'))=='\0'){
					g_ErrReason = E_NOT_FOUND_CLOSED_ARC;
					//SAFE_FREE(*ppRecv);
					return FALSE; //終端が)で終わっていない.
				}
				if(i==0){
					g_ErrReason = E_WRONG_NUMBER_OF_ARGUMENT;
					//SAFE_FREE(*ppRecv);
					return FALSE; //(,や引数,,の場合
				}
				
				*ppChar+=sizeof(char)*sizeof('a'); //次の文字を指しておく.
				break;
			}else if((**ppChar==')')){
				if((*(*ppChar+sizeof(char)*sizeof('a'))=='\0')){ //)\0の場合, つまり正常に関数が呼び出されている場合.
					pRecv[i] = '\0';
					if(i==0){
						g_ErrReason = E_LESS_ARGUMENT;
						//SAFE_FREE(*ppRecv);
						return FALSE; //()や,)の場合
					}
					
					*ppChar+=sizeof(char)*sizeof('a'); //次の文字を指しておく.
					break;
				}else{ //))とか.
					g_ErrReason = E_INCORRECT_FUNCCALL;
					//SAFE_FREE(*ppRecv);
					return FALSE;
				}
			}else if(**ppChar=='\0'){
				////")\0"は一見正常だが、この関数が呼ばれるのはデータ読み込みの要求があったときであって、
				////すなわちここに入るということは記述されているデータ数よりも関数が要求するデータ数が多いということを意味するので、
				////エラー扱いにすべきである。
				if((*(*ppChar-sizeof(char)*sizeof('a'))==')')){ //")\0かどうかチェックする"
					g_ErrReason = E_LESS_ARGUMENT;
				}else{
					g_ErrReason = E_NOT_FOUND_CLOSED_ARC;	//")\0"ではないということは, )が見つかる前に終端が発見されたということ.
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
		pRecv[i] = **ppChar; //順次代入
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

	////ppCharのサイズは4バイトなので,普通に++してしまうと変な場所を指す.
	//for(int i=0;; i++, *ppChar+=sizeof(char)*sizeof('a'))
	//{
	//	//','を見つけるか, 終端の')'を見つけたらそこまでの情報を文字列で返却する.
	//	if(**ppChar==',' || (**ppChar==')' && *(*ppChar+sizeof(char)*sizeof('a'))=='\0'))
	//	{
	//		string[i] = '\0';
	//		if(i==0) return NULL; //()の場合
	//		*ppChar+=sizeof(char)*sizeof('a'); //次の文字を指しておく.
	//		break;
	//	}
	//	string[i] = **ppChar; //順次代入
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

//テクスチャ更新
int UpdateTexture(const char* szTexture)
{
	if(szTexture==NULL || szTexture[0]=='\0'){
		g_ErrReason = E_UNEXPECTED_EOS;
		g_ErrSymbol = -1;
		return FALSE;
	}

	ZeroMemory(szTextureFormula, TEXT_BUFFER_SIZE);
	lstrcpy(szTextureFormula, szTexture); //直接値をいじるわけにはいかない(内部表記に変換してしまうとコメントなどがすべて消えるから)
		//-------------------------------------------------------------------------
		//C++形式のコメントを削除
		RemoveCPPCommentFromString(szTextureFormula);
		//C形式のコメントを削除
		RemoveCCommentFromString(szTextureFormula);
		//' '&&'\t'を削除
		RemoveCharFromString(szTextureFormula, ' ' , '"');
		RemoveCharFromString(szTextureFormula, '\t', '"');
		RemoveCaridgeReturnFromString(szTextureFormula);
		////'\r\n'を削除
		//RemoveCharFromString(szTextureFormula, '\r');
		//RemoveCharFromString(szTextureFormula, '\n');
		//-------------------------------------------------------------------------
        //ここで内部形式に変換してやる
		//すべての関数は序数で管理される.
		//管理序数は'A'から始まり, 'z'を終端とする
		//エンコードは、以下のとおり
		//(序数,引数1,引数2;序数....)

		if(tex_name!=0){
			//glDeleteTextures(1, &tex_name);
			tex_name = 0;
		}
	ZeroMemory(szFinalTexture , TEXT_BUFFER_SIZE);
	for(int i=-1; szTextureFormula[0]!='\0';) //szTextureFormulaには1行に整形されたテキストデータが格納されている
	{
		//次のパースポイントを探す(TOKEN)
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
			if(!isText && *p==TOKEN) break; //テキスト中の:には反応しないため
			p++;
			parse_point++;
		}
		szTextureFormula[parse_point] = '\0';	//つめておく

		//どれでパースすべきか探索する
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
				found_index = -2;//サイズ指定子は通常の序数ではない。
				break;			
			}
		}
		if(found_index==-1){
			g_ErrReason = E_NOT_FOUND_SYMBOL;
			g_ErrSymbol = -1;
			//GlobalFree(szFinalTexture);
			//GlobalFree(szTextureFormula);
			return FALSE; //何かおかしい
		}

		//この時点で、関数の名前は確定している
		szFinalTexture[++i] = found_index+'A'; //これで、序数をセーブする
		szFinalTexture[++i] = ','; //カンマを挿入(関数名と引数の区別のため)

		if(found_index>=0){
			//データ読み取り準備
			char* c=szTextureFormula;
			c+=lstrlen(parser[found_index]);
			if(*c=='(') c++;		//'('を破棄
			else{
				g_ErrReason = E_NOT_FOUND_OPENED_ARC;
				g_ErrSymbol = found_index;
				//GlobalFree(szFinalTexture);
				//GlobalFree(szTextureFormula);
				return FALSE;		//'('がない.文法誤り.
			}

			if(lstrlen(parser_val_type[found_index])!=0 && //引数を持つ
				(*c)==')'){
					g_ErrReason = E_LESS_ARGUMENT;
					g_ErrSymbol = found_index;
					//GlobalFree(szFinalTexture);
					//GlobalFree(szTextureFormula);
					return FALSE; //引数がなかったら誤り.
			}

			char* test = c;
			//引数の数が多すぎないかチェック
			for(j=0; ; j++) //lstrlen()だけでいい
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

				if(j>=lstrlen(parser_val_type[found_index])){ //多すぎる
					g_ErrReason = E_OVER_ARGUMENT;
					g_ErrSymbol = found_index;
					//GlobalFree(szFinalTexture);
					//GlobalFree(szTextureFormula);
					return FALSE;
				}
			}

			if(parser_val_type[found_index][0]=='v'){ //可変長の場合
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

				for(j=0; j<cnt*(parser_val_type[found_index][1]-'0'); j++) //引数リストの2番目ずつ読み込む
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
				for(j=0; j<lstrlen(parser_val_type[found_index]); j++) //lstrlen()だけでいい
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
			//データ読み取り準備
			char* c=szTextureFormula;
			c+=lstrlen("texturesize");
			if(*c=='(') c++;		//'('を破棄
			else{
				g_ErrReason = E_NOT_FOUND_OPENED_ARC;
				g_ErrSymbol = found_index;
				return FALSE;		//'('がない.文法誤り.
			}

			if(lstrlen("ii")!=0 && //引数を持つ
				(*c)==')'){
					g_ErrReason = E_LESS_ARGUMENT;
					g_ErrSymbol = found_index;
					return FALSE; //引数がなかったら誤り.
			}

			char* test = c;
			//引数の数が多すぎないかチェック
			for(j=0; ; j++) //lstrlen()だけでいい
			{
				BOOL isOK = GetValueStr(&test, num_str, (DWORD)GlobalSize(num_str));
				if(!isOK){
					break;
				}

				if(j>=lstrlen("ii")){ //多すぎる
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

		//次のループのために,使用した分を消去する
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
	//ダイアログへの表示
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

//関数名 : RemoveCPPCommentFromString()
//機能   : 指定された文字列から, C++スタイルのコメントの行を削除して前方につめていく.
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
	//				//通常の行末処理
	//				if(szSrc[i+diff]=='\r' && szSrc[i+diff+1]=='\n')
	//				{
	//					diff+=2;
	//					break;
	//				}
	//				//テキストが１行しかなかった場合
	//				else if(szSrc[i+diff] == '\0') break;
	//				else diff++;	//ポインタを進める.
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
						//通常の行末処理
						if(szSrc[i+diff]=='\r' && szSrc[i+diff+1]=='\n')
						{
							*ctrf+=2;
							diff+=2;
							break;
						}
						//テキストが１行しかなかった場合
						else if(szSrc[i+diff] == '\0') break;
						else diff++;	//ポインタを進める.
					}
				}else break;
			}else{
				break;//テキスト中の場合はポインタを進める
			}
		}
		////str[i] = szSrc[i+diff];
		szRemoveCommentBuffer[i] = szSrc[i+diff];
	}
	lstrcpy(szSrc, szRemoveCommentBuffer);
	//GlobalFree(szRemoveCommentBuffer);
	return diff;
}

//関数名 : RemoveCCommentFromString()
//機能   : 指定された文字列から, Cスタイルのコメントの範囲を削除して前方につめていく.
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
						//通常の行末処理
						if(szSrc[i+diff]=='*' && szSrc[i+diff+1]=='/')
						{
							*ctrf+=2;
							diff+=2;
							break;
						}
						//テキストが１行しかなかった場合
						else if(szSrc[i+diff] == '\0') break;
						else diff++;	//ポインタを進める.
					}
				}else break;
			}else{
				break;//テキスト中の場合はポインタを進める
			}
		}
		szRemoveCommentBuffer[i] = szSrc[i+diff];
	}
	lstrcpy(szSrc, szRemoveCommentBuffer);
	//GlobalFree(szRemoveCommentBuffer);
	return diff;
}

//関数名 : RemoveCharFromString()
//機能   : 指定された文字列から, 指定された文字を削除して前方につめていく.
//備考   : Exceptionで指定された文字で囲まれた区間においては, Targetは無視される.
//         既知のバグとして,"\"\""が""としてみなされないことがあげられる(KTexture側で対処).
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

//関数名 : RemoveCaridgeReturnFromString()
//機能   : 指定された文字列から, \r\nを削除.
//問題点 : 日本語非対応
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