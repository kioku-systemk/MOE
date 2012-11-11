#include "stdafx.h"
#include "MaterialWindow.h"
#include "FileOperation.h"
#include "modeler window.h"
#include "StartupCode.h"


extern PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD3FARBPROC		glMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD4FARBPROC		glMultiTexCoord4fARB;
extern PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;

CListView vMaterial;
extern KTextureEdit ktex;
extern KModelEdit mdl;
extern CWindowGL win;

int selected_material = -1;
static char szLastTexture[1024*64] = {'\0'};
int nCurrentTextureLevel = 0;

const int sfactornum = 9;
const int dfactornum = 8;
const int factornum = 11;
const char szFactorList [11][64]         = { "GL_ZERO", "GL_ONE", "GL_DST_COLOR", "GL_SRC_COLOR", "GL_ONE_MINUS_DST_COLOR", "GL_ONE_MINUS_SRC_COLOR", "GL_SRC_ALPHA", "GL_ONE_MINUS_SRC_ALPHA", "GL_DST_ALPHA", "GL_ONE_MINUS_DST_ALPHA", "GL_SRC_ALPHA_SATURATE" };
const char szSfactorList[sfactornum][64] = { "GL_ZERO", "GL_ONE", "GL_DST_COLOR", "GL_ONE_MINUS_DST_COLOR", "GL_SRC_ALPHA", "GL_ONE_MINUS_SRC_ALPHA", "GL_DST_ALPHA", "GL_ONE_MINUS_DST_ALPHA", "GL_SRC_ALPHA_SATURATE" };
const char szDfactorList[dfactornum][64] = { "GL_ZERO", "GL_ONE", "GL_SRC_COLOR", "GL_ONE_MINUS_SRC_COLOR", "GL_SRC_ALPHA", "GL_ONE_MINUS_SRC_ALPHA", "GL_DST_ALPHA", "GL_ONE_MINUS_DST_ALPHA" };

const char* szPage[] = {
"1",
"2",
"3",
"4",
"5",
"6",
"7",
"8"
};
const char* szChannel[] = {
"RGB",
"ALPHA"
};
const char* szglMultiTextureOperator[] = {
"REPLACE",
"MODULATE",
"ADD",
"ADD_SIGNED",
"INTERPOLATE",
"SUBTRACT"
};
const char* szglMultiTextureSource[] = {
"TEXTURE",
"CONSTANT",
"PRIMARY_COLOR",
"PREVIOUS"
};
const char* szglMultiTextureOperand[] = {
"SRC_COLOR",
"1-SRC_COLOR",
"SRC_ALPHA",
"1-SRC_ALPHA"
};
const char* szglMultiTextureScale[] = {
"1.0f",
"2.0f",
"4.0f"
};
const char* szglMultiTextureMapping[] = {
"PLANE",
"ENVIROMENT",
"CYLINDER",
"SPHERE"
};

const unsigned int nMultiTextureOperator[] = {
	GL_REPLACE,
	GL_MODULATE,
	GL_ADD,
	GL_ADD_SIGNED_ARB,
	GL_INTERPOLATE_ARB,
	GL_SUBTRACT_ARB
};
const unsigned int nMultiTextureSource[] = {
	GL_TEXTURE,
	GL_CONSTANT_ARB,
	GL_PRIMARY_COLOR_ARB,
	GL_PREVIOUS_ARB
};
const unsigned int nMultiTextureOperand[] = {
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA
};
const float fMultiTextureScale[] = {
1.0f,
2.0f,
4.0f
};
WNDPROC OldComboBoxProc = NULL;

KMaterial g_org;

UCHAR g_Mat[256];

void InitMaterialWindow(){
	//ZeroMemory(&g_mDp, sizeof(g_mDp));
	ZeroMemory(&g_org, sizeof(KMaterial));
	//glEnable(GL_TEXTURE_2D);
	//ktex.GenerateTextureIndirect(&alphatex, "Z,50,50,255,255,255,255,192,192,192,192;");
}

void AddMaterialList(HWND hListView, int nOrder, char* str)
{
	vMaterial.AddItem(str, nOrder);
}

void ClearMaterialList(HWND hListView)
{
	vMaterial.DeleteAllItem();
}

void RefreshMaterialList()
{
	int lastpos = vMaterial.GetSelectedItem();
	ClearMaterialList(vMaterial.GetListWnd());
	long i;
	char ilist[128];
	for(i=0; i<256; i++){
		KMaterial* mat = mdl.GetMaterial(i);
		if(mat==NULL)	sprintf(ilist,"%d: %s ", i, "-");
		else{
			sprintf(ilist,"%d: %s", i, (mat->mat_name[0]!='\0') ? mat->mat_name : "no name");
		}
		AddMaterialList(vMaterial.GetListWnd(), i, ilist);
	}
	if(lastpos==-1) lastpos = 0;
	vMaterial.SelectItem(lastpos);
	vMaterial.EnsureVisible(lastpos);
}

//マテリアルの名称が変更されていない場合に限り有効
int KMaterialToListView(KMaterial* mat)
{
	for(int i=0; i<mdl.GetMaterialNum(); i++)
	{
		if(mdl.GetMaterial(i) == mat)
		{
			return i;
		}
	}
	return -1;
}

KMaterial* ListViewToKMaterial(int nLv)
{
	char szMat[MAX_PATH];
	lstrcpy(szMat, vMaterial.GetItemName(nLv));
	int item = atoi(szMat);
	//win.CMessageBox("%d, %s, %d", nLv, szMat, item);
	return mdl.GetMaterial(item);
}

int MaterialNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit)
{
	int length = lstrlen(szNewName)+1;
	//for(int i=0; i<lstrlen(szNewName)+1; i++)
	//{
	//	if(szNewName[i] == ' ' && szNewName[i-1] == ':')
	//	{
	//		length -= (i+1);
	//		break;
	//	}
	//}
	if(length>8) return FALSE;
	KMaterial* mat = mdl.GetMaterial(selected_material);
	if(mat == NULL) return FALSE;
	if(!isAboutToEdit){
		//SetUndo();
		ZeroMemory(mat->mat_name, sizeof(char)*8);
		lstrcpy(mat->mat_name, szNewName);
		RefreshMaterialList();
	}
	return TRUE;
}

void MaterialPopup(HWND	hWnd, WPARAM wParam, LPARAM	lParam)
{
	LV_HITTESTINFO lvhtst;
	HWND hList = vMaterial.GetListWnd();

	lvhtst.pt.x	= LOWORD(lParam);
	lvhtst.pt.y	= HIWORD(lParam);
	ScreenToClient(hList, &lvhtst.pt);
	int nDest =	ListView_HitTest(hList,	&lvhtst);
	if(nDest>=0){
		HMENU hMenu, hSub;
		vMaterial.SelectItem(nDest);
		hMenu =	LoadMenu(GetModuleHandle(NULL), "MATERIALVIEW");//GetMenu(hWnd);
		hSub = GetSubMenu(hMenu, 0);
		TrackPopupMenu(hSub, TPM_LEFTALIGN, (int)LOWORD(lParam), (int)HIWORD(lParam), 0, hWnd,	NULL);
	}
}

