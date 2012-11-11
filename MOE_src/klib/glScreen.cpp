#include "stdafx.h"
#include "glScreen.h"

extern PFNGLISRENDERBUFFEREXTPROC		glIsRenderbufferEXT;
extern PFNGLBINDRENDERBUFFEREXTPROC		glBindRenderbufferEXT;
extern PFNGLDELETERENDERBUFFERSEXTPROC	glDeleteRenderbuffersEXT;
extern PFNGLGENRENDERBUFFERSEXTPROC		glGenRenderbuffersEXT;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC	glRenderbufferStorageEXT;
extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT;
extern PFNGLISFRAMEBUFFEREXTPROC		glIsFramebufferEXT;
extern PFNGLBINDFRAMEBUFFEREXTPROC		glBindFramebufferEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC	glDeleteFramebuffersEXT;
extern PFNGLGENFRAMEBUFFERSEXTPROC		glGenFramebuffersEXT;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
extern PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
extern PFNGLGENERATEMIPMAPEXTPROC		glGenerateMipmapEXT;


void __fastcall glScreen::ViewOrtho(int width, int height)												// Set Up An Ortho View
{
	glMatrixMode(GL_PROJECTION);								// Select Projection
	glPushMatrix();												// Push The Matrix
	glLoadIdentity();											// Reset The Matrix
	glOrtho( 0, width, height, 0, 0.0f, 100.0f );				// Select Ortho Mode (640x480)
	glMatrixMode(GL_MODELVIEW);									// Select Modelview Matrix
	glPushMatrix();												// Push The Matrix
	glLoadIdentity();											// Reset The Matrix
}

void __fastcall glScreen::ViewPerspective()											// Set Up A Perspective View
{
	glMatrixMode( GL_PROJECTION );								// Select Projection
	glPopMatrix();												// Pop The Matrix
	glMatrixMode( GL_MODELVIEW );								// Select Modelview
	glPopMatrix();
}


void __fastcall glScreen::CreateTexture(int size)
{
	tex_wsize=size;
	tex_hsize=size;
	//scr_image.Create(size,size,255,255,255);
	//scr_image.Convert();

#ifdef FRIENDLY_WITH_OLDIES_20
	if( glBindRenderbufferEXT ){
#endif
		glGenFramebuffersEXT( 1, &m_frameBuffer );
		glGenRenderbuffersEXT( 1, &m_depthRenderBuffer );
		glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, m_depthRenderBuffer );
#ifdef FRIENDLY_WITH_OLDIES_20
	}else{

	}
#endif


#ifdef FRIENDLY_WITH_OLDIES_20
	if( glBindRenderbufferEXT ){
#endif
		glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, tex_wsize, tex_hsize );
		GLenum status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
#ifdef FRIENDLY_WITH_OLDIES_20
	}else{

	}
#endif

	glGenTextures( 1, &m_texture_id );
	glBindTexture( GL_TEXTURE_2D, m_texture_id );

#ifdef FRIENDLY_WITH_OLDIES_20
	if( glBindRenderbufferEXT ){
#endif
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex_wsize, tex_hsize, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
#ifdef FRIENDLY_WITH_OLDIES_20
	}else{
		unsigned char* pdata = (unsigned char*)GlobalAlloc(GPTR, sizeof(unsigned char) * 3 * tex_wsize * tex_hsize);
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex_wsize, tex_hsize, 0, GL_RGB, GL_UNSIGNED_BYTE, pdata );
		GlobalFree(pdata);
	}
#endif

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
}


void __fastcall glScreen::CreateTexture(int wsize, int hsize)
{
	tex_wsize=wsize;
	tex_hsize=hsize;
	//scr_image.Create(size,size,255,255,255);
	//scr_image.Convert();

#ifdef FRIENDLY_WITH_OLDIES_20
	if( glBindRenderbufferEXT ){
#endif
		glGenFramebuffersEXT( 1, &m_frameBuffer );
		glGenRenderbuffersEXT( 1, &m_depthRenderBuffer );
		glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, m_depthRenderBuffer );
#ifdef FRIENDLY_WITH_OLDIES_20
	}else{

	}
#endif


#ifdef FRIENDLY_WITH_OLDIES_20
	if( glBindRenderbufferEXT ){
#endif
		glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, tex_wsize, tex_hsize );
		GLenum status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
#ifdef FRIENDLY_WITH_OLDIES_20
	}else{

	}
#endif

	glGenTextures( 1, &m_texture_id );
	glBindTexture( GL_TEXTURE_2D, m_texture_id );

#ifdef FRIENDLY_WITH_OLDIES_20
	if( glBindRenderbufferEXT ){
#endif
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex_wsize, tex_hsize, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
#ifdef FRIENDLY_WITH_OLDIES_20
	}else{
		unsigned char* pdata = (unsigned char*)GlobalAlloc(GPTR, sizeof(unsigned char) * 3 * tex_wsize * tex_hsize);
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex_wsize, tex_hsize, 0, GL_RGB, GL_UNSIGNED_BYTE, pdata );
		GlobalFree(pdata);
	}
#endif

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
}

//void __fastcall glScreen::Deinitialize (void)
//{
 //   glDeleteTextures( 1, &m_texture_id );

	//glDeleteFramebuffersEXT( 1, &m_frameBuffer );
	//glDeleteRenderbuffersEXT( 1, &m_depthRenderBuffer );
//glDeleteTextures(1,&scr_image);
//}

void __fastcall glScreen::RenderStart()
{
	glGetIntegerv(GL_VIEWPORT, original_viewport);

#ifdef FRIENDLY_WITH_OLDIES_20
	if( glBindRenderbufferEXT ){
#endif
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_frameBuffer );
		//glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, g_depthRenderBuffer );
		glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture_id, 0 );
		glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_depthRenderBuffer );
