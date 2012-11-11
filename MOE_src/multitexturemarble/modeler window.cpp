#include "stdafx.h"
#include "modeler window.h"
#include "clonewindow.h"
#include "materialwindow.h"
#include "startupcode.h"

#include "../klib/matrix.h"
#include "../gl/glext.h"

extern CWindowGL win;
extern KModelEdit mdl;//宣言


#include <vector>
using namespace std;
vector<KModelEdit> undo_mdl,redo_mdl;


KCloneData cpy_clonedata;//for KOPY

long select_clonepnum=-1;

#include "dkText.h"
#include "glImage.h"
dkText txt;
glImage font[128];

//マルチテクスチャ拡張用エントリポイント
PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1fARB;
PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
PFNGLMULTITEXCOORD3FARBPROC		glMultiTexCoord3fARB;
PFNGLMULTITEXCOORD4FARBPROC		glMultiTexCoord4fARB;
PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;

unsigned char keydata[256];
long view_mode=0;
long help_mode=0;
long texture_mode=0;

#define MODIFY_TRANSLATION		0
#define MODIFY_ROTATION			1
#define MODIFY_SCALING			2
#define MODIFY_SELECTION		3
#define MODIFY_MATERIAL			4

long ModifyMode=MODIFY_TRANSLATION;

static BOOL bLPressing=FALSE,bRPressing=FALSE,bMPressing=FALSE,bShiftPressing=FALSE;
static int select_axis[3] = {0,0,0};

//=========================UNDO & REDO =================================
#include <vector>
using namespace std;

void SetUndo()
{
	undo_mdl.push_back(mdl);
	redo_mdl.clear();
}

void Undo()
{
	long udsize = (long)undo_mdl.size()-1;
	if(udsize>=0){
		//primitiveのみ現在の値を引き継ぐ
		undo_mdl[udsize].CopyMaterialPrimitive(mdl);

		redo_mdl.push_back(mdl);
		mdl = undo_mdl[udsize];
		undo_mdl.pop_back();
		RefreshAllView();
	}
}

void Redo()
{
	long rdsize = (long)redo_mdl.size()-1;
	if(rdsize>=0){
		//primitiveのみ現在の値を引き継ぐ
		redo_mdl[rdsize].CopyMaterialPrimitive(mdl);

		undo_mdl.push_back(mdl);
		mdl = redo_mdl[rdsize];
		redo_mdl.pop_back();
		RefreshAllView();
	}
}

//======================FONT=============================================
void InitFont()
{
	long i;
	txt.dkSetTextColor(55,255,55);
	txt.dkSetFontSize(128);
	txt.dkSetBold(true);
	txt.dkSetItalic(false);
	for(i=0; i<128; i++){
		font[i].Create(128,128,0,0,0);
		char str[2];
		str[0]=(char)i;
		str[1]='\0';
		txt.dkPrint(font[i].GetDC(),24,0,str);
		font[i].Convert();
	}
}


void DrawSquare()
{
	glBegin(GL_QUADS);
		glTexCoord2i(0,0); glVertex3f(-5.0f, -5.0f, 0.0f);
		glTexCoord2i(0,1); glVertex3f(-5.0f, 5.0f, 0.0f);
		glTexCoord2i(1,1); glVertex3f(5.0f, 5.0f, 0.0f);
		glTexCoord2i(1,0); glVertex3f(5.0f, -5.0f, 0.0f);
	glEnd();
}

void DrawString(const char* str)
{
	int n = lstrlen(str);
	int i;
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glColor4f(1,1,1,0);
		glRotated(180,0,1,0);
		glScalef(0.01f, 0.01f, 0.01f);
		for(i=0; i<n; i++){
			glBindTexture(GL_TEXTURE_2D,font[str[i]].GetTextureNum());
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTranslatef(6.0f, 0.0f, 0.0f);
			DrawSquare();
		}
		//glEnable(GL_LIGHTING);
		//glEnable(GL_DEPTH_TEST);
	glPopAttrib();
	glPopMatrix();
}