void MaterialCommand(HWND hWnd,	WPARAM wParam, LPARAM lParam)
{
	HWND hList = vMaterial.GetListWnd();

	switch (LOWORD(wParam)){
		case ID_ITEMVIEW_NEW:
		{
			//int item = vMaterial.AddItem(NULL, -1);
			////マテリアル生成
			//mdl.FreeMaterial(item);
			//mdl.CreateMaterial(item);
			//KMaterial* mat = mdl.GetMaterial(item);
			//SetUndo();
			int item = selected_material;
			mdl.FreeMaterial(item);
			mdl.CreateMaterial(item);

			//KMaterial* mat = mdl.GetMaterial(item);
			//mat->color.r = 0.0f;
			//mat->color.g = 0.0f;
			//mat->color.b = 0.0f;
			//mat->color.a = 1.0f;
			//mat->gltexure_num = 0;
			//mat->subdivide = 0;
			//lstrcpy(mat->mat_name, "new mat");

			RefreshMaterialList();
			break;
		}
		case ID_EDIT_COPY:
		{
			int item = selected_material;
			KMaterial* mat = mdl.GetMaterial(item);
			if(!mat) return;	//そもそもコピー元が空

			int i=0;
			for(i=0; i<KMD_MATERIAL_NUM; i++){//空のスロットを探してそこにコピー先を確保する
				if(mdl.GetMaterial(i)==NULL){
					mdl.CreateMaterial(i);
					break;
				}
			}
			KMaterial* tmp = mdl.GetMaterial(i);
			if(!tmp) return;

			for(i=0; i<tmp->number_of_texture; i++){//コピー先に確保される情報を削除する
				GlobalFree(tmp->texture[i]);
			}
			GlobalFree(tmp->texture);
			GlobalFree(tmp->texenv);
			GlobalFree(tmp->texture_id);
			GlobalFree(tmp->multi_texture_env);

			*tmp = *mat;//メモリの操作がいらない変数は一括コピー
			lstrcpy(tmp->mat_name, mat->mat_name);//配列だから=じゃアドレスの代入しかされん
			//CopyMemory(tmp->reserve, mat->reserve, sizeof(mat->reserve)/*KMD_MATERIAL_RESERVE_VER1*/);

			tmp->texture = (char**)GlobalAlloc(GPTR, sizeof(char*) * mat->number_of_texture);
			for(i=0; i<mat->number_of_texture; i++){
				tmp->texture[i] = (char*)GlobalAlloc(GPTR, GlobalSize(mat->texture[i]));
				CopyMemory(tmp->texture[i], mat->texture[i], GlobalSize(mat->texture[i]));
			}

			tmp->texture_id = (unsigned int*)GlobalAlloc(GPTR, GlobalSize(mat->texture_id));
			CopyMemory(tmp->texture_id, mat->texture_id, GlobalSize(mat->texture_id));

			tmp->texenv = (unsigned char*)GlobalAlloc(GPTR, GlobalSize(mat->texenv));
			CopyMemory(tmp->texenv, mat->texenv, GlobalSize(mat->texenv));

			tmp->multi_texture_env = (KMultiTextureEnv*)GlobalAlloc(GPTR, GlobalSize(mat->multi_texture_env));
			CopyMemory(tmp->multi_texture_env, mat->multi_texture_env, GlobalSize(mat->multi_texture_env));

			RefreshMaterialList();
			break;
		}
		case ID_ITEMVIEW_EDITNAME:
			//名称変更コードを書く

			break;
		case ID_ITEMVIEW_DELETE:
		{
			//int item = vMaterial.GetSelectedItem();
			//vMaterial.DeleteItem(item);
			//mdl.FreeMaterial(item);
			//SetUndo();
			int item = selected_material;
			mdl.FreeMaterial(item);
			RefreshMaterialList();
			break;
		}
		case ID_ITEMVIEW_PROPERTY:
		{
			//SetUndo();
			int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_MATERIAL_DIALOG), hMainWnd, (DLGPROC)MaterialDlgProc);
			if(ret>=0){
				int item = selected_material;
				KMaterial* mat = mdl.GetMaterial(item);
				if(mat == NULL) return;

				unsigned int nTexNum = mat->number_of_texture;
				unsigned int i;
				for(i=mat->number_of_texture-1; i>=0; i--){
					if(mat->texture_id[i]==0){
						nTexNum--;
					}else{
						break;
					}
				}
				if(mat->number_of_texture > nTexNum){//８枚から減っている分だけ変更する
					if(nTexNum==0) nTexNum=1;
					//mat->texture = (char**)GlobalReAlloc(mat->texture, sizeof(char*) * nTexNum, GMEM_ZEROINIT);
					KMaterial* tmp = new KMaterial();

					tmp->texture = (char**)GlobalAlloc(GPTR, sizeof(char*) * nTexNum);
					//CopyMemory(tmp->texture, mat->texture, mat->number_of_texture * sizeof(char*));
					for(i=0; i<nTexNum; i++){
						if(mat->texture==NULL){
							MessageBox(NULL, "mat->texture==NULL!", 0, MB_OK);
							mat->texture = (char**)GlobalAlloc(GPTR, 1 * sizeof(char*));
							if(mat->texture[i]==NULL){
								MessageBox(NULL, "mat->texture[i]==NULL!", 0, MB_OK);
								mat->texture[i] = (char*)GlobalAlloc(GPTR, 1);
								//SaveModel(mdl);
							}
						}
						DWORD dwSize = GlobalSize(mat->texture[i]);
						tmp->texture[i] = (char*)GlobalAlloc(GPTR, dwSize);
						//CopyMemory(tmp->texture[i], mat->texture[i], dwSize);
						lstrcpy(tmp->texture[i], mat->texture[i]);
					}

					for(i=0; i<mat->number_of_texture; i++){
						GlobalFree(mat->texture[i]);
						mat->texture[i] = NULL;
					}
					GlobalFree(mat->texture);
					mat->texture = NULL;

					mat->texture = tmp->texture;//すり替え
					for(i=0; i<nTexNum; i++){
						mat->texture[i] = tmp->texture[i];
						//tmp->texture[i] = NULL;
					}
					//tmp->texture = NULL;

					//mat->texture_id = (unsigned int*)GlobalReAlloc(mat->texture_id, sizeof(unsigned int) * nTexNum, GMEM_ZEROINIT);
					tmp->texture_id = (unsigned int*)GlobalAlloc(GPTR, sizeof(unsigned int) * nTexNum);
					for(i=0; i<nTexNum; i++){
						tmp->texture_id[i] = mat->texture_id[i];
					}
					//CopyMemory(tmp->texture_id, mat->texture_id, GlobalSize(tmp->texture_id));
					GlobalFree(mat->texture_id);
					mat->texture_id = NULL;
					mat->texture_id = tmp->texture_id;
					//tmp->texture_id = NULL;

					//mat->texenv = (unsigned char*)GlobalReAlloc(mat->texenv, sizeof(unsigned char) * nTexNum, GMEM_ZEROINIT);
					tmp->texenv = (unsigned char*)GlobalAlloc(GPTR, sizeof(unsigned char) * nTexNum);
					for(i=0; i<nTexNum; i++){
						tmp->texenv[i] = mat->texenv[i];
					}
					//CopyMemory(tmp->texenv, mat->texenv, GlobalSize(tmp->texenv));
					GlobalFree(mat->texenv);
					mat->texenv = NULL;
					mat->texenv = tmp->texenv;
					//tmp->texenv = NULL;

					//mat->multi_texture_env = (KMultiTextureEnv*)GlobalReAlloc(mat->multi_texture_env, sizeof(KMultiTextureEnv) * nTexNum, GMEM_ZEROINIT);
					tmp->multi_texture_env = (KMultiTextureEnv*)GlobalAlloc(GPTR, sizeof(KMultiTextureEnv) * nTexNum);
					for(i=0; i<nTexNum; i++){
						tmp->multi_texture_env[i] = mat->multi_texture_env[i];
					}
					//CopyMemory(tmp->multi_texture_env, mat->multi_texture_env, GlobalSize(tmp->multi_texture_env));
					GlobalFree(mat->multi_texture_env);
					mat->multi_texture_env = NULL;
					mat->multi_texture_env = tmp->multi_texture_env;
					//tmp->multi_texture_env = NULL;

					delete tmp;
					mat->number_of_texture = nTexNum;
				}
			}else{
				int item = selected_material;
				KMaterial* mat = mdl.GetMaterial(item);
				if(mat == NULL) return;
				
				//キャンセルされた場合は変更前のコードに戻す
				unsigned int i;
				if(mat->texture != NULL){
					for(i=0; i<mat->number_of_texture; i++){
						GlobalFree(mat->texture[i]);
						mat->texture[i] = NULL;
					}
					GlobalFree(mat->texture);
					mat->texture = NULL;
				}
				if(g_org.texture!=NULL){
					mat->texture = (char**)GlobalAlloc(GPTR, sizeof(char*) * g_org.number_of_texture);
					for(i=0; i<g_org.number_of_texture; i++){
						mat->texture[i] = (char*)GlobalAlloc(GPTR, /*sizeof(char) * (lstrlen(g_org.texture[i])+1)*/GlobalSize(g_org.texture[i]));
						//CopyMemory(mat->texture[i], g_org.texture[i], GlobalSize(g_org.texture[i]));
						lstrcpy(mat->texture[i], g_org.texture[i]);
					}
				}

				if(mat->multi_texture_env!=NULL){
					GlobalFree(mat->multi_texture_env);
					mat->multi_texture_env = NULL;
				}
				if(g_org.multi_texture_env!=NULL){
					mat->multi_texture_env = (KMultiTextureEnv*)GlobalAlloc(GPTR, GlobalSize(g_org.multi_texture_env));
					CopyMemory(mat->multi_texture_env, g_org.multi_texture_env, GlobalSize(g_org.multi_texture_env));
				}

				if(mat->texture_id!=NULL){
					GlobalFree(mat->texture_id);
					mat->texture_id = NULL;
				}
				if(g_org.texture_id!=NULL){
					mat->texture_id = (unsigned int*)GlobalAlloc(GPTR, GlobalSize(g_org.texture_id));
					CopyMemory(mat->texture_id, g_org.texture_id, GlobalSize(g_org.texture_id));
				}

				if(mat->texenv!=NULL){
					GlobalFree(mat->texenv);
					mat->texenv = NULL;
				}
				if(g_org.texenv!=NULL){
					mat->texenv = (unsigned char*)GlobalAlloc(GPTR, GlobalSize(g_org.texenv));
					CopyMemory(mat->texenv, g_org.texenv, GlobalSize(g_org.texenv));
				}

				mat->number_of_texture = g_org.number_of_texture;
				mat->blendf = g_org.blendf;
				mat->color = g_org.color;
				CopyMemory(mat->mat_name, g_org.mat_name, sizeof(char) * 8);
				//lstrcpy(mat->mat_name , g_org.mat_name);
				mat->shade = g_org.shade;
				mat->subdivide = g_org.subdivide;
				mat->uv_rot = g_org.uv_rot;
				mat->uv_scale = g_org.uv_scale;
				mat->uv_trans = g_org.uv_trans;
				//if(lstrcmp(g_org.texture, mat->texture)!=0)
				//	MaterialCreateTexture(item, mat->texture); //変更前のテクスチャを生成

				//if(mat->texture!=NULL && ){
				//	GlobalFree(mat->texture);
				//mat->texture = (char*)GlobalAlloc(GPTR, (lstrlen(g_mDp.szTexture)+1)*sizeof(char));
				//	lstrcpy(mat->texture, g_mDp.szTexture);
				//	if(mat->texture[0]=='\0') MaterialCreateTexture(item, mat->texture);
				//}
			}
			RefreshMaterialList();
			break;
		}
	}
	InvalidateRect(hMainWnd, NULL, FALSE);
}