#ifdef FRIENDLY_WITH_OLDIES_20
	}else{
	}
#endif
	glViewport(0,0,tex_wsize,tex_hsize);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void __fastcall glScreen::RenderEnd(bool lumi, bool alpha)
{
//	glEnable(GL_TEXTURE_2D);
//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//	glBindTexture(GL_TEXTURE_2D,scr_image.GetTextureNum());//scr_tex);
//
////   if( lumi )       glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, tex_size, tex_size, 0);
// //  	else if( alpha ) glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0,tex_size,tex_size,0);
//	//else			 
//
//	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tex_size, tex_size);
#ifdef FRIENDLY_WITH_OLDIES_20
	if( glBindRenderbufferEXT ){
#endif
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	//glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, 0 );
	//glFlush();
#ifdef FRIENDLY_WITH_OLDIES_20
	}else{
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, m_texture_id);//scr_tex);
	
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tex_wsize, tex_hsize);
	}
#endif

	glViewport(original_viewport[0],original_viewport[1],original_viewport[2],original_viewport[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//void __fastcall glScreen::ReadFrameBuffer(){
//	glEnable(GL_TEXTURE_2D);
//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//	glBindTexture(GL_TEXTURE_2D,scr_image.GetTextureNum());
//	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0,tex_size,tex_size,0);
//}

GLuint __fastcall glScreen::GetTextureNum()
{
	return m_texture_id;//scr_image.GetTextureNum();
}

void __fastcall glScreen::SetBlendState(unsigned long sfact,unsigned long dfact)
{
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	glBlendFunc(sfact,dfact);
    if((sfact==GL_ZERO)&&(dfact==GL_ZERO)) glDisable(GL_BLEND);
    else                                   glEnable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
}


void __fastcall glScreen::DrawGlow(float x, float y, int draw_width, int draw_height,float tex_width, float tex_height, float alpha,int radius,int step,float clrbias)
{
	if(step==0) step=1;

	ViewOrtho(original_viewport[2],original_viewport[3]);

	float th = tex_height;
	float tw = tex_width;
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D,m_texture_id/*scr_image.GetTextureNum()*/);
	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLE_STRIP);
	float i,j;
    float dx=0,dy=0;
    radius = (int)(radius * 0.25f);
	if(radius<0) radius=0;
	for(i=(float)(-radius); i<=(float)radius; i+=step){
        dx+=x;
        dy+=y;
       	for(j=(float)(-radius); j<=(float)radius; j+=step){
            if((i!=0)&&(j!=0)) alpha = 1.0f/(float)sqr(i*i+j*j)*10.0f;
            else alpha = 1.0f;
   			glColor4f(clrbias, clrbias, clrbias, alpha);
    		glTexCoord2f(tw  ,0.0f);  	glVertex2f(dx+draw_width	, dy+draw_height);
  	    	glTexCoord2f(tw  ,th  );	glVertex2f(dx+draw_width	, dy-j			 );
    	    glTexCoord2f(0.0f,0.0f);    glVertex2f(dx-i				, dy+draw_height);
   		    glTexCoord2f(0.0f,th  );	glVertex2f(dx-i				, dy-j			 );
		}
	}
	glEnd();
    
	ViewPerspective();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_CULL_FACE);
}

void __fastcall glScreen::DrawScreen(float fx, float fy, float alpha)
{
	ViewOrtho(original_viewport[2],original_viewport[3]);
	
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_LIGHTING);
//	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D,m_texture_id/*scr_image.GetTextureNum()*/);
	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLE_STRIP);
		glColor4f(1,1,1,alpha);
		glTexCoord2f(1.0f  ,0.0f);	glVertex2f(original_viewport[2]*fx+original_viewport[2], original_viewport[3]*fy+original_viewport[3]);
		glTexCoord2f(1.0f  ,1.0f);  glVertex2f(original_viewport[2]*fx+original_viewport[2], original_viewport[3]*fy			 );
		glTexCoord2f(0.0f  ,0.0f);	glVertex2f(original_viewport[2]*fx					   , original_viewport[3]*fy+original_viewport[3]);
		glTexCoord2f(0.0f  ,1.0f);	glVertex2f(original_viewport[2]*fx					   , original_viewport[3]*fy			 );
	glEnd();

	ViewPerspective();

	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
//	glEnable(GL_BLEND);
}

void __fastcall glScreen::DrawBlur(float x, float y, int draw_width, int draw_height,float tex_width, float tex_height, float alpha,int radius,int step,float clrbias)
{
	if(step==0) step=1;

	ViewOrtho(original_viewport[2],original_viewport[3]);
	
	float th = tex_height;
	float tw = tex_width;
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, m_texture_id/*scr_image.GetTextureNum()*/);
	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLE_STRIP);
	float i;
    float dx=0,dy=0;
	if(radius<0) radius=0;
	for(i=0; i<=radius; i+=step){
        dx+=x;
        dy+=y;
		if(radius!=0.0f) alpha -= 1.0f/(float)radius;
		if(alpha>0){
			glColor4f(clrbias, clrbias, clrbias, alpha);
			glTexCoord2f(tw  ,0.0f);	glVertex2f(dx+draw_width+i*2	, dy+draw_height+i*2);
			glTexCoord2f(tw  ,th  );	glVertex2f(dx+draw_width+i*2	, dy-i			 );
			glTexCoord2f(0.0f,0.0f);	glVertex2f(dx-i				, dy+draw_height+i*2);
			glTexCoord2f(0.0f,th  );	glVertex2f(dx-i				, dy-i			 );
		}
	}
	glEnd();

	ViewPerspective();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
}
