#include "stdafx.h"
#include "LayouterWindow.h"
#include "CloneWindow.h"
#include "SceneList.h"
#include "SceneObjectList.h"
#include "Sync.h"

#include "../klib/matrix.h"
#include "camera_model.h"
#include "kCtrl/TimelineView.h"
#include "startupcode.h"

#include "../gl/glext.h"

extern CWindowGL wingl;
extern KModelEdit mdl;
extern long nCurrentMode;
extern CTimelineView vTimeline;
extern CKSynthWindow vSynth;
extern CSoundWindow vSound;
extern CWindow win;


#include "dkText.h"
#include "../klib/glImage.h"

//マルチテクスチャ拡張用エントリポイント
PFNGLISRENDERBUFFEREXTPROC		glIsRenderbufferEXT;
PFNGLBINDRENDERBUFFEREXTPROC		glBindRenderbufferEXT;
PFNGLDELETERENDERBUFFERSEXTPROC	glDeleteRenderbuffersEXT;
PFNGLGENRENDERBUFFERSEXTPROC		glGenRenderbuffersEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC	glRenderbufferStorageEXT;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT;
PFNGLISFRAMEBUFFEREXTPROC		glIsFramebufferEXT;
PFNGLBINDFRAMEBUFFEREXTPROC		glBindFramebufferEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC	glDeleteFramebuffersEXT;
PFNGLGENFRAMEBUFFERSEXTPROC		glGenFramebuffersEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
PFNGLGENERATEMIPMAPEXTPROC		glGenerateMipmapEXT;

PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1fARB;
PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
PFNGLMULTITEXCOORD3FARBPROC		glMultiTexCoord3fARB;
PFNGLMULTITEXCOORD4FARBPROC		glMultiTexCoord4fARB;
PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;


dkText txt;
glImage font[128];


int keydata[256];
long view_mode	=0;
long help_mode	=0;
long camera_mode=0;

#define MODIFY_TRANSLATION		0
#define MODIFY_ROTATION			1
#define MODIFY_SCALING			2
#define MODIFY_SELECTION		3
#define MODIFY_ALPHA			4
long ModifyMode=MODIFY_TRANSLATION;
long GetModifyMode()
{
	return ModifyMode;
}

static BOOL bLPressing=FALSE,bRPressing=FALSE,bMPressing=FALSE;

static int select_axis[3] = {0,0,0};
KModelEdit camera;
CDemo demo(&camera);

#include <vector>
using namespace std;

vector<CDemo> undo_demo,redo_demo;

BOOL isPlaying = FALSE;
DWORD dwPlayStartTime = 0;
DWORD dwOriginalPlayStartTime = 0;
float fSceneCurrentTime = 0.0f;
float fScenePower = 1.0f;
float fTimelineTime = 0.0f;
long nStartScene = -1;//開始時のシーン
const int PLAYMODE_EACH = 1;
const int PLAYMODE_DEMO = 2;

float frame_alpha = 1.0f;

KRGBA clearcolor = {0.1f, 0.1f, 0.1f, 1.0f};


void UpdateFrameTransparency(float newTransparency){
	frame_alpha = newTransparency;
}

inline void DrawFrame(){
	//フレーム描画
	RECT rect;
	GetClientRect(hGLWnd, &rect);
	const long width = rect.right-rect.left;
	const long height = rect.bottom-rect.top;

	const long SCREEN_WIDTH = 800;//
	const long SCREEN_HEIGHT = 600;//
	glMatrixMode(GL_PROJECTION);
	glOrtho(-width/SCREEN_WIDTH, width/SCREEN_WIDTH, -height/SCREEN_HEIGHT, height/SCREEN_HEIGHT, 0.001, 1000.0);
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	//gluLookAt(0.0,0.0,-10.0, 0.0, 0.0,0.0, 0.0,1.0,0.0);
	glLoadIdentity();

	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	//glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
		const float frame_h = 1.0f - 0.125f*2.0f;
		glBegin(GL_QUADS);
			glColor4f(0.0f, 0.0f, 0.0f, frame_alpha);
			glVertex2f(-1, 1);
			glVertex2f(-1 , frame_h);
			glVertex2f(1 ,  frame_h);
			glVertex2f(1 , 1);
		glEnd();

		glBegin(GL_QUADS);
			glColor4f(0.0f, 0.0f, 0.0f, frame_alpha);
			glVertex2f(-1, -frame_h);
			glVertex2f(-1 ,-1);
			glVertex2f(1 , -1);
			glVertex2f(1 , -frame_h);
		glEnd();
	glDisable(GL_COLOR_MATERIAL);
	//glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	//	glVertex2f();
	glEnable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	//glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void StopPlaying(){
	vTimeline.SetSelectRateIndirect(fTimelineTime);
	UpdateTimeline();

	isPlaying=FALSE;
	dwPlayStartTime = 0;
	fScenePower = 1.0f;
	fTimelineTime = 0.0f;
	fSceneCurrentTime = 0.0f;
	nStartScene = -1;
	//settime=0.0f;
	StopMusic();
	
	vSynth.EnableControl(TRUE);
	vSound.EnableControl(TRUE);
	Render();
}