int MaterialKey(UCHAR key, bool isDown)
{
/*
	switch(key)
	{
		case VK_F2:
			ListView_EditLabel(m_hListViewWnd, GetSelectedItem());
			break;
	}
*/
	return FALSE;
}

HWND g_hMatDlg;

static int val = 0;
void OnMouseMove(int nCtrl, bool& isDragging)
{
	static int nOldCtrl = 0;
	if(isDragging){
		POINT xy;
		GetCursorPos(&xy);
		ScreenToClient(GetDlgItem(g_hMatDlg, nCtrl), &xy);
		static POINT last = {xy.x, xy.y};
		POINT now = {xy.x, xy.y};

		if(nOldCtrl!=nCtrl){
			last = now;
			nOldCtrl = nCtrl;
		}
		if(last.x!=now.x){
			val+=now.x - last.x;
			last = now;
		}
		if(val<0) val = 0;
		else if(val>255) val = 255;
		SetDlgItemInt(g_hMatDlg, nCtrl, val, FALSE);

		KMaterial* mat = mdl.GetMaterial(selected_material);
		if(mat){
			switch(nCtrl)
			{
			case IDC_MATERIAL_COLOR_R:
				mat->color.r = val/255.0f;
			break;
			case IDC_MATERIAL_COLOR_G:
				mat->color.g = val/255.0f;
			break;
			case IDC_MATERIAL_COLOR_B:
				mat->color.b = val/255.0f;
			break;
			case IDC_MATERIAL_COLOR_A:
				mat->color.a = val/255.0f;
			break;
			case IDC_MATERIAL_SUBDIVIDE:
				mat->subdivide = val;
			break;
			}
			SendMessage(GetDlgItem(g_hMatDlg, IDC_MATERIAL_PREVIEW), WM_PAINT, 0, 0);
		}
	}
}

void OnLButton(int nCtrl, bool isDown, bool& isDragging, bool& isFocused)
{
	if(isDown){
		if(!isFocused)
		{
			POINT pt;
			GetCursorPos(&pt);
			//ClientToScreen(GetDlgItem(g_hMatDlg, nCtrl), &pt);
			if(WindowFromPoint(pt)==GetDlgItem(g_hMatDlg, nCtrl))
			{
				SetFocus(GetDlgItem(g_hMatDlg, nCtrl));
				isFocused = true;
				isDragging = true;
				char szBuf[64];
				GetDlgItemText(g_hMatDlg, nCtrl, szBuf, 64);
				val = StrToInt(szBuf);

				SetCapture(GetDlgItem(g_hMatDlg, nCtrl));
			}else{
				isFocused = false;
			}
		}
	}else{
		isFocused = false;
		isDragging = false;
		ReleaseCapture();
		val = 0;	
	}
}

void OnWheel(int nCtrl, bool isDown, bool& isDragging, bool& isFocused){
	POINT pt;
	::GetCursorPos(&pt);
	if(WindowFromPoint(pt)==GetDlgItem(g_hMatDlg, nCtrl))
	{
		isFocused = true;
		char szBuf[64];
		GetDlgItemText(g_hMatDlg, nCtrl, szBuf, 64);
		val = StrToInt(szBuf);
	}else{
		isFocused = false;
	}
	
	if(isFocused)
	{
		if(isDown)
			val++;
		else
			val--;
		if(val<=0) val = 0;
		else if(val>=255) val = 255;
		SetDlgItemInt(g_hMatDlg, nCtrl, val, FALSE);
		KMaterial* mat = mdl.GetMaterial(selected_material);
		if(mat){
			switch(nCtrl)
			{
			case IDC_MATERIAL_COLOR_R:
				mat->color.r = val/255.0f;
			break;
			case IDC_MATERIAL_COLOR_G:
				mat->color.g = val/255.0f;
			break;
			case IDC_MATERIAL_COLOR_B:
				mat->color.b = val/255.0f;
			break;
			case IDC_MATERIAL_COLOR_A:
				mat->color.a = val/255.0f;
			break;
			case IDC_MATERIAL_SUBDIVIDE:
				mat->subdivide = val;
			break;
			}
			SendMessage(GetDlgItem(g_hMatDlg, IDC_MATERIAL_PREVIEW), WM_PAINT, 0, 0);
		}
	}
}

HWND hMaterialEditControl[5];
int GetMaterialEditControlId(HWND hWnd){
	const int aMaterialEditControlId[5] = {IDC_MATERIAL_COLOR_R, IDC_MATERIAL_COLOR_G, IDC_MATERIAL_COLOR_B, IDC_MATERIAL_COLOR_A, IDC_MATERIAL_SUBDIVIDE};
	for(int i=0; i<sizeof(hMaterialEditControl)/sizeof(HWND); i++){
		if(hWnd == hMaterialEditControl[i]){
			return aMaterialEditControlId[i];
		}
	}
	return 0;
}

WNDPROC OldEditProc;
LRESULT CALLBACK EditControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)//support mouse
{
	static bool isFocused = false;
	static bool isDragging = false;

	int nMaterialEditControlId = GetMaterialEditControlId(hWnd);
	switch(msg)
	{
		case WM_MOUSEWHEEL:
		{
			OnWheel(nMaterialEditControlId, ((int)wParam>0), isDragging, isFocused);
		}
		case WM_MOUSEMOVE:
		{
			OnMouseMove(nMaterialEditControlId, isDragging);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			OnLButton(nMaterialEditControlId, TRUE, isDragging, isFocused);
			break;
		}
		case WM_LBUTTONUP:
		{
			OnLButton(nMaterialEditControlId, FALSE, isDragging, isFocused);
			break;
		}
	}
	return CallWindowProc(OldEditProc, hWnd, msg, wParam, lParam);
}

int MaterialCreateTexture(int item, const char* szTexture)
{
	KMaterial* mat = mdl.GetMaterial(item);//ListViewToKMaterial(item);
	if(mat == NULL) return FALSE;

	if(szTexture==NULL || szTexture[0]=='\0') return FALSE;

	if(mat->texture[nCurrentTextureLevel]!=NULL){
		GlobalFree(mat->texture[nCurrentTextureLevel]);
		mat->texture[nCurrentTextureLevel] = NULL;
	}
	mat->texture[nCurrentTextureLevel] = (char*)GlobalAlloc(GPTR, sizeof(char) * (lstrlen(szTexture)+1));
	lstrcpy(mat->texture[nCurrentTextureLevel], szTexture);
	ktex.GenerateTextureIndirect(&mat->texture_id[nCurrentTextureLevel], mat->texture[nCurrentTextureLevel]);

	SendMessage(GetDlgItem(g_hMatDlg, IDC_MATERIAL_PREVIEW), WM_PAINT, 0, 0);
	return TRUE;
}