void DispInfo(int helpmode)
{
	const float aspect = win.GetWidth() / (float)win.GetHeight();
	glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		gluPerspective( 60.0, aspect, 1, 2000);
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	char buf[128];
	glRotatef(180,0.0f,1.0f,0);
	long pnum = select_clonepnum;
	KCloneData* bt=NULL;
	KCloneData texdt;
	if(texture_mode==1){
		int mat = MaterialGetSelectedMaterial();
		if(mat!=-1){
			KMaterial* mtex = mdl.GetMaterial(mat);
			if(mtex!=NULL){
				texdt.pos = mtex->uv_trans;
				texdt.rot = mtex->uv_rot;
				texdt.scale = mtex->uv_scale;
				texdt.clone_name = "TextureMode";
				bt = &texdt;
			}
		}
	}else{
		long pnum = select_clonepnum;
		if((pnum>=0)&&(pnum<mdl.GetCloneAllocNum())){
			KClone* cln = mdl.GetCloneAllocPtr();
			bt = &(cln[select_clonepnum].clone_data);
		}	
	}
	
	if(bt!=NULL){
		//KClone* kcl = mdl.GetCloneAllocPtr();
		KCloneData clone_list = *bt;//kcl[pnum].clone_data;
		glPushMatrix();
			glTranslated(2,1.6,3);
			sprintf(buf,"Clone Info:    [%s]",clone_list.clone_name);
			DrawString(buf);
			glTranslated(0,-0.1,0);

			sprintf(buf,"Trans: (%.3f,%.3f,%.3f)",hftof(ftohf(clone_list.pos.x))
												,hftof(ftohf(clone_list.pos.y))
												,hftof(ftohf(clone_list.pos.z)));
			DrawString(buf);
			glTranslated(0,-0.1,0);
			sprintf(buf,"Rot  : (%.3f,%.3f,%.3f)",ctod(dtoc(clone_list.rot.x))
												,ctod(dtoc(clone_list.rot.y))
												,ctod(dtoc(clone_list.rot.z)));
			DrawString(buf);
			glTranslated(0,-0.1,0);
			sprintf(buf,"Scale: (%.3f,%.3f,%.3f)",hftof(ftohf(clone_list.scale.x))
												,hftof(ftohf(clone_list.scale.y))
												,hftof(ftohf(clone_list.scale.z)));
			DrawString(buf);
			
			//mat
			glTranslated(0,-0.1,0);
			KMaterial* km = mdl.GetMaterial(clone_list.material_id);
			if(km!=NULL) sprintf(buf,"Use Material:  %3d-%s",clone_list.material_id,km->mat_name);
			else		 sprintf(buf,"Use Material:  none");
			DrawString(buf);
			
			//primitive
			glTranslated(0,-0.1,0);
			KObject* kp = mdl.GetPrimitive(clone_list.primitive_id);
			if(kp!=NULL) sprintf(buf,"Use Primitive: %3d-%s",clone_list.primitive_id,kp->GetName());
			else		 sprintf(buf,"Use Primitive:  none");		
			DrawString(buf);
			
		glPopMatrix();
	}

	if(helpmode==1){
		glPushMatrix();
			glTranslated(2,-1.1,3);
			DrawString("Axis:  X   Y   Z  Free Reset");glTranslated(0,-0.1,0);
			DrawString("KEY : [X] [Y] [Z] none  [R] ");glTranslated(0,-0.1,0);
			glPushMatrix();
				if(ModifyMode==MODIFY_SELECTION){
					glTranslated(0.1f,-0.0f,0);
					DrawString("*");
				}else if(ModifyMode==MODIFY_TRANSLATION){
					glTranslated(0.1f,-0.1f,0);
					DrawString("*");
				}else if(ModifyMode==MODIFY_ROTATION){
					glTranslated(0.1f,-0.2f,0);
					DrawString("*");
				}else if(ModifyMode==MODIFY_SCALING){
					glTranslated(0.1f,-0.3f,0);
					DrawString("*");
				}
			glPopMatrix();
			DrawString("Selection  : [S]");glTranslated(0,-0.1,0);
			DrawString("Translation: [V]");glTranslated(0,-0.1,0);
			DrawString("Rotation   : [C]");glTranslated(0,-0.1,0);
			DrawString("Scaling    : [Q]");glTranslated(-2.0,0,0);
			DrawString("View: Right[F1] Top[F2] Front[F3]");glTranslated(0,0.1,0);
			DrawString("MiddleClick Emu : M Key+LeftClick");glTranslated(0,0.1,0);
			//sprintf(buf,"Undo [U] stock(%d)",clone_undo.size());
			//DrawString(buf);glTranslated(0,0.1,0);
			char* vms[] ={"Pers","Orth"};
			sprintf(buf,"ViewMode=%s [P]or[O]",vms[view_mode]);
			DrawString("Select Object Visible Change [SPACE]");glTranslated(0,0.1,0);
			DrawString("Select Object Lock Change [L]");glTranslated(0,0.1,0);
			DrawString("Select Object Copy [K]");glTranslated(0,0.1,0);
			DrawString("Select Object Delete [Del]");glTranslated(0,0.1,0);
			DrawString("Hide This Help [H]");
		glPopMatrix();
	}else{
		glTranslated(1.5,-1.4,3);
		if(texture_mode)  DrawString("[MaterialMode]");
		glTranslated(0,-0.1,0);
		int mat = MaterialGetSelectedMaterial();
		
		if(mat!=-1){
			KMaterial* km = mdl.GetMaterial(mat);
			if(km!=NULL){
				sprintf(buf,"Selected Material > %3d-%s",mat,km->mat_name);
			}else{
				buf[0]='\0';
			}
		}else{
			sprintf(buf,"Selected Material > none");
		}
		DrawString(buf);
		glTranslated(0,-0.1,0);
		if(ModifyMode==MODIFY_SELECTION)		DrawString("MODE:Selection  ");
		else if(ModifyMode==MODIFY_TRANSLATION) DrawString("MODE:Translation");
		else if(ModifyMode==MODIFY_ROTATION)    DrawString("MODE:Rotation   ");
		else if(ModifyMode==MODIFY_SCALING)     DrawString("MODE:Scaling    ");
		else if(ModifyMode==MODIFY_MATERIAL)    DrawString("MODE:SetMaterial");
		glTranslated(-2.0,0.0,0);
		DrawString("Help Mode [H]");
		glTranslated(0,0.1,0);
		sprintf(buf,"Undo_num = %d",(long)undo_mdl.size());
		DrawString(buf);
		glTranslated(0,0.1,0);
		sprintf(buf,"Redo_num = %d",(long)redo_mdl.size());
		DrawString(buf);
	}
}
//===============================================================================
//---------model view state------------------
float grotX=0.0f;
float grotY=0.0f;
float gTransX = 0.0f;
float gTransY = 0.0f;
float gZoom=10.0f;
float lmoveX=0.0f;
float lmoveY=0.0f;
CMatrix3 rot_state;
CVector tvec=CVector(0,0,0);