void StartPlaying(const int mode){
	isPlaying=mode;
	demo.ReadyDemo();

	dwOriginalPlayStartTime = dwPlayStartTime = timeGetTime();//後で使用する
	if(keydata[VK_SHIFT]){
		fScenePower=0.25f;
		keydata[VK_SHIFT] = 0;
	}else{
		fScenePower=1.0f;
	}

	vSynth.EnableControl(FALSE);
	vSound.EnableControl(FALSE);

	nStartScene = GetSelectedScene();
	if(mode == PLAYMODE_EACH){
		if(nStartScene!=-1){
			//fTimelineTime = 0.0f;
			fTimelineTime = vTimeline.GetSelectRate();
			//fTimelineTime = vTimeline.GetSelectRate()*demo.scene[nStartScene].fscene_time;
			//PlayMusic((nStartScene==0) ? 0+fTimelineTime : demo.scene[nStartScene-1].fscene_time+fTimelineTime);
			//PlayMusic((nStartScene==0) ? 0 : demo.scene[nStartScene-1].fscene_time);
			PlayMusic((nStartScene==0) ? 0 : demo.scene_endtime[nStartScene-1]);
		}else{
			StopPlaying();
		}
	}else if(mode == PLAYMODE_DEMO){
		if(nStartScene!=-1){
			if(demo.scene.size() != 0){
				PlayMusic((nStartScene==0) ? 0 : demo.scene_endtime[nStartScene-1]);
				//PlayMusic(0.0f);
			}else{
				StopPlaying();
			}
		}
	}
}

void ResetUndo()
{
	undo_demo.clear();
	redo_demo.clear();
}

void SetUndo()
{
	undo_demo.push_back(demo);
	redo_demo.clear();
}

void Undo()
{
	long udsize = (long)undo_demo.size()-1;
	if(udsize>=0){
		//primitiveのみ現在の値を引き継ぐ
		undo_demo[udsize].CopyPrimitive(demo);

		redo_demo.push_back(demo);
		demo = undo_demo[udsize];
		undo_demo.pop_back();
		RefreshAllView();
	}
}

void Redo()
{
	long rdsize = (long)redo_demo.size()-1;
	if(rdsize>=0){
		//primitiveのみ現在の値を引き継ぐ
		redo_demo[rdsize].CopyPrimitive(demo);

		undo_demo.push_back(demo);
		demo = redo_demo[rdsize];
		redo_demo.pop_back();
		RefreshAllView();
	}
}