WNDPROC OldPreview;
LRESULT CALLBACK PreviewProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HDC hDC;
	static HGLRC hRC;
	static int oldPF;
	static PIXELFORMATDESCRIPTOR pfd=
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,	//Z-Buffer
		0,  //Stencil Buffer
		0,
		PFD_MAIN_PLANE,
		0, //Reserved
		0, 0, 0	
	};
	static unsigned int alphatex = 0;

	switch(msg)
	{
		case WM_CREATE:
		{
			hDC = GetDC(hWnd);
			SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
			hRC = win.GetRC();
			
			//hRC = wglCreateContext(hDC);
			//wglCopyContext(win.GetRC(), hRC, GL_ALL_ATTRIB_BITS); //SiSなどのオンボードがGL_ALL_ATTRIB_BITSをサポートしていない
			//wglShareLists	(win.GetRC(), hRC);
			//wglMakeCurrent(hDC, hRC);

			//GetClientRect(hWnd, &pvrect); //PreviewWindowの大きさを取得しておく
			//wglMakeCurrent(hDC, hRC);
			ktex.GenerateTextureIndirect(&alphatex, "Z,50,50,255,255,255,255,192,192,192,192;");
			break;
		}
		case WM_PAINT:
		{
			//wglMakeCurrent(hDC, hRC);
			if(hDC != wglGetCurrentDC()) //もしメインウィンドウにフォーカスが移っていたら
			{
				RECT pvrect;
				GetClientRect(hWnd, &pvrect);
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glViewport(0, 0, pvrect.right, pvrect.bottom);
				//glViewport(0, 0, pvrect.right - pvrect.left, pvrect.bottom - pvrect.top)
				wglMakeCurrent(hDC, hRC);
			}
			//glMatrixMode(GL_PROJECTION);
			//glLoadIdentity();
			//glViewport(0, 0, pvrect.right, pvrect.bottom);

			KMaterial* mat = mdl.GetMaterial(selected_material);
			if(!mat){
				glEnable(GL_LIGHTING);
				glEnable(GL_LIGHT0);		
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_BLEND);
				float mcolor[] = {0.5f,0.5f,0.5f,0.5f};
				glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,mcolor);					
				return 0;
			}
			unsigned int i;
			glPushMatrix();
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				//gluLookAt(0, 0, 50, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
				glClearColor(255, 255, 255, 255);
				glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER,0.01f);
				glEnable(GL_COLOR_MATERIAL);
				glDisable(GL_LIGHTING);
				glDisable(GL_LIGHT0);

				glDisable(GL_BLEND);

				glDepthMask(GL_FALSE);
				
				glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
					glDisable(GL_CULL_FACE);
					glDisable(GL_TEXTURE_GEN_S);
					glDisable(GL_TEXTURE_GEN_T);

					glMatrixMode(GL_TEXTURE);
						glLoadIdentity();
					glMatrixMode(GL_MODELVIEW);
						glLoadIdentity();

					//αテクスチャを描画する
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, alphatex);
					glColor4f(1.0f,1.0f,1.0f,1.0f);
					glBegin(GL_POLYGON);
							glTexCoord2f(0.0f, 1.0f);
							glVertex2d(-1.0, -1.0);
							glTexCoord2f(1.0f, 1.0f);
							glVertex2d(1.0, -1.0);
							glTexCoord2f(1.0f, 0.0f);
							glVertex2d(1.0, 1.0);
							glTexCoord2f(0.0f, 0.0f);
							glVertex2d(-1.0, 1.0);
					glEnd();
					glBindTexture(GL_TEXTURE_2D, 0);
					glDisable(GL_TEXTURE_2D);
				glPopAttrib();
				glDepthMask(GL_TRUE);

				//Draw a textured preview polygon 
				unsigned int nTexNum = mat->number_of_texture;
				if(mat->texture_id){
					//if(nTexNum>1){
						glDisable(GL_TEXTURE_GEN_S);
						glDisable(GL_TEXTURE_GEN_T);

						glMatrixMode(GL_TEXTURE);
							glLoadIdentity();
						glMatrixMode(GL_MODELVIEW);
							glLoadIdentity();
						for(i=0; i<nTexNum; i++){
							if(mat->texture_id[i]!=0){
								int k;
								glActiveTextureARB(GL_TEXTURE0_ARB+i);
								glEnable(GL_TEXTURE_2D);
								glBindTexture(GL_TEXTURE_2D, mat->texture_id[i]);
								glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);

								//setup multi-texture on rgb channel.
								glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, nMultiTextureOperator[mat->multi_texture_env[i].op&0x0F]);
								for(k=0; k<3; k++){
									glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB+k, nMultiTextureSource[mat->multi_texture_env[i].source_param[k]&0x0F]);
								}
								for(k=0; k<3; k++){
									glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB+k, nMultiTextureOperand[mat->multi_texture_env[i].operand_param[k]&0x0F]);
								}
								glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, fMultiTextureScale[mat->multi_texture_env[i].fscale&0x0F]);

								//setup multi-texture on alpha channel.
								glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, nMultiTextureOperator[mat->multi_texture_env[i].op>>4]);
								for(k=0; k<3; k++){
									glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB+k, nMultiTextureSource[mat->multi_texture_env[i].source_param[k]>>4]);
								}
								for(k=0; k<3; k++){
									glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB+k, nMultiTextureOperand[mat->multi_texture_env[i].operand_param[k]>>4]);
								}
								glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, fMultiTextureScale[mat->multi_texture_env[i].fscale>>4]);
							}
						}
						glDisable(GL_LIGHTING);
						glDisable(GL_LIGHT0);
						glDisable(GL_TEXTURE_GEN_S);
						glDisable(GL_TEXTURE_GEN_T);
			
						KRGBA rgba = mat->color;
						glColor4f(rgba.r,rgba.g,rgba.b,rgba.a);
						float mcolor[] = {rgba.r,rgba.g,rgba.b,rgba.a};
						//glEnable(GL_BLEND);
						//glBlendFunc(nBlendFactorList[mat->shade&0x0F], nBlendFactorList[mat->shade>>4]); 
						//glBlendFunc(GL_ONE,GL_ZERO);
						glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,mcolor);
						//glTranslatef(0.0f,0.0f,-5.0f);

						glDepthMask(GL_FALSE);
						glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
							glDisable(GL_CULL_FACE);
							glBegin(GL_POLYGON);
								for(i=0; i<nTexNum; i++){
									if(mat->texture_id[i]!=0)
										glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, 0.0f, 1.0f);
								}
								glVertex2d(-1.0, -1.0);

								for(i=0; i<nTexNum; i++){
									if(mat->texture_id[i]!=0)
										glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, 1.0f, 1.0f);
								}
								glVertex2d(1.0, -1.0);

								for(i=0; i<nTexNum; i++){
									if(mat->texture_id[i]!=0)
										glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, 1.0f, 0.0f);
								}
								glVertex2d(1.0, 1.0);

								for(i=0; i<nTexNum; i++){
									if(mat->texture_id[i]!=0)
										glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, 0.0f, 0.0f);
								}
								glVertex2d(-1.0, 1.0);
							glEnd();
							for(i=7; (signed)i>=0; i--){//UNBIND TEXTUREs
								//if(mat->texture_id[i]!=0){
									glActiveTextureARB(GL_TEXTURE0_ARB + i);
									glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
									glBindTexture(GL_TEXTURE_2D, 0);
									glDisable(GL_TEXTURE_2D);
								//}
							}
							glEnable(GL_CULL_FACE);
						glPopAttrib();
						glDepthMask(GL_TRUE);
				}else{
					//for(i=7; (signed)i>=0; i--){//UNBIND TEXTUREs
					//	glActiveTextureARB(GL_TEXTURE0_ARB + i);
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
					glBindTexture(GL_TEXTURE_2D, 0);
					glDisable(GL_TEXTURE_2D);
					//}
				}
			glPopMatrix();
			
			SwapBuffers(hDC);
			break;
		}
		case WM_DESTROY:
		{
//			wglShareLists(hRC, win.GetRC());
			//SetPixelFormat(win.GetDC(), oldPF, &pfd);
			wglMakeCurrent(win.GetDC(), win.GetRC()); //メインウィンドウをアクティブにする
			
			//--------メインウィンドウにWM_SIZEを送って、glViewport()を実行させる
			RECT rect;
			GetClientRect(hMainWnd, &rect);
			LPARAM send = rect.right&0x0000FFFF;
			send |= rect.bottom<<16;
			SendMessage(hMainWnd, WM_SIZE, 0, send);
			
			//--------PreviewWindowの破棄処理
			//if(hRC!=NULL)   wglDeleteContext(hRC); //削除するな.
			if(hDC!=NULL) ReleaseDC(hWnd, hDC);
			break;
		}
	}
	return CallWindowProc(OldPreview, hWnd, msg, wParam, lParam);
}

