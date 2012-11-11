/*
	render to texture 用のクラス
	coded by KIOKU	2003/9/17
	Ver 0.1

	2007.02.05 FBOコードに書き換え by c.r.v.
*/
#pragma once

#include "glImage.h"
#include "../kLib/kmath.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "../GL/glext.h"
#include "../GL/wglext.h"

class glScreen
{
	private:
		//GLuint scr_tex;
		int tex_wsize;
		int tex_hsize;
		int original_viewport[4];
		GLuint m_texture_id;
		GLuint m_frameBuffer;
		GLuint m_depthRenderBuffer;
	public:
        void __fastcall SetBlendState(unsigned long sfact,unsigned long dfact);
		glImage scr_image;
		void __fastcall DrawScreen(float fx, float fy, float alpha=1.0f);
		void __fastcall ViewOrtho(int width, int height);
		void __fastcall ViewPerspective();
		void __fastcall CreateTexture(int size);
		void __fastcall CreateTexture(int wsize, int hsize);
		void __fastcall RenderStart();
		void __fastcall RenderEnd(bool lumi=false, bool alpha=false);
		GLuint __fastcall GetTextureNum();
		void __fastcall DrawGlow(float x, float y, int draw_width, int draw_height,float tex_width, float tex_height, float alpha,int radius,int step,float clrbias);
		void __fastcall DrawBlur(float x, float y, int draw_width, int draw_height,float tex_width, float tex_height, float alpha,int radius, int step,float clrbias);
		//void __fastcall ReadFrameBuffer();
};