long GetFindAnimNumber(float select_keyframe, CAnimation* ani)
{
	long tsel=-1;
	vector<float>::iterator tit,teit=ani->anim_time.end();
	for(tit=ani->anim_time.begin(); tit!=teit; tit++){
		if(select_keyframe==(*tit)){
			tsel = (long)(tit - ani->anim_time.begin());
		}
	}
	return tsel;
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
		//glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
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
	glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		gluPerspective( 60.0, 800.0f/600.0f, 1, 2000);
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	char buf[256];
	glRotatef(180,0.0f,1.0f,0);
	
	if(nCurrentMode==LAYOUTER){
		KClone* kcl = GetSelectedClone();
		if(kcl!=NULL){
			long sc = GetSelectedScene();
			long sco = GetSelectedSceneObject();
			long objn = (long)(kcl - demo.scene[sc].sceneobj[sco].model->GetCloneAllocPtr());
			long skt = GetFindAnimNumber(vTimeline.select_keyframe_rate, &(demo.scene[sc].sceneobj[sco].anim));
			if(skt!=-1){
				KCloneData clone_list = demo.scene[sc].sceneobj[sco].anim.anim[objn][skt];
				glPushMatrix();
				glTranslated(2,1.6,3);
				long m;
				for(m=0; m<256; m++){
					if(demo.obj[m]==demo.scene[sc].sceneobj[sco].model) break;
				}
				const char* nstr;
				if(m==256) nstr="camera";
				else nstr = demo.obj_name[m].c_str();
				sprintf(buf,"Layout Info:  [%s] - [%s]",nstr, kcl->clone_data.clone_name );
				DrawString(buf);
				glTranslated(0,-0.1,0);

				sprintf(buf,"Trans: (%.3f,%.3f,%.3f)",hftof(ftohf(clone_list.pos.x))
													,hftof(ftohf(clone_list.pos.y))
													,hftof(ftohf(clone_list.pos.z)));
				DrawString(buf);
				glTranslated(0,-0.1,0);
				sprintf(buf,"Rot  : (%.3f,%.3f,%.3f)",hftof(ftohf(clone_list.rot.x))
													,hftof(ftohf(clone_list.rot.y))
													,hftof(ftohf(clone_list.rot.z)));
				DrawString(buf);
				glTranslated(0,-0.1,0);
				sprintf(buf,"Scale: (%.3f,%.3f,%.3f)",hftof(ftohf(clone_list.scale.x))
												,hftof(ftohf(clone_list.scale.y))
												,hftof(ftohf(clone_list.scale.z)));
				DrawString(buf);
				glTranslated(0,-0.1,0);
				sprintf(buf,"Alpha: %.3f",hftof(ftohf(clone_list.alpha)));
				DrawString(buf);

				glTranslated(0,-0.1,0);
				 if(m==256){
					sprintf(buf,"Visible: %s",(demo.GetCameraPtr()->GetCloneAllocPtr()[CloneGetSelectedItem()].clone_data.visible==0)?"True":"False");
					DrawString(buf);
				}else if(demo.obj[m]!=NULL){
					sprintf(buf,"Visible: %s",(demo.obj[m]->GetCloneAllocPtr()[CloneGetSelectedItem()].clone_data.visible==0)?"True":"False");
					DrawString(buf);
				}
				glTranslated(0,-0.2,-0.2);
				sprintf(buf,"Edit Keyframe: %.3f",vTimeline.select_keyframe_rate);
				DrawString(buf);
				glTranslated(0,-0.1,0.0);
				sprintf(buf,"Now  Keyframe: %.3f",vTimeline.GetSelectRate());
				DrawString(buf);
				
				//glTranslated(0,-0.1,0.0);
				//float fEndTime = (this->scene[sn].fscene_time<=0.0f) ? 0.0000001f : this->scene[sn].fscene_time;//ゼロ除算回避
				//sprintf(buf,"Rate         : %.3f",(sn==0) ? vTimeline.GetSelectRate()*demo.scene[sc].fscene_time : (demotime_ms - demo.scene_endtime[sc-1]));
				//DrawString(buf);

				glPopMatrix();
			}
		}

		glTranslated(1.6,-1.6,3);
		long sc = GetSelectedScene();
		if(sc!=-1){
			sprintf(buf,"Scene[%d]:%s",sc,demo.scene[sc].scenename.c_str());
			DrawString(buf);
		}
		glTranslated(-2.0,0.0,0);
		if(ModifyMode==MODIFY_SELECTION)		DrawString("MODE:Selection  ");
		else if(ModifyMode==MODIFY_TRANSLATION) DrawString("MODE:Translation");
		else if(ModifyMode==MODIFY_ROTATION)    DrawString("MODE:Rotation   ");
		else if(ModifyMode==MODIFY_SCALING)     DrawString("MODE:Scaling    ");
		glTranslated(0,0.1,0);
		sprintf(buf,"Undo_num = %d",(long)undo_demo.size());
		DrawString(buf);
		glTranslated(0,0.1,0);
		sprintf(buf,"Redo_num = %d",(long)redo_demo.size());
		DrawString(buf);
	}else{//TIMELINER
		long sc = GetSelectedScene();
		float fSceneTime = 0.0f;
		if(sc!=-1){
			fSceneTime = demo.scene[sc].fscene_time/1000.0f;
		}
		glPushMatrix();
			glPushMatrix();
				glTranslated(2,1.6,3);

				glTranslated(0,-0.2,-0.2);
				sprintf(buf,"Edit Keyframe: %.3f",vTimeline.select_keyframe_rate);
				DrawString(buf);

				glTranslated(0,-0.1,0.0);
				sprintf(buf,"Edit     Time: %.3f(sec)",vTimeline.select_keyframe_rate * fSceneTime);
				DrawString(buf);

				glTranslated(0,-0.1,0.0);
				sprintf(buf,"Now  Keyframe: %.3f",vTimeline.GetSelectRate());
				DrawString(buf);

				glTranslated(0,-0.1,0.0);
				sprintf(buf,"Now      Time: %.3f(sec)",vTimeline.GetSelectRate() * fSceneTime);
				DrawString(buf);
			glPopMatrix();

			glTranslated(1.6,-1.6,3);
			sprintf(buf,"Scene[%d]:%s", sc, sc ==-1 ? "scene is not selected.":demo.scene[sc].scenename.c_str());
			DrawString(buf);

		glPopMatrix();
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
//
//void ModelerSelectClone(KClone* clone)
//{
//	long ccnt=0;
//	KClone* cptr = mdl.GetCloneAllocPtr();
//	while(&cptr[ccnt]!=clone){
//		ccnt++;
//		if(ccnt>=mdl.GetCloneAllocNum()) break;
//	}
//	select_clonepnum = ccnt;
//}
//
//KClone* GetSelectClone()
//{
//	KClone* cln = mdl.GetCloneAllocPtr();
//	return &cln[select_clonepnum];
//}


//-------------------------------------------

void InitLayouterWindow()
{
	//get entry points
	glMultiTexCoord1fARB = (PFNGLMULTITEXCOORD1FARBPROC)wglGetProcAddress("glMultiTexCoord1fARB");
	glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
	glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)wglGetProcAddress("glMultiTexCoord3fARB");
	glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)wglGetProcAddress("glMultiTexCoord4fARB");
	glActiveTextureARB	 = (PFNGLACTIVETEXTUREARBPROC)	wglGetProcAddress("glActiveTextureARB");

	glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)wglGetProcAddress("glIsRenderbufferEXT");
	glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
	glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
	glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
	glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
	glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)wglGetProcAddress("glGetRenderbufferParameterivEXT");
	glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)wglGetProcAddress("glIsFramebufferEXT");
	glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
	glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
	glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
	glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
	glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)wglGetProcAddress("glFramebufferTexture1DEXT");
	glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
	glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)wglGetProcAddress("glFramebufferTexture3DEXT");
	glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
	glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
	glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)wglGetProcAddress("glGenerateMipmapEXT");


	camera.Load(camera_model);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0.01f);
	glEnable(GL_COLOR_MATERIAL);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CW);
	glLightf(GL_LIGHT0,GL_DIFFUSE,0.6f);
	
	rot_state.SetRotateX(0.0f);

	InitFont();
	demo.CreateDOFTexture();
	demo.ReadyDemo();

	demo.InitSoundSystem(wingl.CGethWnd());
	//demo.Load("test.kdf");
	//demo.ReadyDemo();
}