BOOL isChannelAlpha(){
	return (ComboBox_GetCurSel(GetDlgItem(g_hMatDlg, IDC_MATERIAL_CHANNEL)) == 1);//0==RGB
}

const int nTexenvToMapping[] = {
0,//0x0
0,
0,
0,
1,//0x4
0,
0,
0,
2,//0x8
0,
0,//0xA
0,
0,
0,
0,
0,
3//0x10
};
void UpdateTextureControl(){
	KMaterial* mat = mdl.GetMaterial(selected_material);
	if(!mat) return;

	SetDlgItemText(g_hMatDlg, IDC_MATERIAL_TEXTURE_FORMULA, mat->texture[nCurrentTextureLevel]);

	BOOL isAlpha = isChannelAlpha();
	HWND hOperator = GetDlgItem(g_hMatDlg, IDC_MATERIAL_OPERATOR);
	if(isAlpha)		ComboBox_SetCurSel(hOperator, mat->multi_texture_env[nCurrentTextureLevel].op>>4);
	else			ComboBox_SetCurSel(hOperator, mat->multi_texture_env[nCurrentTextureLevel].op&0x0F);

	HWND hScale = GetDlgItem(g_hMatDlg, IDC_MATERIAL_SCALE);
	if(isAlpha)		ComboBox_SetCurSel(hScale, mat->multi_texture_env[nCurrentTextureLevel].fscale>>4);
	else			ComboBox_SetCurSel(hScale, mat->multi_texture_env[nCurrentTextureLevel].fscale&0x0F);

	int i;
	for(i=0; i<3; i++){
		HWND hSource = GetDlgItem(g_hMatDlg, IDC_MATERIAL_SOURCE0+i);
		if(isAlpha)		ComboBox_SetCurSel(hSource, mat->multi_texture_env[nCurrentTextureLevel].source_param[i]>>4);
		else			ComboBox_SetCurSel(hSource, mat->multi_texture_env[nCurrentTextureLevel].source_param[i]&0x0F);
	}
	for(i=0; i<3; i++){
		HWND hOperand = GetDlgItem(g_hMatDlg, IDC_MATERIAL_OPERAND0+i);
		if(isAlpha)		ComboBox_SetCurSel(hOperand, mat->multi_texture_env[nCurrentTextureLevel].operand_param[i]>>4);
		else			ComboBox_SetCurSel(hOperand, mat->multi_texture_env[nCurrentTextureLevel].operand_param[i]&0x0F);
	}

	HWND hMapping = GetDlgItem(g_hMatDlg, IDC_MATERIAL_MAPPING);
	ComboBox_SetCurSel(hMapping, nTexenvToMapping[mat->texenv[nCurrentTextureLevel]]);

	SetDlgItemText(g_hMatDlg, IDC_MATERIAL_TEXTURE_FORMULA, mat->texture[nCurrentTextureLevel]);
}

void RenderTexture(){
	SendMessage(GetDlgItem(g_hMatDlg, IDC_MATERIAL_PREVIEW), WM_PAINT, 0, 0);
}

LRESULT CALLBACK PageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg){
	case WM_COMMAND:
	{
		nCurrentTextureLevel = ComboBox_GetCurSel(hWnd);
		UpdateTextureControl();
	}break;
	}
	return CallWindowProc(OldComboBoxProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK ChannelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int oldselection;
	switch(msg){
	case WM_COMMAND:
	{
		int newselection = ComboBox_GetCurSel(hWnd);
		if(oldselection!=newselection){
			UpdateTextureControl();
			oldselection = newselection;
		}
	}break;
	}
	return CallWindowProc(OldComboBoxProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK OperatorProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
	{
		KMaterial* mat = mdl.GetMaterial(selected_material);
		if(isChannelAlpha()){
			UCHAR back = mat->multi_texture_env[nCurrentTextureLevel].op&0x0F;
			mat->multi_texture_env[nCurrentTextureLevel].op = 0;
			mat->multi_texture_env[nCurrentTextureLevel].op |= back;
			mat->multi_texture_env[nCurrentTextureLevel].op |= ComboBox_GetCurSel(hWnd)<<4;
		}else{
			UCHAR back = mat->multi_texture_env[nCurrentTextureLevel].op>>4;
			mat->multi_texture_env[nCurrentTextureLevel].op = 0;
			mat->multi_texture_env[nCurrentTextureLevel].op |= ComboBox_GetCurSel(hWnd);
			mat->multi_texture_env[nCurrentTextureLevel].op |= back<<4;
		}
		RenderTexture();
	}break;
	}
	return CallWindowProc(OldComboBoxProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK SourceProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
	{
		int i;
		KMaterial* mat = mdl.GetMaterial(selected_material);
		if(isChannelAlpha()){
			for(i=0; i<3; i++){
				UCHAR back = mat->multi_texture_env[nCurrentTextureLevel].source_param[i]&0x0F;
				mat->multi_texture_env[nCurrentTextureLevel].source_param[i] = 0;
				mat->multi_texture_env[nCurrentTextureLevel].source_param[i] |= back;
				mat->multi_texture_env[nCurrentTextureLevel].source_param[i] |= ComboBox_GetCurSel(GetDlgItem(g_hMatDlg, IDC_MATERIAL_SOURCE0+i))<<4;
			}
		}else{
			for(i=0; i<3; i++){
				UCHAR back = mat->multi_texture_env[nCurrentTextureLevel].source_param[i]>>4;
				mat->multi_texture_env[nCurrentTextureLevel].source_param[i] = 0;
				mat->multi_texture_env[nCurrentTextureLevel].source_param[i] |= ComboBox_GetCurSel(GetDlgItem(g_hMatDlg, IDC_MATERIAL_SOURCE0+i));
				mat->multi_texture_env[nCurrentTextureLevel].source_param[i] |= back<<4;
			}
		}
		RenderTexture();
	}break;
	}
	return CallWindowProc(OldComboBoxProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK OperandProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
	{
		int i;
		KMaterial* mat = mdl.GetMaterial(selected_material);
		if(isChannelAlpha()){
			for(i=0; i<3; i++){
				UCHAR back = mat->multi_texture_env[nCurrentTextureLevel].operand_param[i]&0x0F;
				mat->multi_texture_env[nCurrentTextureLevel].operand_param[i] = 0;
				mat->multi_texture_env[nCurrentTextureLevel].operand_param[i] |= back;
				mat->multi_texture_env[nCurrentTextureLevel].operand_param[i] |= ComboBox_GetCurSel(GetDlgItem(g_hMatDlg, IDC_MATERIAL_OPERAND0+i))<<4;
			}
		}else{
			for(i=0; i<3; i++){
				UCHAR back = mat->multi_texture_env[nCurrentTextureLevel].operand_param[i]>>4;
				mat->multi_texture_env[nCurrentTextureLevel].operand_param[i] = 0;
				mat->multi_texture_env[nCurrentTextureLevel].operand_param[i] |= ComboBox_GetCurSel(GetDlgItem(g_hMatDlg, IDC_MATERIAL_OPERAND0+i));
				mat->multi_texture_env[nCurrentTextureLevel].operand_param[i] |= back<<4;
			}
		}
		RenderTexture();
	}break;
	}
	return CallWindowProc(OldComboBoxProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK ScaleProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
	{
		KMaterial* mat = mdl.GetMaterial(selected_material);
		if(isChannelAlpha()){
			UCHAR back = mat->multi_texture_env[nCurrentTextureLevel].fscale&0x0F;
			mat->multi_texture_env[nCurrentTextureLevel].fscale = 0;
			mat->multi_texture_env[nCurrentTextureLevel].fscale |= back;
			mat->multi_texture_env[nCurrentTextureLevel].fscale |= ComboBox_GetCurSel(hWnd)<<4;
		}else{
			UCHAR back = mat->multi_texture_env[nCurrentTextureLevel].fscale>>4;
			mat->multi_texture_env[nCurrentTextureLevel].fscale = 0;
			mat->multi_texture_env[nCurrentTextureLevel].fscale |= ComboBox_GetCurSel(hWnd);
			mat->multi_texture_env[nCurrentTextureLevel].fscale |= back<<4;
		}
		RenderTexture();
	}break;
	}
	return CallWindowProc(OldComboBoxProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK MappingProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
	{
		KMaterial* mat = mdl.GetMaterial(selected_material);
		mat->texenv[nCurrentTextureLevel] = 0;
		if(ComboBox_GetCurSel(hWnd)==0){//plane
			mat->texenv[nCurrentTextureLevel] = 0x01;
		}else if(ComboBox_GetCurSel(hWnd)==1){//env
			mat->texenv[nCurrentTextureLevel] = 0x04;
		}else if(ComboBox_GetCurSel(hWnd)==2){//cylinder
			mat->texenv[nCurrentTextureLevel] = 0x08;
		}else if(ComboBox_GetCurSel(hWnd)==3){//sphere
			mat->texenv[nCurrentTextureLevel] = 0x10;
		}
		//RenderTexture();
	}break;
	}
	return CallWindowProc(OldComboBoxProc, hWnd, msg, wParam, lParam);
}