long wireframe_mode=0;

void ModelerSelectClone(KClone* clone)
{
	long ccnt=0;
	KClone* cptr = mdl.GetCloneAllocPtr();
	while(&cptr[ccnt]!=clone){
		ccnt++;
		if(ccnt>=mdl.GetCloneAllocNum()) break;
	}
	select_clonepnum = ccnt;
}

KClone* GetSelectClone()
{
	KClone* cln = mdl.GetCloneAllocPtr();
	return &cln[select_clonepnum];
}


//-------------------------------------------

int CheckForExtension(const char* szSrchExt){//指定された文字列が示す拡張がサポートされているか調べる
	const char* extensions = (char*)glGetString(GL_EXTENSIONS);
	int isExtensionFound = 0;
	if(strstr(extensions, szSrchExt) != NULL){
		isExtensionFound = 1;
	}
	return isExtensionFound;
}

void InitModelerWindow()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0.01f);
	glEnable(GL_COLOR_MATERIAL);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	glLightf(GL_LIGHT0,GL_DIFFUSE,0.6f);
	
	rot_state.SetRotateX(0.0f);

	InitFont();

	//
	//マルチテクスチャ拡張初期化
	//
	const char* szMultiTextureExt[] = {
	"GL_ARB_multitexture",		//MultiTexture
	"GL_EXT_texture_env_combine"//texture_env_combining
	};
	int i;
	int isMTSupported = 0;
	for(i=0; i<sizeof(szMultiTextureExt)/sizeof(szMultiTextureExt[0]); i++){
		isMTSupported |= CheckForExtension(szMultiTextureExt[i]);
	}
	if(isMTSupported==0){
		FatalAppExit(0, "This application requires a video card which supports \"GL_ARB_multitexture\" and \"GL_EXT_texture_env_combine\" extensions. Unfortunately, your one doesn't. Buy new video card(ATi-RADEON preferred).");
	}

	//get entry points
	glMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC)wglGetProcAddress("glMultiTexCoord1fARB");
	glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
	glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)wglGetProcAddress("glMultiTexCoord3fARB");
	glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)wglGetProcAddress("glMultiTexCoord4fARB");
	glActiveTextureARB	 = (PFNGLACTIVETEXTUREARBPROC)	wglGetProcAddress("glActiveTextureARB");
}

void DrawMesh(float size,float trans_x,float trans_y,float trans_z)
{
	glTranslatef(trans_x,trans_y,trans_z);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glLineWidth(1);
	glColor3f(0.5,0.5,0.5);
	glBegin(GL_LINES);
		float i;
		for(i=-size; i<=size; i+=size/10.0f){
			glVertex3f(-size,0,i); glVertex3f(size,0,i);
			glVertex3f(i,0,-size); glVertex3f(i,0,size);
		}
	glEnd();
	glLineWidth(1);
}


void DrawAxis(float size,float trans_x,float trans_y,float trans_z, float lwidth=8.0)
{
	glTranslatef(trans_x,trans_y,trans_z);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glLineWidth(lwidth);
	glLoadName(1);
	glBegin(GL_LINES);
		glColor3f(1,0,0);
		glVertex3f(-size,0,0); glVertex3f(size,0,0);
		glVertex3f(size,0,0);glVertex3f(size*0.9f,size*0.05f,0);
	glEnd();
	glLoadName(2);
	glBegin(GL_LINES);
		glColor3f(0,1,0);
		glVertex3f(0,-size,0); glVertex3f(0,size,0);
		glVertex3f(0,size,0);glVertex3f(size*0.05f,size*0.9f,size*0.05f);
	glEnd();
	glLoadName(3);
	glBegin(GL_LINES);
		glColor3f(0,0,1);
		glVertex3f(0,0,-size); glVertex3f(0,0,size);
		glVertex3f(0,0,size);glVertex3f(0,size*0.05f,size*0.9f);
	glEnd();
	glLineWidth(1);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_TEXTURE_2D);
}