void DrawMesh(float size,float trans_x,float trans_y,float trans_z)
{
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);
	glLineWidth(1);

	glTranslatef(trans_x,trans_y,trans_z);
	//glColor3f(0.5,0.5,0.5);
	float meshclr[] = {0.5f, 0.5f, 0.5f, 1.0f};
	glColor4fv(meshclr);
	glBegin(GL_LINES);
		float i;
		for(i=-size; i<=size; i+=size/10.0f){
			glVertex3f(-size,0,i); glVertex3f(size,0,i);
			glVertex3f(i,0,-size); glVertex3f(i,0,size);
		}
	glEnd();

	glLineWidth(1);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
}


void DrawAxis(float size,float trans_x,float trans_y,float trans_z, float lwidth=8.0)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glLineWidth(lwidth);

	glTranslatef(trans_x,trans_y,trans_z);
	glLoadName(1);
	glBegin(GL_LINES);
		float axclr[] = {1.0f, 0.0f, 0.0f, 0.8f};
		glColor4fv(axclr);
		glVertex3f(-size,0,0); glVertex3f(size,0,0);
		glVertex3f(size,0,0);glVertex3f(size*0.9f,size*0.05f,0);
	glEnd();
	glLoadName(2);
	glBegin(GL_LINES);
		float ayclr[] = {0.0f, 1.0f, 0.0f, 0.8f};
		glColor4fv(ayclr);
		glVertex3f(0,-size,0); glVertex3f(0,size,0);
		glVertex3f(0,size,0);glVertex3f(size*0.05f,size*0.9f,size*0.05f);
	glEnd();
	glLoadName(3);
	glBegin(GL_LINES);
		float azclr[] = {0.0f, 0.0f, 1.0f, 0.8f};
		glColor4fv(azclr);
		glVertex3f(0,0,-size); glVertex3f(0,0,size);
		glVertex3f(0,0,size);glVertex3f(0,size*0.05f,size*0.9f);
	glEnd();
	
	glLineWidth(1);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);
}


void DrawRotAxis(float size,float rot_x,float rot_y,float rot_z)
{
	long i;
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glLineWidth(8);

	glRotatef(rot_x,1,0,0);
		float xclr[] = {1,0,0,0.8f};
		glColor4fv(xclr);
		glLoadName(1);
		glBegin(GL_LINE_LOOP);
		for(i=0; i<360; i++){
			//glNormal3f(0,size*cosf((i-90.0f)/180.0f*PI),size*sinf((i-90.0f)/180.0f*PI));
			glVertex3f(0,size*cosf((i-90.0f)/180.0f*PI),size*sinf((i-90.0f)/180.0f*PI));
		}
		glEnd();
	glRotatef(rot_y,0,1,0);
		float yclr[] = {0,1,0,0.8f};
		glColor4fv(yclr);
		glLoadName(2);
		glBegin(GL_LINE_LOOP);
		for(i=0; i<360; i++){
			//glNormal3f(size*cosf((i-90.0f)/180.0f*PI),0,size*sinf((i-90.0f)/180.0f*PI));
			glVertex3f(size*cosf((i-90.0f)/180.0f*PI),0,size*sinf((i-90.0f)/180.0f*PI));
		}
		glEnd();
	glRotatef(rot_z,0,0,1);
		float zclr[] = {0,0,1,0.8f};
		glColor4fv(zclr);
		glLoadName(3);
		glBegin(GL_LINE_LOOP);
		for(i=0; i<360; i++){
			//glNormal3f(size*cosf((i-90.0f)/180.0f*PI),size*sinf((i-90.0f)/180.0f*PI),0);
			glVertex3f(size*cosf((i-90.0f)/180.0f*PI),size*sinf((i-90.0f)/180.0f*PI),0);
		}
		glEnd();

	glLineWidth(1);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);
}