void SetDefaultMultiTextureParams(KMaterial* mat, int nTargetElement){
	int i = nTargetElement;

	//ZeroMemory(&mat->multi_texture_env[i], sizeof(KMultiTextureEnv));
	
	mat->multi_texture_env[i].op = 0;
	mat->multi_texture_env[i].op |= 0x01;//GL_MODULATE
	mat->multi_texture_env[i].op |= 0x01<<4;//GL_MODULATE

	mat->multi_texture_env[i].source_param[0] = 0;
	mat->multi_texture_env[i].source_param[0] |= 0x00;//GL_TEXTURE
	mat->multi_texture_env[i].source_param[0] |= 0x00<<4;//GL_TEXTURE

	mat->multi_texture_env[i].source_param[1] = 0;
	mat->multi_texture_env[i].source_param[1] |= 0x03;//GL_PREVIOUS
	mat->multi_texture_env[i].source_param[1] |= 0x03<<4;//GL_PREVIOUS

	mat->multi_texture_env[i].source_param[2] = 0;
	mat->multi_texture_env[i].source_param[2] |= 0x01;//GL_CONSTANT
	mat->multi_texture_env[i].source_param[2] |= 0x01<<4;//GL_CONSTANT


	mat->multi_texture_env[i].operand_param[0] = 0;
	mat->multi_texture_env[i].operand_param[0] |= 0x00;//GL_COLOR
	mat->multi_texture_env[i].operand_param[0] |= 0x02<<4;//GL_ALPHA

	mat->multi_texture_env[i].operand_param[1] = 0;
	mat->multi_texture_env[i].operand_param[1] |= 0x00;//GL_COLOR
	mat->multi_texture_env[i].operand_param[1] |= 0x02<<4;//GL_ALPHA

	mat->multi_texture_env[i].operand_param[2] = 0;
	mat->multi_texture_env[i].operand_param[2] |= 0x02;//GL_ALPHA
	mat->multi_texture_env[i].operand_param[2] |= 0x02<<4;//GL_ALPHA

	mat->multi_texture_env[i].fscale = 0;
	mat->multi_texture_env[i].fscale |= 0x00;//1.0f
	mat->multi_texture_env[i].fscale |= 0x00<<4;//1.0f
}