void DrawRotAxis(float size,float rot_x,float rot_y,float rot_z)
{
	long i;
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glLineWidth(8);
	glRotatef(rot_x,1,0,0);
		float xclr[] = {1,0,0,0.8f};
		glColor4fv(xclr);
		glLoadName(1);
		glBegin(GL_LINE_LOOP);
		for(i=0; i<360; i++){
			glNormal3f(0,size*cosf((i-90.0f)/180.0f*PI),size*sinf((i-90.0f)/180.0f*PI));
			glVertex3f(0,size*cosf((i-90.0f)/180.0f*PI),size*sinf((i-90.0f)/180.0f*PI));
		}
		glEnd();
	glRotatef(rot_y,0,1,0);
		float yclr[] = {0,1,0,0.8f};
		glColor4fv(yclr);
		glLoadName(2);
		glBegin(GL_LINE_LOOP);
		for(i=0; i<360; i++){
			glNormal3f(size*cosf((i-90.0f)/180.0f*PI),0,size*sinf((i-90.0f)/180.0f*PI));
			glVertex3f(size*cosf((i-90.0f)/180.0f*PI),0,size*sinf((i-90.0f)/180.0f*PI));
		}
		glEnd();
	glRotatef(rot_z,0,0,1);
		float zclr[] = {0,0,1,0.8f};
		glColor4fv(zclr);
		glLoadName(3);
		glBegin(GL_LINE_LOOP);
		for(i=0; i<360; i++){
			glNormal3f(size*cosf((i-90.0f)/180.0f*PI),size*sinf((i-90.0f)/180.0f*PI),0);
			glVertex3f(size*cosf((i-90.0f)/180.0f*PI),size*sinf((i-90.0f)/180.0f*PI),0);
		}
		glEnd();
	glLineWidth(1);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
}


void DrawScaleAxis(float size,float lwidth=8.0)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glLineWidth(lwidth);
	glLoadName(1);
	glBegin(GL_LINES);
		glColor3f(1,0,0);	
		glVertex3f(0,0,0); glVertex3f(size,0,0);
		glVertex3f(size,0,0);glVertex3f(size*0.9f,size*0.05f,0);
	glEnd();
	glLoadName(2);
	glBegin(GL_LINES);
		glColor3f(0,1,0);
		glVertex3f(0,0,0); glVertex3f(0,size,0);
		glVertex3f(0,size,0);glVertex3f(size*0.05f,size*0.9f,size*0.05f);
	glEnd();
	glLoadName(3);
	glBegin(GL_LINES);
		glColor3f(0,0,1);	
		glVertex3f(0,0,0); glVertex3f(0,0,size);
		glVertex3f(0,0,size);glVertex3f(0,size*0.05f,size*0.9f);
	glEnd();
	glLineWidth(1);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
}

void DrawPointer()
{
	glDisable(GL_LIGHTING);
	glPushMatrix();
		KClone* cln = mdl.GetCloneAllocPtr();
		long cln_num = mdl.GetCloneAllocNum();
		KCloneData* bt=NULL;
		KCloneData texdt;
		if(texture_mode==1){
			int mat = MaterialGetSelectedMaterial();
			if(mat!=-1){
				KMaterial* mtex = mdl.GetMaterial(mat);
					if(mtex!=NULL){
					texdt.pos = mtex->uv_trans;
					texdt.rot = mtex->uv_rot;
					texdt.scale = mtex->uv_scale;
					bt = &texdt;
				}
			}
		}else{
			long pnum = select_clonepnum;
			if((pnum>=0)&&(pnum<mdl.GetCloneAllocNum())){
				KClone* cln = mdl.GetCloneAllocPtr();
				bt = &(cln[select_clonepnum].clone_data);
			}	
		}
		
		if(bt!=NULL){
			KCloneData* cd = bt;
			if(ModifyMode==MODIFY_TRANSLATION){
				DrawAxis(gZoom/3.0f,hftof(ftohf(cd->pos.x)),hftof(ftohf(cd->pos.y)),hftof(ftohf(cd->pos.z)));
			}else{
				glTranslatef(hftof(ftohf(cd->pos.x)),hftof(ftohf(cd->pos.y)),hftof(ftohf(cd->pos.z)));
			}
			if(ModifyMode==MODIFY_ROTATION){
				DrawRotAxis(gZoom/3.0f,ctod(dtoc(cd->rot.x)),ctod(dtoc(cd->rot.y)),ctod(dtoc(cd->rot.z)));
			}else{
				glRotatef(ctod(dtoc(cd->rot.x)),1,0,0);
				glRotatef(ctod(dtoc(cd->rot.y)),0,1,0);
				glRotatef(ctod(dtoc(cd->rot.z)),0,0,1);
			}
			if(ModifyMode==MODIFY_SCALING){
				DrawScaleAxis(gZoom/3.0f);
			}
		}
	glPopMatrix();
}