void DrawScaleAxis(float size,float lwidth=8.0)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glLineWidth(lwidth);

	//glTranslatef(0.0f, 0.0f, 0.0f);
	glLoadName(1);
	glBegin(GL_LINES);
		float axclr[] = {1.0f, 0.0f, 0.0f, 0.8f};
		glColor4fv(axclr);
		glVertex3f(0,0,0); glVertex3f(size,0,0);
		glVertex3f(size,0,0);glVertex3f(size*0.9f,size*0.05f,0);
	glEnd();
	glLoadName(2);
	glBegin(GL_LINES);
		float ayclr[] = {0.0f, 1.0f, 0.0f, 0.8f};
		glColor4fv(ayclr);
		glVertex3f(0,0,0); glVertex3f(0,size,0);
		glVertex3f(0,size,0);glVertex3f(size*0.05f,size*0.9f,size*0.05f);
	glEnd();
	glLoadName(3);
	glBegin(GL_LINES);
		float azclr[] = {0.0f, 0.0f, 1.0f, 0.8f};
		glColor4fv(azclr);	
		glVertex3f(0,0,0); glVertex3f(0,0,size);
		glVertex3f(0,0,size);glVertex3f(0,size*0.05f,size*0.9f);
	glEnd();

	glLineWidth(1);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);
}
void DrawPointer()
{
	if(camera_mode!=0){
		return;//カメラモード編集時は軸を描画しない
	}
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glPushMatrix();
		KClone* kcl = GetSelectedClone();
		if(kcl!=NULL){
			long sc = GetSelectedScene();
			long sco = GetSelectedSceneObject();
			long objn = (long)(kcl - demo.scene[sc].sceneobj[sco].model->GetCloneAllocPtr());
			long skt = GetFindAnimNumber(vTimeline.select_keyframe_rate, &(demo.scene[sc].sceneobj[sco].anim));
			if(skt!=-1){
				KCloneData* cd = &(demo.scene[sc].sceneobj[sco].anim.anim[objn][skt]);
				
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
		}
	glPopMatrix();

	glEnable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glEnable(GL_LIGHTING);
}

void KeyInput();
void DrawObject(int select_mode=0, int wire_mode=0, int camera_mode=0)
{
	if(nCurrentMode==LAYOUTER){
		glLoadIdentity();
		glPushMatrix();			
			if(camera_mode==0){
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
				if(select_mode==0){//SelectDraw以外から呼ばれたとき（通常は０が入っている）
					DrawMesh(10,0,0,0);
					DrawScaleAxis(100,3);//world
				}
			}
			long sc = GetSelectedScene();
			if(sc!=-1){
				glPushMatrix();
					demo.RenderScene(sc, vTimeline.GetSelectRate(), camera_mode);
					//demo.scene[sc].RenderScene(vTimeline.GetSelectRate(),camera_mode);
				glPopMatrix();
				glPushMatrix();
					KClone* sel_cln = GetSelectedClone();
					if(sel_cln!=NULL){
						long sco = GetSelectedSceneObject();
						demo.scene[sc].sceneobj[sco].model->DrawFunc(sel_cln,DrawPointer);
					}
				glPopMatrix();
			}
		glPopMatrix();	
	}else{
		glLoadIdentity();
		glPushMatrix();			
			long sc = GetSelectedScene();
			if(sc!=-1){
				glPushMatrix();
					demo.RenderScene(sc, vTimeline.GetSelectRate(), 1);
					//demo.scene[sc].RenderScene(vTimeline.GetSelectRate(),camera_mode);
				glPopMatrix();
			}
		glPopMatrix();	
	}
}

void Render()
{
	if(isPlaying){
	}else{
        wingl.ClearScreen(clearcolor.r, clearcolor.g, clearcolor.b, 1.0f);
		//wingl.ClearScreen(0.1f,0.1f,0.1f,1.0f);
			glLoadIdentity();
			if(demo.scene.size()>0)
			{
				DrawObject(0, wireframe_mode, camera_mode);
				if(camera_mode==1) DrawFrame();
				DispInfo(help_mode);
			}
		wingl.RedrawScreen();
	}
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
		gluPickMatrix(mx+6, window_height - my, 12, 10, vp);//mouse point->window pos(upsidedown)
		if(view_mode==0){
			gluPerspective( 60.0, (GLdouble)window_width/(GLdouble)window_height, 1, 2000);
		}else if(view_mode==1){
			GLdouble aspect = (GLdouble)window_width/(GLdouble)window_height;
			glOrtho(-gZoom/15.0f*10.0*aspect,gZoom/15.0f*10.0*aspect,-gZoom/15.0f*10.0,gZoom/15.0f*10.0f,-100,2000);
		}
	glMatrixMode(GL_MODELVIEW);
	//
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
		//}else{
		//	long selectobj = mobj-10;//0-9は軸
		//	if((selectobj>=0)&&(selectobj<(long)mdl.GetCloneAllocNum())){
		//		select_clonepnum = selectobj;
		//		KClone* cln = mdl.GetCloneAllocPtr();
		//		CloneSelectClone(&cln[select_clonepnum]);
		//	}
		}	
	}
}