LRESULT CALLBACK MaterialDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	g_hMatDlg = hWnd;
	static int item;
	switch(msg)
	{
	case WM_INITDIALOG:
	{
		item = selected_material;
		KMaterial* mat = mdl.GetMaterial(item);
		if(mat == NULL) return FALSE;
		nCurrentTextureLevel = 0;
		ZeroMemory(&g_org, sizeof(KMaterial));

		//
		//サブクラス化
		//
		unsigned int i;
		//オリジナルのプロシージャを保存する
		//OldCheckBoxProc = (WNDPROC)GetWindowLong(GetDlgItem(hWnd, IDC_CHECK1), GWL_WNDPROC);
		OldComboBoxProc = (WNDPROC)GetWindowLong(GetDlgItem(hWnd, IDC_MATERIAL_CHANNEL), GWL_WNDPROC);
		OldEditProc = (WNDPROC)GetWindowLong(GetDlgItem(hWnd, IDC_MATERIAL_COLOR_R), GWL_WNDPROC);

		for(i=0; i<=IDC_MATERIAL_SUBDIVIDE-IDC_MATERIAL_COLOR_R; i++){
			hMaterialEditControl[i] = GetDlgItem(hWnd, IDC_MATERIAL_COLOR_R + i);
		}
		for(i=0; i<sizeof(hMaterialEditControl)/sizeof(HWND); i++){
			SetWindowLong(hMaterialEditControl[i], GWL_WNDPROC, (LONG)EditControlProc);
		}

		SetWindowLong(GetDlgItem(hWnd, IDC_MATERIAL_MAPPING), GWL_WNDPROC, (LONG)MappingProc);
		SetWindowLong(GetDlgItem(hWnd, IDC_MATERIAL_PAGE), GWL_WNDPROC, (LONG)PageProc);
		SetWindowLong(GetDlgItem(hWnd, IDC_MATERIAL_SCALE), GWL_WNDPROC, (LONG)ScaleProc);
		SetWindowLong(GetDlgItem(hWnd, IDC_MATERIAL_OPERATOR), GWL_WNDPROC, (LONG)OperatorProc);
		SetWindowLong(GetDlgItem(hWnd, IDC_MATERIAL_CHANNEL), GWL_WNDPROC, (LONG)ChannelProc);
		for(i=0; i<=IDC_MATERIAL_SOURCE2 - IDC_MATERIAL_SOURCE0; i++){
			SetWindowLong(GetDlgItem(hWnd, IDC_MATERIAL_SOURCE0+i), GWL_WNDPROC, (LONG)SourceProc);
		}
		for(i=0; i<=IDC_MATERIAL_OPERAND2 - IDC_MATERIAL_OPERAND0; i++){
			SetWindowLong(GetDlgItem(hWnd, IDC_MATERIAL_OPERAND0+i), GWL_WNDPROC, (LONG)OperandProc);
		}


		SetWindowPos(GetDlgItem(g_hMatDlg, IDC_MATERIAL_PREVIEW), 0, 0, 0, 256, 256, SWP_NOMOVE);
		OldPreview = (WNDPROC)GetWindowLong(GetDlgItem(hWnd, IDC_MATERIAL_PREVIEW), GWL_WNDPROC);
		SetWindowLong(GetDlgItem(hWnd, IDC_MATERIAL_PREVIEW), GWL_WNDPROC, (LONG)PreviewProc);
		SendMessage(GetDlgItem(hWnd, IDC_MATERIAL_PREVIEW), WM_CREATE, 0, 0);
		SendMessage(GetDlgItem(g_hMatDlg, IDC_MATERIAL_PREVIEW), WM_PAINT, 0, 0);

		//
		//値の設定
		//
		SetDlgItemInt(hWnd,  IDC_MATERIAL_COLOR_R, (UCHAR)(mat->color.r*255.0f), FALSE);
		SetDlgItemInt(hWnd,  IDC_MATERIAL_COLOR_G, (UCHAR)(mat->color.g*255.0f), FALSE);
		SetDlgItemInt(hWnd,  IDC_MATERIAL_COLOR_B, (UCHAR)(mat->color.b*255.0f), FALSE);
		SetDlgItemInt(hWnd,  IDC_MATERIAL_COLOR_A, (UCHAR)(mat->color.a*255.0f), FALSE);
		SetDlgItemInt(hWnd,  IDC_MATERIAL_SUBDIVIDE,       mat->subdivide, FALSE);

		if(mat->texture!=NULL){
			SetDlgItemText(hWnd, IDC_MATERIAL_TEXTURE_FORMULA, mat->texture[nCurrentTextureLevel]);
		}
		if(mat->texenv!=NULL){
			if(mat->texenv[nCurrentTextureLevel] == 0) mat->texenv[nCurrentTextureLevel]|=0x01;
		}
		
		if(mat->shade == 0) mat->shade |= 0x04;
		CheckDlgButton(hWnd, IDC_MATERIAL_LIGHTING_CONSTANT,   (mat->shade&0x02));
		CheckDlgButton(hWnd, IDC_MATERIAL_LIGHTING_FLAT,   mat->shade&0x01);
		CheckDlgButton(hWnd, IDC_MATERIAL_LIGHTING_SMOOTH, mat->shade&0x04);
		CheckDlgButton(hWnd, IDC_MATERIAL_LIGHTING_WIREFRAME, mat->shade&0x10);

		//
		//キャンセルされた場合に備えてバックアップを取っておく
		//
		if(g_org.texture_id!=NULL){
			GlobalFree(g_org.texture_id);
			g_org.texture_id = NULL;
		}
		if(mat->texture_id!=NULL){
			g_org.texture_id = (unsigned int*)GlobalAlloc(GPTR, GlobalSize(mat->texture_id));
			CopyMemory(g_org.texture_id, mat->texture_id, GlobalSize(mat->texture_id));
		}

		if(g_org.texture != NULL){
			for(i=0; i<g_org.number_of_texture; i++){
				GlobalFree(g_org.texture[i]);
				g_org.texture[i] = NULL;
			}
			GlobalFree(g_org.texture);
			g_org.texture = NULL;
		}
		if(mat->texture!=NULL){
			g_org.texture = (char**)GlobalAlloc(GPTR, sizeof(char*) * mat->number_of_texture);
			for(i=0; i<mat->number_of_texture; i++){
				g_org.texture[i] = (char*)GlobalAlloc(GPTR, /*sizeof(char) * */GlobalSize(mat->texture[i])/*(lstrlen(mat->texture[i])+1)*/);
				//lstrcpy(g_org.texture[i], mat->texture[i]);
				CopyMemory(g_org.texture[i], mat->texture[i], GlobalSize(mat->texture[i]));
			}
		}

		if(g_org.multi_texture_env!=NULL){
			GlobalFree(g_org.multi_texture_env);
			g_org.multi_texture_env = NULL;
		}
		if(mat->multi_texture_env!=NULL){
			g_org.multi_texture_env = (KMultiTextureEnv*)GlobalAlloc(GPTR, GlobalSize(mat->multi_texture_env));
			CopyMemory(g_org.multi_texture_env, mat->multi_texture_env, GlobalSize(mat->multi_texture_env));
		}

		//g_org.texenv = mat->texenv;
		if(g_org.texenv!=NULL){
			GlobalFree(g_org.texenv);
			g_org.texenv = NULL;
		}
		if(mat->texenv!=NULL){
			g_org.texenv = (unsigned char*)GlobalAlloc(GPTR, GlobalSize(mat->texenv));
			CopyMemory(g_org.texenv, mat->texenv, GlobalSize(mat->texenv));
		}

		g_org.number_of_texture = mat->number_of_texture;
		CopyMemory(g_org.mat_name, mat->mat_name, sizeof(char) * 8);
		//lstrcpy(g_org.mat_name, mat->mat_name);
		g_org.shade = mat->shade;
		g_org.subdivide = mat->subdivide;
		g_org.blendf = mat->blendf;
		g_org.color = mat->color;
		g_org.uv_rot = mat->uv_rot;
		g_org.uv_scale = mat->uv_scale;
		g_org.uv_trans = mat->uv_trans;

		//
		//ダイアログが閉じるまでの間、最大まで拡張する
		//
		unsigned int nTextureMax = 8;
		if(mat->number_of_texture < nTextureMax){
			//mat->texture = (char**)GlobalReAlloc(mat->texture, sizeof(char*) * nTextureMax, GMEM_ZEROINIT);
			KMaterial* tmp = new KMaterial();
			tmp->texture = (char**)GlobalAlloc(GPTR, sizeof(char*) * nTextureMax);
			//CopyMemory(tmp->texture, mat->texture, mat->number_of_texture * sizeof(char*));
			for(i=0; i<mat->number_of_texture; i++){
				DWORD dwSize = GlobalSize(mat->texture[i]);
				tmp->texture[i] = (char*)GlobalAlloc(GPTR, dwSize);
				CopyMemory(tmp->texture[i], mat->texture[i], dwSize);
			}

			for(i=0; i<mat->number_of_texture; i++){
				GlobalFree(mat->texture[i]);
				mat->texture[i] = NULL;
			}
			GlobalFree(mat->texture);
			mat->texture = NULL;

			//すりかえる
			mat->texture = tmp->texture;
			for(i=0; i<nTextureMax; i++){
				mat->texture[i] = tmp->texture[i];
				//tmp->texture[i] = NULL;
			}
			//tmp->texture = NULL;

			//mat->texture = (char**)GlobalReAlloc(mat->texture, sizeof(char*) * nTextureMax, GMEM_ZEROINIT);
			//mat->texture[i]は、適宜ロード時に領域が確保される
			//for(i=0; i<nTextureMax - mat->number_of_texture; i++){
			//	mat->texture[i] = (char*)GlobalAlloc(GPTR, sizeof(char) * 
			//}

			//mat->texture_id = (unsigned int*)GlobalReAlloc(mat->texture_id, sizeof(unsigned int) * nTextureMax, GMEM_ZEROINIT);
			tmp->texture_id = (unsigned int*)GlobalAlloc(GPTR, sizeof(unsigned int) * nTextureMax);
			for(i=0; i<mat->number_of_texture; i++){
				tmp->texture_id[i] = mat->texture_id[i];
			}
			//CopyMemory(tmp->texture_id, mat->texture_id, GlobalSize(mat->texture_id));
			GlobalFree(mat->texture_id);
			mat->texture_id = NULL;
			mat->texture_id = tmp->texture_id;
			//tmp->texture_id = NULL;

			//mat->texenv = (unsigned char*)GlobalReAlloc(mat->texenv, sizeof(unsigned char) * nTextureMax, GMEM_ZEROINIT);
			tmp->texenv = (unsigned char*)GlobalAlloc(GPTR, sizeof(unsigned char) * nTextureMax);
			for(i=0; i<mat->number_of_texture; i++){
				tmp->texenv[i] = mat->texenv[i];
			}
			//CopyMemory(tmp->texenv, mat->texenv, GlobalSize(mat->texenv));
			GlobalFree(mat->texenv);
			mat->texenv = NULL;
			mat->texenv = tmp->texenv;
			//tmp->texenv = NULL;

			//mat->multi_texture_env = (KMultiTextureEnv*)GlobalReAlloc(mat->texenv, sizeof(KMultiTextureEnv) * nTextureMax, GMEM_ZEROINIT);
			tmp->multi_texture_env = (KMultiTextureEnv*)GlobalAlloc(GPTR, sizeof(KMultiTextureEnv) * nTextureMax);
			for(i=0; i<mat->number_of_texture; i++){
				tmp->multi_texture_env[i] = mat->multi_texture_env[i];
			}
			//CopyMemory(tmp->multi_texture_env, mat->multi_texture_env, GlobalSize(mat->multi_texture_env));
			GlobalFree(mat->multi_texture_env);
			mat->multi_texture_env = NULL;
			mat->multi_texture_env = tmp->multi_texture_env;
			//tmp->multi_texture_env = NULL;

			for(i=mat->number_of_texture; i<nTextureMax; i++){//足りない分だけ新規の値で埋める
				SetDefaultMultiTextureParams(mat, i);
			}
			delete tmp;
		}
		mat->number_of_texture = nTextureMax;//8枚にする

		//
		//コンボボックスのセットアップ
		//
		int j;
		for(j=0; j<=IDC_MATERIAL_PAGE-IDC_MATERIAL_PAGE; j++){
			HWND hPage = GetDlgItem(hWnd, IDC_MATERIAL_PAGE+j);
			for(i=0; i<sizeof(szPage)/sizeof(char*); i++){
				ComboBox_InsertString(hPage, i, szPage[i]);
			}
			ComboBox_SetCurSel(hPage, 0);//page 1
		}

		for(j=0; j<=IDC_MATERIAL_CHANNEL-IDC_MATERIAL_CHANNEL; j++){//画素チャンネル
			HWND hChannel = GetDlgItem(hWnd, IDC_MATERIAL_CHANNEL+j);
			for(i=0; i<sizeof(szChannel)/sizeof(char*); i++){
				ComboBox_InsertString(hChannel, i, szChannel[i]);
			}
			ComboBox_SetCurSel(hChannel, 0);//rgb
		}
		for(j=0; j<=IDC_MATERIAL_OPERATOR-IDC_MATERIAL_OPERATOR; j++){//操作演算子
			HWND hOperator = GetDlgItem(hWnd, IDC_MATERIAL_OPERATOR+j);
			for(i=0; i<sizeof(szglMultiTextureOperator)/sizeof(char*); i++){
				ComboBox_InsertString(hOperator, i, szglMultiTextureOperator[i]);
			}
		}
		for(j=0; j<=IDC_MATERIAL_SOURCE2-IDC_MATERIAL_SOURCE0; j++){//SOURCE
			HWND hSource = GetDlgItem(hWnd, IDC_MATERIAL_SOURCE0+j);
			for(i=0; i<sizeof(szglMultiTextureSource)/sizeof(char*); i++){
				ComboBox_InsertString(hSource, i, szglMultiTextureSource[i]);
			}
			ComboBox_SetCurSel(hSource, 0);//rgb
		}
		for(j=0; j<=IDC_MATERIAL_OPERAND2-IDC_MATERIAL_OPERAND0; j++){//OPERAND
			HWND hOperand = GetDlgItem(hWnd, IDC_MATERIAL_OPERAND0+j);
			for(i=0; i<sizeof(szglMultiTextureOperand)/sizeof(char*); i++){
				ComboBox_InsertString(hOperand, i, szglMultiTextureOperand[i]);
			}
		}
		for(j=0; j<=IDC_MATERIAL_SCALE-IDC_MATERIAL_SCALE; j++){//SCALE
			HWND hScale = GetDlgItem(hWnd, IDC_MATERIAL_SCALE+j);
			for(i=0; i<sizeof(szglMultiTextureScale)/sizeof(char*); i++){
				ComboBox_InsertString(hScale, i, szglMultiTextureScale[i]);
			}
		}
		for(j=0; j<=IDC_MATERIAL_MAPPING-IDC_MATERIAL_MAPPING; j++){//MAPPING
			HWND hMapping = GetDlgItem(hWnd, IDC_MATERIAL_MAPPING+j);
			for(i=0; i<sizeof(szglMultiTextureMapping)/sizeof(char*); i++){
				ComboBox_InsertString(hMapping, i, szglMultiTextureMapping[i]);
			}
		}

		//体裁を整える
		for(j=0; j<=IDC_MATERIAL_MAPPING-IDC_MATERIAL_PAGE; j++){
			HWND hTarget = GetDlgItem(hWnd, IDC_MATERIAL_PAGE+j);

			RECT rect;
			ComboBox_GetDroppedControlRect(hTarget, &rect);
			int iheight = ComboBox_GetItemHeight(hTarget);
			int iCount = ComboBox_GetCount(hTarget);
			SetWindowPos(hTarget, 0, 0, 0, rect.right - rect.left, iheight * iCount + rect.bottom + 2, SWP_NOMOVE);
		}


		//SFACTOR DFACTOR
		HWND hSfactor = GetDlgItem(hWnd, IDC_MATERIAL_SFACTOR);
		for(i=0; i<sfactornum; i++)
		{
			ComboBox_InsertString(hSfactor, i, szSfactorList[i]);
		}
		HWND hDfactor = GetDlgItem(hWnd, IDC_MATERIAL_DFACTOR);
		for(i=0; i<dfactornum; i++)
		{
			ComboBox_InsertString(hDfactor, i, szDfactorList[i]);
		}
		
		if( (mat->blendf&0x0F)>factornum || (mat->blendf&0x0F)<0 || (mat->blendf>>4)>factornum || (mat->blendf>>4)<0)
		{
			mat->blendf = 0x01;
		}
		for(i=0; i<sfactornum; i++)
		{
			if(lstrcmp(szSfactorList[i], szFactorList[(mat->blendf&0x0F)])==0){
				ComboBox_SetCurSel(hSfactor, i);
				break;
			}else{
				ComboBox_SetCurSel(hSfactor, 1);
			}
		}
		for(j=0; j<dfactornum; j++)
		{
			if(lstrcmp(szDfactorList[j], szFactorList[(mat->blendf>>4)])==0){
				ComboBox_SetCurSel(hDfactor, j);
				break;
			}else{
				ComboBox_SetCurSel(hDfactor, 0);
			}
		}
		if(i==0 && j==0){
			ComboBox_SetCurSel(hSfactor, 1);
			ComboBox_SetCurSel(hDfactor, 0);
		}
		
		//
		//コンボボックスの体裁を整える
		//
		RECT rect;
		ComboBox_GetDroppedControlRect(hSfactor, &rect);
		int iheight = ComboBox_GetItemHeight(hSfactor);
		SetWindowPos(hSfactor, 0, 0, 0, rect.right - rect.left, iheight * sfactornum + rect.bottom + 2, SWP_NOMOVE);
		SetWindowPos(hDfactor, 0, 0, 0, rect.right - rect.left, iheight * dfactornum + rect.bottom + 2, SWP_NOMOVE);

		UpdateTextureControl();
		win.CSetWindowText(hWnd, "%d:%s", item, mat->mat_name);
		return TRUE;
	}
	case WM_PAINT:
	{
	}
	case WM_COMMAND:
	{
		switch(LOWORD(wParam))
		{
			case IDOK:
			{
				int item = selected_material;
				KMaterial* mat = mdl.GetMaterial(item);
				if(mat==NULL) return FALSE;

				unsigned int i;

				//lighting
				mat->shade = 0;
				if(IsDlgButtonChecked(hWnd, IDC_MATERIAL_LIGHTING_CONSTANT)){
					mat->shade |= 0x02;
				}else if(IsDlgButtonChecked(hWnd, IDC_MATERIAL_LIGHTING_FLAT)){
					mat->shade |= 0x01;
				}else if(IsDlgButtonChecked(hWnd, IDC_MATERIAL_LIGHTING_SMOOTH)){
					mat->shade |= 0x04;
				}
				if(IsDlgButtonChecked(hWnd, IDC_MATERIAL_LIGHTING_WIREFRAME)) mat->shade |= 0x10;

				//BlendFunc
				mat->blendf = 0;
				HWND hSfactor = GetDlgItem(hWnd, IDC_MATERIAL_SFACTOR);
				int sSelectedItem = ComboBox_GetCurSel(hSfactor);
				for(i=0; i<factornum; i++)
				{
					if(lstrcmp(szSfactorList[sSelectedItem], szFactorList[i])==0){
						mat->blendf |= i;
						break;
					}
				}
				
				HWND hDfactor = GetDlgItem(hWnd, IDC_MATERIAL_DFACTOR);
				int dSelectedItem = ComboBox_GetCurSel(hDfactor);
				for(i=0; i<factornum; i++)
				{
					if(lstrcmp(szDfactorList[dSelectedItem], szFactorList[i])==0){
						mat->blendf |= i<<4;
						break;
					}
				}

				//Exit Dialog
				EndDialog(hWnd, 0);
				return TRUE;
			}
			case IDCANCEL:
			{
				EndDialog(hWnd, -1);
				return TRUE;
			}
			case IDC_MATERIAL_LOAD_TEXTURE:
			{
				char szFile[MAX_PATH];
				char szLoadedTexture[1024*64];
				if(GetOpenFileNameSingle(hWnd, "ktf", szFile, FALSE))
				{
					if(ktex.Load(szFile, szLoadedTexture, sizeof(szLoadedTexture)))
					{
						SetDlgItemText(hWnd, IDC_MATERIAL_TEXTURE_FORMULA, szLoadedTexture);
						MaterialCreateTexture(item, szLoadedTexture);
					}else{
						MessageBox(NULL, "ERROR: MAY BE IT'S NOT A KTF FILE.\nIF IT IS, PLEASE FIXME.\nTECH. INFO: LOADING KTF FILE AT MaterialWindow.cpp::MaterialDlgProc::IDC_MATERIAL_LOAD_TEXTURE, KTextureEdit::Load() RETURNED ZERO.","FIXME?", MB_OK|MB_ICONERROR);
					}
				}
				return TRUE;
			}
			case IDC_MATERIAL_CLEAR_TEXTURE:
			{
				KMaterial* mat = mdl.GetMaterial(item);
				if(!mat) return TRUE;

				//mat->number_of_texture = 0;
				//GlobalFree(mat->texture_id);
				//mat->texture_id = NULL;
				//lstrcpy(mat->texture, "");

				mat->texture_id[nCurrentTextureLevel] = 0;

				GlobalFree(mat->texture[nCurrentTextureLevel]);
				mat->texture[nCurrentTextureLevel] = NULL;
				mat->texture[nCurrentTextureLevel] = (char*)GlobalAlloc(GPTR, 1);

				SetDlgItemText(hWnd, IDC_MATERIAL_TEXTURE_FORMULA, mat->texture[nCurrentTextureLevel]);
				return TRUE;
			}
			case IDC_MATERIAL_INIT_PARAM:
			{
				KMaterial* mat = mdl.GetMaterial(item);
				if(!mat) return TRUE;
				SetDefaultMultiTextureParams(mat, nCurrentTextureLevel);
				UpdateTextureControl();
			}
		}
	}
	case WM_DESTROY:
	{
		break;
	}
	case WM_NOTIFY:
	{
		break;
	}
	}
	return FALSE;
}

int MaterialGetSelectedMaterial()
{
	return selected_material;
}

int MaterialMouse(long x, long y, UINT btn)
{
	selected_material = vMaterial.GetSelectedItem(); 
	if(btn==WM_LBUTTONDBLCLK)
	{
//		if(mdl.GetCloneAllocPtr()!=NULL)
//		{
			int item = selected_material;
			if(item<0) return FALSE;
		
			KMaterial* mat = mdl.GetMaterial(item);
			if(!mat){
				mdl.CreateMaterial(item);
			}
			MaterialCommand(vMaterial.GetHolderWnd(), (ID_ITEMVIEW_PROPERTY), 0);
//		}
	}
	return FALSE;
}

void GetMaterial()
{
	RefreshMaterialList();
}