void DrawObject(int select_mode=0, int wire_mode=0)
{
	glLoadIdentity();
	glPushMatrix();		
		glTranslated(0,0,-gZoom);
		CMatrix3 rotX,rotY;
		rotX.SetRotateX(-grotX);
		rotY.SetRotateY(grotY);
		grotX=0;
		grotY=0;
		
		if((gTransX!=0.0f)||(gTransY!=0.0f)){
			tvec += rot_state*CVector(gTransX,gTransY,0); 
			gTransX=0.0f;
			gTransY=0.0f;
		}
		
		rot_state = rot_state*rotY*rotX;
		float rm[] = {rot_state.m[0].x,rot_state.m[0].y,rot_state.m[0].z,0
					 ,rot_state.m[1].x,rot_state.m[1].y,rot_state.m[1].z,0
					 ,rot_state.m[2].x,rot_state.m[2].y,rot_state.m[2].z,0
					 ,0				  ,0			   ,0			    ,1};
		glMultMatrixf(rm);
		glTranslatef(tvec.x,tvec.y,tvec.z);
		if(select_mode==0){
			DrawMesh(10,0,0,0);
			DrawScaleAxis(100,3);//world
		}
	
		glPushMatrix();
			mdl.Draw(wireframe_mode, NULL);
		glPopMatrix();
		glPushMatrix();
			KClone* cln = mdl.GetCloneAllocPtr();
			long cln_num = mdl.GetCloneAllocNum();
			if((select_clonepnum>=0)&&(select_clonepnum<cln_num)){//axis
				mdl.DrawFunc(&cln[select_clonepnum],DrawPointer);
			}
		glPopMatrix();
	glPopMatrix();	
}

inline void DrawCube()
{
	glBegin(GL_QUADS);
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glVertex3f(-1.0f, -1.0f,  1.0f);
		glVertex3f( 1.0f, -1.0f,  1.0f);
		glVertex3f( 1.0f,  1.0f,  1.0f);
		glVertex3f(-1.0f,  1.0f,  1.0f);
		glNormal3f( 0.0f, 0.0f,-1.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		glVertex3f(-1.0f,  1.0f, -1.0f);
		glVertex3f( 1.0f,  1.0f, -1.0f);
		glVertex3f( 1.0f, -1.0f, -1.0f);
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glVertex3f(-1.0f,  1.0f, -1.0f);
		glVertex3f(-1.0f,  1.0f,  1.0f);
		glVertex3f( 1.0f,  1.0f,  1.0f);
		glVertex3f( 1.0f,  1.0f, -1.0f);
		glNormal3f( 0.0f,-1.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		glVertex3f( 1.0f, -1.0f, -1.0f);
		glVertex3f( 1.0f, -1.0f,  1.0f);
		glVertex3f(-1.0f, -1.0f,  1.0f);
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glVertex3f( 1.0f, -1.0f, -1.0f);
		glVertex3f( 1.0f,  1.0f, -1.0f);
		glVertex3f( 1.0f,  1.0f,  1.0f);
		glVertex3f( 1.0f, -1.0f,  1.0f);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		glVertex3f(-1.0f, -1.0f,  1.0f);
		glVertex3f(-1.0f,  1.0f,  1.0f);
		glVertex3f(-1.0f,  1.0f, -1.0f);
	glEnd();
}


void Render()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	const float aspect = win.GetWidth() / (float)win.GetHeight();
	if(view_mode==0){
		gluPerspective( 60.0, aspect, 1, 2000);
	}else if(view_mode==1){
		glOrtho(-gZoom/15.0f*10.0*aspect,gZoom/15.0f*10.0*aspect,-gZoom/15.0f*10.0,gZoom/15.0f*10.0f,-100,2000);
	}
	glMatrixMode(GL_MODELVIEW);

	if(texture_mode!=1) win.ClearScreen(0.3f,0.3f,0.3f,1.0f);
	else			    win.ClearScreen(0.7f,0.7f,0.7f,1.0f);
		glLoadIdentity();
		//float lpos[] = {1,1,1};
		//glLightfv(GL_LIGHT0, GL_POSITION, lpos);
		DrawObject();
		DispInfo(help_mode);
	win.RedrawScreen();
}

#define SELECTIONS 1000
void SelectDraw(long mx,long my, long window_width, long window_height)
{
	static GLuint selection[SELECTIONS];  /* セレクションバッファ　　　　　 */
	static GLint hits = 0;                /* セレクトされたオブジェクトの数 */

	GLint vp[4];
	glSelectBuffer(SELECTIONS, selection);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);//dummy

	glMatrixMode(GL_PROJECTION);
		glPushMatrix();//Push Projection Matrix
		glLoadIdentity();
		glGetIntegerv(GL_VIEWPORT, vp);
		gluPickMatrix(mx+5, window_height - my, 12, 10, vp);//mouse point->window pos(upsidedown)
		if(view_mode==0){
			gluPerspective( 60.0, (GLdouble)window_width/(GLdouble)window_height, 1, 2000);
		}else if(view_mode==1){
			GLdouble aspect = (GLdouble)window_width/(GLdouble)window_height;
			glOrtho(-gZoom/15.0f*10.0*aspect,gZoom/15.0f*10.0*aspect,-gZoom/15.0f*10.0,gZoom/15.0f*10.0f,-100,2000);
		}
	glMatrixMode(GL_MODELVIEW);
	
	glLoadIdentity();
	glPushMatrix();
		DrawObject(1);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
		glPopMatrix();//Pop Projection Matrix
	glMatrixMode(GL_MODELVIEW);
	
	hits = glRenderMode(GL_RENDER);
	if(hits>0){
		unsigned int* sb = selection;
		float  snear[256], sfar[256];
		unsigned int sobj[256];
		int i;
		for (i=0; i<hits; i++) {
			/* セレクションバッファの４つ目の要素（添字＝３）から選択されたオブジェクトの番号が入っている */
			long si  = *sb;									sb++;
			snear[i] = (float)(*sb) / (float)0x7fffffff;	sb++;
			sfar[i]  = (float)(*sb) / (float)0x7fffffff;	sb++;
			int sn;
			for(sn=0; sn<si; sn++){
				sobj[i]  = *sb;								sb++;
			}
		}
		float mnear=snear[0];
		unsigned int mobj=sobj[0];
		for (i=1; i<hits; i++){//Zbuffer比較
			if(mnear>snear[i]){
				mnear = snear[i];
				mobj  = sobj[i];
			}
			
		}
		
		if(mobj==1){
			select_axis[0]=1;
		}else if(mobj==2){
			select_axis[1]=1;
		}else if(mobj==3){
			select_axis[2]=1;
		}else{
			long selectobj = mobj-10;//0-9は軸
			if((selectobj>=0)&&(selectobj<(long)mdl.GetCloneAllocNum())){
				select_clonepnum = selectobj;
				KClone* cln = mdl.GetCloneAllocPtr();
				CloneSelectClone(&cln[select_clonepnum]);
			}
		}	
	}
}