void KeyInput()
{
	if(!isPlaying){
		if(nCurrentMode==LAYOUTER){//deny TIMELINER
			bLPressing   = (GetAsyncKeyState(VK_LBUTTON)&0x8000)==0x8000 ? 1:0;
			bRPressing   = (GetAsyncKeyState(VK_RBUTTON)&0x8000)==0x8000 ? 1:0;
			if(bLPressing==0){
				select_axis[0] = select_axis[1] = select_axis[2] = 0;
			}
			if(select_axis[0]==1) keydata['X'] = 1;
			else keydata['X'] = (GetAsyncKeyState('X')&0x8000)==0x8000 ? 1:0;
			if(select_axis[1]==1) keydata['Y'] = 1;
			else keydata['Y'] = (GetAsyncKeyState('Y')&0x8000)==0x8000 ? 1:0;
			if(select_axis[2]==1) keydata['Z'] = 1;
			else keydata['Z'] = (GetAsyncKeyState('Z')&0x8000)==0x8000 ? 1:0;


			static KCloneData cpy_clonedata;
			static long mousedrag=0;

			if(keydata['V']==1) ModifyMode=MODIFY_TRANSLATION;
			else if(keydata['C']==1) ModifyMode=MODIFY_ROTATION;
			else if(keydata['Q']==1) ModifyMode=MODIFY_SCALING;
			else if(keydata['A']==1) ModifyMode=MODIFY_ALPHA;
			//else if(keydata['S']==1) ModifyMode=MODIFY_SELECTION;

			float mv_rate = keydata[VK_SHIFT]==0? 1 : 0.1f;

			float rate = 0.05f*gZoom/15.0f;
			CVector mv = rot_state*CVector(-lmoveY*rate,-lmoveX*rate,0);
			KClone* kcl = GetSelectedClone();
			if(kcl!=NULL){
				long sc = GetSelectedScene();
				long sco = GetSelectedSceneObject();
				long objn = (long)(kcl - demo.scene[sc].sceneobj[sco].model->GetCloneAllocPtr());
				long skt = GetFindAnimNumber(vTimeline.select_keyframe_rate, &(demo.scene[sc].sceneobj[sco].anim));
				if(skt!=-1){
					KCloneData* bt = &(demo.scene[sc].sceneobj[sco].anim.anim[objn][skt]);
					if(bt->lock==0){//LOCK
						KCloneData bef_bt=*bt;
						if(ModifyMode==MODIFY_ROTATION){//ROTATE
							if(keydata['Y']==1){
								bt->rot.y += mv.x/0.05f*mv_rate;
							}else if(keydata['X']==1){
								bt->rot.x += -mv.y/0.05f*mv_rate;
							}else if(keydata['Z']==1){
								bt->rot.z += (mv.x+mv.y)/0.05f*mv_rate;
							}else if(keydata['R']==1){
								bt->rot = CVector(0,0,0);
							}
						}else if(ModifyMode==MODIFY_SCALING){//SCALE
							if((keydata['X']==1)&&(keydata['Y']==1)&&(keydata['Z']==1)){
								CVector ma(mv.y+mv.x,mv.y+mv.x,mv.y+mv.x);
								bt->scale += ma;
							}else if(keydata['Y']==1){
								bt->scale.y += (mv.y*2*mv_rate);
							}else if(keydata['X']==1){
								bt->scale.x += (mv.x*2*mv_rate);
							}else if(keydata['Z']==1){
								bt->scale.z += (mv.z*2*mv_rate);
							}else if(keydata['R']==1){
								bt->scale = CVector(1,1,1);
							}else{
								bt->scale += (mv*2);
							}
						}else if(ModifyMode==MODIFY_TRANSLATION){//Translation
							if(keydata['Y']==1){
								bt->pos.y += mv.y*mv_rate;
							}else if(keydata['X']==1){
								bt->pos.x +=mv.x*mv_rate;
							}else if(keydata['Z']==1){
								bt->pos.z += mv.z*mv_rate;
							}else if(keydata['R']==1){
								bt->pos = CVector(0,0,0);
							}else{
								bt->pos += mv;
							}
						}else if(ModifyMode==MODIFY_ALPHA){//TRANSPARENT
							if(keydata['R']==1){
								bt->alpha = 1.0f;
							}else{
								bt->alpha += (mv.x+mv.y)*mv_rate*0.1f;
							}
							//if(bt->alpha>1.0f) bt->alpha = 1.0f;
							if(bt->alpha<0.0f) bt->alpha = 0.0f;
						}

						if(keydata['L']){//LOAD
							SetUndo();
							if(ModifyMode==MODIFY_TRANSLATION) bt->pos = cpy_clonedata.pos;
							else if(ModifyMode==MODIFY_ROTATION) bt->rot = cpy_clonedata.rot;
							else if(ModifyMode==MODIFY_SCALING) bt->scale = cpy_clonedata.scale;
							else if(ModifyMode==MODIFY_ALPHA) bt->alpha = cpy_clonedata.alpha;
							else *bt = cpy_clonedata ;
							keydata['L']=0;
							
						}

						if((mousedrag==0)&&(bLPressing==TRUE)
						&& ((bef_bt.primitive_id!=bt->primitive_id)
						|| (bef_bt.pos.x!=bt->pos.x) || (bef_bt.pos.y!=bt->pos.y) || (bef_bt.pos.z!=bt->pos.z)
						|| (bef_bt.rot.x!=bt->rot.x) || (bef_bt.rot.y!=bt->rot.y) || (bef_bt.rot.z!=bt->rot.z)
						|| (bef_bt.scale.x!=bt->scale.x) || (bef_bt.scale.y!=bt->scale.y) || (bef_bt.scale.z!=bt->scale.z)
						|| (bef_bt.alpha!=bt->alpha)))
						{
							mousedrag=1;
							SetUndo();//for undo
						}
					}
				
					//
					if(keydata['K']){//KOPY
						//if((GetAsyncKeyState(VK_SHIFT)&0x8000)==0x8000){
						//	rate = vTimeline.GetSelectRate();
						//}else{
							cpy_clonedata = *bt;
						//}
						keydata['K']=0;
					}
				}
			}	
			lmoveX=0.0f;
			lmoveY=0.0f;	

			if(bLPressing==FALSE){//for undo
				mousedrag=0;
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
			}else if(keydata[' ']==1){
				keydata[' ']=0;
				camera_mode=(++camera_mode)&1;
			}

			if((keydata['U'])&&(keydata[VK_SHIFT])){
				Redo();
				keydata['U']=0;
			}else if(keydata['U']){
				Undo();
				keydata['U']=0;
			}
			Render();
		}
	}else if(isPlaying){
		//if(keydata[VK_CONTROL]){
		//	DWORD dwCurrentTime = timeGetTime();

		//	//早送り・巻き戻し
		//	if(keydata[VK_SHIFT]){
		//		DWORD dwOldTime = dwPlayStartTime;
		//		dwPlayStartTime += 1000;//dwPlayStartTimeを書き換えることで描画ポイントを飛ばす
		//		if(dwCurrentTime < dwPlayStartTime){
		//			dwPlayStartTime = dwOldTime;
		//			//dwPlayStartTime = dwOriginalPlayStartTime;
		//		}
		//	}else{
//                 dwPlayStartTime -= 1000;
		//	}

		//	float offset_scene_time = (nStartScene==0) ? 0 : demo.scene_endtime[nStartScene-1];
		//	PlayMusic((dwCurrentTime - dwPlayStartTime) + offset_scene_time);
		//}
		if(keydata[VK_RIGHT] || keydata[VK_LEFT]){
			DWORD dwCurrentTime = timeGetTime();

			//早送り・巻き戻し
			if(keydata[VK_LEFT]){
				DWORD dwOldTime = dwPlayStartTime;
				dwPlayStartTime += 1000;//dwPlayStartTimeを書き換えることで描画ポイントを飛ばす
				if(dwCurrentTime < dwPlayStartTime){
					dwPlayStartTime = dwOldTime;
					//dwPlayStartTime = dwOriginalPlayStartTime;
				}
			}else if(keydata[VK_RIGHT]){
				dwPlayStartTime -= 1000;
			}

			float offset_scene_time = (nStartScene==0) ? 0 : demo.scene_endtime[nStartScene-1];
			PlayMusic((dwCurrentTime - dwPlayStartTime) + offset_scene_time);
		}
	}
	if(keydata[VK_F5]==1){//scene play
		StartPlaying(PLAYMODE_EACH);
		keydata[VK_F5]=0;
	}
	if(keydata[VK_F6]==1){
		StartPlaying(PLAYMODE_DEMO);
		keydata[VK_F6] = 0;
	}
	if(keydata[VK_F4]){
		StopPlaying();
		keydata[VK_F4] = 0;
	}
}

