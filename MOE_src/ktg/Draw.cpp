#include "stdafx.h"
#include "common.h"
#include "Draw.h"

extern CWindowGL win;
extern UINT tex_name;
extern bool isAnimating;
UINT alphatex_name = 0;
extern KTextureEdit ktex;
extern bool isCapturing;
extern POINT* mousept;
extern long currentpoint;
extern long current_function;
UINT under_tex = 0;
float under_tex_alpha = 0.00f;
float alphatex_alpha = 1.00f;
float mastertex_alpha = 1.00f;

void InitOpenGL()
{
	//moeよりコピペ
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0.01f);
	glEnable(GL_COLOR_MATERIAL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	//glLightf(GL_LIGHT0,GL_DIFFUSE,0.6f);

	//テクスチャ
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	ktex.GenerateTextureIndirect(&alphatex_name, "Z,50,50,255,255,255,255,192,192,192,192;");
}

void GetScreenCoordinates(int x, int y, float* fx, float* fy){
	RECT rect;
	GetClientRect(hMainWnd, &rect);
	float window_w = (float)(rect.right - rect.left);
	float window_h = (float)(rect.bottom - rect.top);

	*fx = (x-(window_w/2))*(1/(window_w-window_w/2));
	*fy = ((window_h/2)-y)*(1/(window_h-window_h/2));	
}

void OnDraw(HDC hDC)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	RECT rect;
	GetClientRect(hMainWnd, &rect);

	glViewport(0, 0, rect.right, rect.bottom);
	//glLoadIdentity();
	//glOrtho(-rect.right, rect.right, -rect.bottom, rect.bottom, -0.1f,10.0f);
	//gluLookAt(0, 0, 200, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	//テクスチャ行列いじりテスト
//	glMatrixMode(GL_TEXTURE);
//	glLoadIdentity();
//	if(isAnimating){
//		static float vt = 0.0f;
//		static float r  = 0.0f;
//		vt+= 0.001f;
//		r += 0.01f;
//		if(vt>=1.0f) vt=0.0f;
//		if(r>=1.0f) r=0.0f;
//
////		glRotatef(r*360, 1, 1, 0);
//		glTranslatef(vt, 0, 0);
//	}

	glMatrixMode(GL_MODELVIEW);

	win.ClearScreen(1,1,1,1);
	glLoadIdentity();

	glPushMatrix();
		//半透明描画のための設定
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
	 //αテクスチャを描画する
		glBindTexture(GL_TEXTURE_2D, alphatex_name);
		glColor4f(1.0f,1.0f,1.0f,alphatex_alpha);
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

	  //生成したテクスチャを描画する
		glBindTexture(GL_TEXTURE_2D, tex_name);
		glColor4f(1.0f,1.0f,1.0f,mastertex_alpha);
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

	  //下絵
  		glBindTexture(GL_TEXTURE_2D, under_tex);
		glColor4f(1.0f,1.0f,1.0f, under_tex_alpha);
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

		glDepthMask(GL_TRUE); //半透明描画の設定を元に戻す
	glPopMatrix();

	int i;
	glDisable(GL_TEXTURE_2D);
	glColor4f(0,1,0,1);
	glBegin(GL_LINE_STRIP);
		POINT curpos;
		GetCursorPos(&curpos);
		ScreenToClient(hMainWnd, &curpos);
		float x = (float)curpos.x;
		float y = (float)curpos.y;

		float dx, dy;
		for(i=0; i<=currentpoint; i++){
			GetScreenCoordinates(mousept[i].x, mousept[i].y, &dx, &dy);
			glVertex2f(dx, dy);
		}

		GetScreenCoordinates((int)x, (int)y, &dx, &dy);
		glVertex2f(dx, dy);
		//glVertex2f(-1 , 1);
		//glVertex2f(dx , dy);
		//glVertex2f(1 , -1);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	win.RedrawScreen();
}