void KeyInput()
{
	bLPressing   = (GetAsyncKeyState(VK_LBUTTON)&0x8000)==0x8000 ? 1:0;
	bRPressing   = (GetAsyncKeyState(VK_RBUTTON)&0x8000)==0x8000 ? 1:0;
	bShiftPressing = (GetAsyncKeyState(VK_SHIFT)&0x8000)==0x8000 ? 1:0;
	if(bLPressing==0){
		select_axis[0] = select_axis[1] = select_axis[2] = 0;
	}
	

	if(keydata['F']==1) {
		keydata['X'] = 1;
		keydata['Y'] = 1;
		keydata['Z'] = 1;
	}
	else
	{
		keydata['X'] = 0;
		keydata['Y'] = 0;
		keydata['Z'] = 0;

		if(select_axis[0]==1) keydata['X'] = 1;
		else keydata['X'] = (GetAsyncKeyState('X')&0x8000)==0x8000 ? 1:0;
		if(select_axis[1]==1) keydata['Y'] = 1;
		else keydata['Y'] = (GetAsyncKeyState('Y')&0x8000)==0x8000 ? 1:0;
		if(select_axis[2]==1) keydata['Z'] = 1;
		else keydata['Z'] = (GetAsyncKeyState('Z')&0x8000)==0x8000 ? 1:0;
	}

	static long mousedrag=0;
	if(keydata['T']==1) texture_mode=(++texture_mode)&1;

	if(keydata['V']==1) ModifyMode=MODIFY_TRANSLATION;
	else if(keydata['C']==1) ModifyMode=MODIFY_ROTATION;
	else if(keydata['Q']==1) ModifyMode=MODIFY_SCALING;
	else if(keydata['S']==1) ModifyMode=MODIFY_SELECTION;
	else if(keydata['M']==1) ModifyMode=MODIFY_MATERIAL;

	float rate = 0.05f*gZoom/15.0f;
	CVector mv = rot_state*CVector(-lmoveY*rate,-lmoveX*rate,0);
	KCloneData* bt=NULL;
	KCloneData texdt;
	if(texture_mode==1){
		int mat = MaterialGetSelectedMaterial();
		if(mat!=-1){
			KMaterial* mtex = mdl.GetMaterial(mat);
			if(mtex){
				texdt.pos = mtex->uv_trans;
				texdt.rot = mtex->uv_rot;
				texdt.scale = mtex->uv_scale;
				texdt.visible = 0;
				bt = &texdt;
			}
		}
	}else{
		long pnum = select_clonepnum;
		if((pnum>=0)&&(pnum<mdl.GetCloneAllocNum())){
			KClone* cln = mdl.GetCloneAllocPtr();
			bt = &(cln[select_clonepnum].clone_data);
		}	
	}
	if(bt!=NULL){
		if((bt->lock==0)&&(bt->visible==0)){//LOCK
			KCloneData bef_bt=*bt;
			if(ModifyMode==MODIFY_ROTATION){//ROTATE
				if(keydata['Y']==1){
					bt->rot.y += mv.x/0.05f;
				}if(keydata['X']==1){
					bt->rot.x += -mv.y/0.05f;
				}if(keydata['Z']==1){
					bt->rot.z += (mv.x+mv.y)/0.05f;
				}if(keydata['R']==1){
					bt->rot = CVector(0,0,0);
				}
			}else if(ModifyMode==MODIFY_SCALING){//SCALE
				if((keydata['X']==1)&&(keydata['Y']==1)&&(keydata['Z']==1)){
					CVector ma(mv.y+mv.x,mv.y+mv.x,mv.y+mv.x);
					bt->scale += ma;
				}if(keydata['Y']==1){
					bt->scale.y += (mv.y*2);
				}if(keydata['X']==1){
					bt->scale.x += (mv.x*2);
				}if(keydata['Z']==1){
					bt->scale.z += (mv.z*2);
				}if(keydata['R']==1){
					bt->scale = CVector(1,1,1);
				}else{
					//bt->scale += (mv*2);
				}
			}else if(ModifyMode==MODIFY_TRANSLATION){//Translation
				if(keydata['Y']==1){
					bt->pos.y += mv.y;
				}if(keydata['X']==1){
					bt->pos.x +=mv.x;
				}if(keydata['Z']==1){
					bt->pos.z += mv.z;
				}if(keydata['R']==1){
					bt->pos = CVector(0,0,0);
				}
			}else if(ModifyMode==MODIFY_MATERIAL){//Material
				if(bLPressing){
					static KCloneData* sbt=NULL;
					static long ssmat=-1;
					long smat = MaterialGetSelectedMaterial();
					if((sbt!=bt)||(ssmat!=smat)){
						SetUndo();
						if(smat!=-1) bt->material_id = (unsigned char)smat;
						sbt = bt;
						ssmat = smat;
					}
				}
			}

			if(keydata['L']){//LOAD
				SetUndo();
				if(ModifyMode==MODIFY_TRANSLATION){
					bt->pos = cpy_clonedata.pos;
					if(keydata['X']==1) bt->pos.x = -bt->pos.x;
					if(keydata['Y']==1) bt->pos.y = -bt->pos.y;
					if(keydata['Z']==1) bt->pos.z = -bt->pos.z;
				}else if(ModifyMode==MODIFY_ROTATION){
					bt->rot = cpy_clonedata.rot;
					if(keydata['X']==1) bt->rot.x = -bt->rot.x;
					if(keydata['Y']==1) bt->rot.y = -bt->rot.y;
					if(keydata['Z']==1) bt->rot.z = -bt->rot.z;
				}else if(ModifyMode==MODIFY_SCALING){
					bt->scale = cpy_clonedata.scale;
					if(keydata['X']==1) bt->scale.x = -bt->scale.x;
					if(keydata['Y']==1) bt->scale.y = -bt->scale.y;
					if(keydata['Z']==1) bt->scale.z = -bt->scale.z;
				}else{
					*bt = cpy_clonedata;
					bt->clone_name = (char*)GlobalAlloc(GPTR,lstrlen(cpy_clonedata.clone_name)+1);
					lstrcpy(bt->clone_name,cpy_clonedata.clone_name);
					if(keydata['X']==1) bt->pos.x = -bt->pos.x;
					if(keydata['Y']==1) bt->pos.y = -bt->pos.y;
					if(keydata['Z']==1) bt->pos.z = -bt->pos.z;
				}
				keydata['L']=0;
				RefreshCloneTree();
			}

			if((mousedrag==0)&&(bLPressing==TRUE)
			&& ((bef_bt.primitive_id!=bt->primitive_id)
			|| (bef_bt.pos.x!=bt->pos.x) || (bef_bt.pos.y!=bt->pos.y) || (bef_bt.pos.z!=bt->pos.z)
			|| (bef_bt.rot.x!=bt->rot.x) || (bef_bt.rot.y!=bt->rot.y) || (bef_bt.rot.z!=bt->rot.z)
			|| (bef_bt.scale.x!=bt->scale.x) || (bef_bt.scale.y!=bt->scale.y) || (bef_bt.scale.z!=bt->scale.z)))
			{
				mousedrag=1;
				if(texture_mode!=1) SetUndo();//for undo
			}
		}
		//
		if(keydata['K']){//KOPY
			cpy_clonedata = *bt;
			cpy_clonedata.clone_name = (char*)GlobalAlloc(GPTR,lstrlen(bt->clone_name)+1);
			lstrcpy(cpy_clonedata.clone_name,bt->clone_name);
			keydata['K']=0;
		}

		if(texture_mode==1){
			int mat = MaterialGetSelectedMaterial();
			if(mat!=-1){
				KMaterial* mtex = mdl.GetMaterial(mat);
				mtex->uv_trans = texdt.pos;
				mtex->uv_rot = texdt.rot;
				mtex->uv_scale = texdt.scale;
			}
		}
	}	
	lmoveX=0.0f;
	lmoveY=0.0f;	

	if(bLPressing==FALSE){//for undo
		mousedrag=0;
	}

	if(keydata['U']&&keydata[VK_SHIFT]){
		Redo();
		keydata['U']=0;
	}else if(keydata['U']){
		Undo();
		keydata['U']=0;
	}
	


	if(keydata[VK_F1]==1){
		CMatrix3 rm;
		rm.SetRotateY(PI/2);
		rot_state=rm;
	}else if(keydata[VK_F2]==1){
		CMatrix3 rm;
		rm.SetRotateX(-PI/2);
		rot_state=rm;
	}else if(keydata[VK_F3]==1){
		CMatrix3 rm;
		rm.SetRotateX(0);
		rot_state=rm;
	}
	
	if(keydata['P']==1){
		keydata['P']=0;
		view_mode=0;
	}else if(keydata['O']==1){
		keydata['O']=0;
		view_mode=1;
	}else if(keydata['W']==1){
		keydata['W']=0;
		wireframe_mode=(++wireframe_mode)&1;
	}

	if(keydata['H']){//H
		help_mode = (++help_mode)&1;
		keydata['H']=0;
	}

	Render();
}