void OnIdle(){
	if(!isPlaying) Sleep(1);
	else{
		//long nStartScene = GetSelectedScene();
		if(nStartScene==-1) return;
		else{
			float music_end = demo.ksl->GetSoundTime()*1000.0f;//sec->msec
			DWORD dwCurrentTime = timeGetTime();

			if(isPlaying==PLAYMODE_EACH){//1シーンごと
				static float current_rate = 0.0f;
				//float fCurrentTimelineRate = vTimeline.GetSelectRate();
				//if(current_rate<
				//fCurrentTimelineRate = fCurrentTimelineRate * demo.scene[nStartScene].fscene_time;

				fSceneCurrentTime = 0.0f;
                fSceneCurrentTime += (dwCurrentTime - dwPlayStartTime)*fScenePower;
				fSceneCurrentTime += (nStartScene==0) ? 0.0f : demo.scene_endtime[nStartScene-1];
				//fSceneCurrentTime = (dwCurrentTime+fTimelineTime - dwPlayStartTime)*fScenePower;
				//fSceneCurrentTime = (dwCurrentTime - dwPlayStartTime) * fScenePower;

				float s_end = demo.scene_endtime[nStartScene];
				//float s_end = (nStartScene==0) ? demo.scene_endtime[0] : (demo.scene_endtime[nStartScene] - demo.scene_endtime[nStartScene-1]);
				//float s_end = (nStartScene==0) ? demo.scene[0].fscene_time : (demo.scene[nStartScene].fscene_time - demo.scene[nStartScene-1].fscene_time);
				//float s_end = (sn==0) ? demo.scene_endtime[0] : (demo.scene_endtime[sn] - demo.scene_endtime[sn-1]);
				if(s_end == 0.000f || fSceneCurrentTime>s_end){// || fSceneCurrentTime > music_end){
					StopPlaying(); return;
				}

				//dwCurrentTime -= dwPlayStartTime;//単純に再生デバッグ用
				wingl.ClearScreen(0,0,0,1);
					glLoadIdentity();
					demo.RenderDemo(fSceneCurrentTime, &current_rate);
					DrawFrame();
					DispInfo(0);
				wingl.RedrawScreen();

				vTimeline.SetSelectRateIndirect(current_rate);
				UpdateTimeline();
				//vTimeline.ForceDraw();
			}else{//PLAYMODE_DEMO
				//float fSceneCurrentTime = (dwCurrentTime - dwPlayStartTime)*fScenePower;
				//BOOL isEnd = FALSE;
				//wingl.ClearScreen(0,0,0,1);
				//	glLoadIdentity();
				//	float current_rate = 0.0f;
				//	isEnd = demo.RenderDemo(fSceneCurrentTime, &current_rate);
				//wingl.RedrawScreen();

				//vTimeline.SetSelectRateIndirect(current_rate);
				//UpdateTimeline();

				//if(isEnd){// || fSceneCurrentTime > music_end){
				//	StopPlaying(); return;
				//}
				float fSceneCurrentTime = 0.0f;
                fSceneCurrentTime += (dwCurrentTime - dwPlayStartTime)*fScenePower;
				fSceneCurrentTime += (nStartScene==0) ? 0.0f : demo.scene_endtime[nStartScene-1];
				BOOL isEnd = FALSE;
				wingl.ClearScreen(0,0,0,1);
					glLoadIdentity();
					float current_rate = 0.0f;
					isEnd = demo.RenderDemo(fSceneCurrentTime, &current_rate);
					DrawFrame();
					//DispInfo(0);
				wingl.RedrawScreen();

				vTimeline.SetSelectRateIndirect(current_rate);
				UpdateTimeline();

				if(isEnd){// || fSceneCurrentTime > music_end){
					StopPlaying(); return;
				}
			}
		}
	}
}

void LayouterKeyEvent(UCHAR	key, bool isDown)
{
    if(key==VK_F7 && isDown){
		if(nCurrentMode!=LAYOUTER){
			SendMessage(hMainWnd, WM_COMMAND, ID_MODE_LAYOUTER, 0);
		}
	}else if(key==VK_F8 && isDown){
		if(nCurrentMode!=TIMELINER){
			SendMessage(hMainWnd, WM_COMMAND, ID_MODE_TIMELINER, 0);
		}
	}else if(key==VK_F1 && isDown){
		clearcolor.r = clearcolor.g = clearcolor.b = 0.1f;
	}else if(key==VK_F2 && isDown){
		clearcolor.r = clearcolor.g = clearcolor.b = 0.9f;		
	}

	if(isDown) keydata[key]=1;
	else	   keydata[key]=0;
	KeyInput();
}


void LayouterMouseEvent(long x,	long y,	int	btn, bool isDown)
{
	if(nCurrentMode==TIMELINER){
		Render();
		return;//タイムライン編集中はマウスなし
	}

	if(!isPlaying){
		static int myPress,mxPress;
		switch(btn)
		{
		case LBUTTON:
		{
			if(isDown) SetCapture(hGLWnd);
			else	   ReleaseCapture();
			if(isDown){
				mxPress = x;
				myPress = y;
				RECT rt;
				GetClientRect(wingl.CGethWnd(),&rt);
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
			if(isDown) SetCapture(hGLWnd);
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
				if(GetActiveWindow()!=hGLWnd)
				{
					if(WindowFromPoint(cp)==hGLWnd){
						SetFocus(hGLWnd);
						Render();
					}
				}

				ScreenToClient(hGLWnd,&cp);
				int mx = cp.x;
				int my = cp.y;
				if (bLPressing&&bRPressing){
					gZoom += float((mx - mxPress)*0.1f);
					gZoom += float((my - myPress)*0.1f);
					if(gZoom<0) gZoom=0;
				}else if ((bMPressing)||((bLPressing)&&(keydata['M']))) {
					gTransX = +float((mx - mxPress)*0.1f);
					gTransY = -float((my - myPress)*0.1f);
				}else if (bLPressing) {
					lmoveY -= float((mx - mxPress)/2.0f);
					lmoveX += float((my - myPress)/2.0f);
					KeyInput();
				}else if (bRPressing) {	
					grotY -= float((mx - mxPress)/200.0f);
					grotX += float((my - myPress)/200.0f);				
				}
				mxPress = mx;
				myPress = my;
				Render();
				InvalidateRect( vTimeline.GetHWnd(), NULL, false);
				break;
			}
		}
	}
}