void KeyEvent(UCHAR	key, bool isDown)
{
	if(isDown) keydata[key]=1;
	else	   keydata[key]=0;
	KeyInput();
	//switch(key)
	//{
	//case VK_ESCAPE:
	//	win.CExit();
	//	break;
	//}
}


void MouseEvent(long x,	long y,	int	btn, bool isDown)
{
	static int myPress,mxPress;
	switch(btn)
	{
		case LBUTTON:
		{
			if(isDown) SetCapture(hMainWnd);
			else	   ReleaseCapture();
			if(isDown){
				mxPress = x;
				myPress = y;
				RECT rt;
				GetClientRect(win.CGethWnd(),&rt);
				SelectDraw(mxPress,myPress,rt.right,rt.bottom);//select
				bLPressing = TRUE;
			}else{
				bLPressing = FALSE;
				keydata['X']=0;
				keydata['Y']=0;
				keydata['Z']=0;
			}
			KeyInput();
			Render();
			break;
		}
		case RBUTTON:
		{
			if(isDown) SetCapture(hMainWnd);
			else	   ReleaseCapture();
			if(isDown){
				mxPress = x;
				myPress = y;
				bRPressing = TRUE;
			}else{
				bRPressing = FALSE;
			}
			break;
		}
		case MBUTTON:
		{
			if(isDown){
				mxPress = x;
				myPress = y;
				bMPressing = TRUE;
			}else{
				bMPressing = FALSE;
			}
			break;
		}
		case WHEEL:
		{
			if(isDown){
				gZoom-=1;			
				if(gZoom<0) gZoom=0;
			}else{
				gZoom+=1;
			}
			Render();
			break;
		}
		case MOUSEMOVE:
		{
			POINT cp;
			GetCursorPos(&cp);
			if(GetActiveWindow()!=hMainWnd)
			{
				if(WindowFromPoint(cp)==hMainWnd){
					SetFocus(hMainWnd);
					Render();
				}
			}

			ScreenToClient(hMainWnd,&cp);
			int mx = cp.x;
			int my = cp.y;
			
			float spdratio = 1.0f;
			if(bShiftPressing){
				spdratio *= 0.1f;
			}
			if (bLPressing&&bRPressing){
				gZoom += spdratio *  float((mx - mxPress)*0.1f);
				gZoom += spdratio * float((my - myPress)*0.1f);
				if(gZoom<0) gZoom=0;
			}else if ((bMPressing)||((bLPressing)&&(keydata['M']))) {
				gTransX = + spdratio * float((mx - mxPress)*0.1f);
				gTransY = - spdratio * float((my - myPress)*0.1f);
			}else if (bLPressing) {
				lmoveY -= spdratio * float((mx - mxPress)/2.0f);
				lmoveX += spdratio * float((my - myPress)/2.0f);
				KeyInput();
			}else if (bRPressing) {	
				grotY -= spdratio * float((mx - mxPress)/200.0f);
				grotX += spdratio * float((my - myPress)/200.0f);				
			}
			mxPress = mx;
			myPress = my;
			Render();
			break;
		}
	}
}