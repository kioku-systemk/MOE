#include "stdafx.h"
#include "KDemo.h"

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

extern PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD3FARBPROC		glMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD4FARBPROC		glMultiTexCoord4fARB;
extern PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
extern PFNWGLCHOOSEPIXELFORMATARBPROC	wglChoosePixelFormatARB;

void KScene::Init()
{
	//scenename="scene_name";
	anim_data=(KCloneData*)GlobalAlloc(GPTR,sizeof(KCloneData)*65000);
	sceneobj_num=0;
	fscene_time=0;

	glGetIntegerv(GL_VIEWPORT, npViewport);
	//float fX = npViewport[0] - npViewport[2] * 0.5f;
	//float fY = npViewport[1] - npViewport[3] * 0.5f;
}

KScene::KScene(const KScene &scene_obj)
{
	//scenename = scene_obj.scenename;
	sceneobj_num = scene_obj.sceneobj_num;
	sceneobj = scene_obj.sceneobj;
	anim_data=(KCloneData*)GlobalAlloc(GPTR,sizeof(KCloneData)*65000);
	fscene_time = scene_obj.fscene_time;
}

void KScene::RenderScene( float scenetime, int nomat, int drawis_cameratrans2)
{	
	const int camera_mode = 1;
	if(camera_mode==1){
		glPushMatrix();
		KCloneData bps = sceneobj[0].anim.GetBoneTrans(0,scenetime, sceneobj[0].interpolate);
		KCloneData blk = sceneobj[0].anim.GetBoneTrans(1,scenetime, sceneobj[0].interpolate);
		CVector pos = bps.pos;
		CVector lookat = blk.pos;
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		float aspect = npViewport[2]/(float)npViewport[3];
		//const float initial_aspect = 1.0f;//1.333333334f;// for 4:3
		//const float correct_aspect = (float)npViewport[3] / (float)npViewport[2];// * 4.0f / 3.0f;
		//const float aspect = initial_aspect / correct_aspect;//アスペクト比の初期化

		gluPerspective ((60.0f+bps.rot.y), aspect, 0.1f, 1000.0f);
		//char str[64];
		//wsprintf(str, "%d\n", (int)(aspect*1000));
		//OutputDebugString(str);

		glMatrixMode(GL_MODELVIEW);
		glRotatef(bps.rot.z,0,0,1);
		gluLookAt(pos.x, pos.y, pos.z, lookat.x, lookat.y, lookat.z, 0, 1, 0);
	}


	//vector<CSceneObject>::iterator it, eit=sceneobj.end();
	KSceneObject* it; 
	for(it=sceneobj+camera_mode; it!=&sceneobj[sceneobj_num]; it++){//camera_modeならカメラをスキップ
		if(it->model!=NULL){
			BOOL isIndependFromCamera = ((it->is_cameratrans==0) || (it->is_cameratrans==2 && drawis_cameratrans2>=1));
			if(isIndependFromCamera){
				glPushMatrix();
				glLoadIdentity();
				gluLookAt(0, 0, -1, 0, 0, 1, 0, 1, 0);
			}

			if(it->is_visible==1){
				long ci;
				const long ci_max = it->model->GetCloneAllocNum();
				for(ci=0; ci<ci_max; ci++) anim_data[ci] = it->anim.GetBoneTrans(ci,scenetime, it->interpolate);

				if(isIndependFromCamera){
					glDisable(GL_DEPTH_TEST);
				}

				if(drawis_cameratrans2<=1){//NO_DOFのオブジェクトを含めて描画するか，もしくはNO_DOFのオブジェクト抜きで描画するか
					if((it->is_cameratrans!=0)||(camera_mode!=0)){//今のところすべての条件
						if(it->is_cameratrans==2 && drawis_cameratrans2==0){//NO_DOFなモデル && NO_DOFのオブジェクトを描画しない
							//nop
						}else{//主観モード，または，カメラの座標変換を受ける場合で，なおかつNO_DOFなオブジェクトな場合で，なおかつNO_DOFなオブジェクトを描画しない設定ではない場合
							it->model->Draw(0,anim_data,nomat);
						}
					}
				}else if(drawis_cameratrans2==2){//NO_DOFのオブジェクトのみ描画
					if(it->is_cameratrans==2){//NO_DOFなモデルなら描画
					//if(camera_mode!=0 && it->is_cameratrans==2){//NO_DOFなモデルで，カメラモードの場合
						it->model->Draw(0,anim_data,nomat);
					}
				}
				
				if(isIndependFromCamera){
					glEnable(GL_DEPTH_TEST);
				}
			}

			if(isIndependFromCamera){
				glPopMatrix();
			}
		}
	}
	if(camera_mode==1) glPopMatrix();
}

KDemo::KDemo()
{
	//camera_ptr = camera;
	ldpersentage=0.0f;
	scene_num=0;
	long i;
	for(i=0; i<CDEMO_OBJECT_NUMMAX; i++) obj[i]=NULL;
}

//
//CDemo::~CDemo()
//{
//	camera_ptr = NULL;
//	offsettime = 0;
//	scene.clear();
//	scene_endtime.clear();
//	long i;
//	for(i=0; i<CDEMO_OBJECT_NUMMAX; i++){
//		if(obj[i]!=NULL){
//			delete  obj[i];
//			obj[i]=NULL;
//		}
//	}
//}

void KDemo::Play(){
	dwStartTime = timeGetTime();
	if(ks){
		//ks->vol = 1.0f;
		ks->Play();
	}
#ifdef NOT64K
	if(cs){
		cs->Play();
	}
#endif
}

float KDemo::GetLoadPersentage()
{
	return ldpersentage;
}

bool KDemo::Load(const unsigned char* dptr, void (*Loader)(float), bool highmode)//binary load
{
	long i;
	//init
	//'K''D''B'
	const unsigned char* doffset=(&dptr[3]);
	long kmb_num  = *(unsigned char*)(doffset++);
	for(i=0; i<kmb_num; i++){
		unsigned long id = *(unsigned char*)(doffset++);
		unsigned long kmbsize = *(unsigned short*)(doffset); doffset+=sizeof(unsigned short);
		obj[id] = (KModel*)GlobalAlloc(GPTR,sizeof(KModel));
		obj[id]->Load(doffset); doffset+=kmbsize;
		ldpersentage = i/(float)kmb_num*0.8f;//progress
		Loader(ldpersentage);
	}
	
	long sc_num = *(unsigned short*)(doffset); doffset+=2;
	scene_num = sc_num;
	scene = (KScene*)GlobalAlloc(GPTR,sizeof(KScene)*sc_num);
	scene_endtime = (float*)GlobalAlloc(GPTR, sizeof(float) * sc_num);

	for(i=0; i<sc_num; i++){
		ldpersentage = (i+1)/(float)(sc_num)*0.2f + 0.8f;//progress
		Loader(ldpersentage);
		scene[i].Init();
//fscene_time
		float scene_time = *(float*)(doffset); doffset+=sizeof(float);
		scene[i].fscene_time = scene_time;
//fscene_time
		long object_num = *(unsigned short*)(doffset); doffset+=sizeof(unsigned short);
		scene[i].sceneobj_num = object_num;
		scene[i].sceneobj = (KSceneObject*)GlobalAlloc(GPTR,sizeof(KSceneObject)*object_num);
		KSceneObject *ob,*eob=&scene[i].sceneobj[object_num];
		for(ob=scene[i].sceneobj; ob!=eob; ob++){
			long prm = *(unsigned char*)(doffset++);
			ob->model_num = prm;
			if(prm!=255) ob->model = obj[prm];//not camera
//iscameratrans and interpolate
			unsigned char optn	= *(unsigned char*)(doffset++);
			ob->is_cameratrans	= optn&0x0F;
			ob->interpolate		= optn>>4;
			//ob->is_cameratrans = *(unsigned char*)(doffset++);
//iscameratrans and interpolate
			ob->is_visible = 1;
			long an_num = *(unsigned short*)(doffset); doffset+=sizeof(unsigned short);
			ob->anim.animation_num = an_num;
			ob->anim.anim_time = (float*)GlobalAlloc(GPTR,sizeof(float)*an_num);
			float *t,*et=&ob->anim.anim_time[an_num];
			for(t=ob->anim.anim_time; t!=et; t++){
				*t = *(float*)(doffset); doffset+=sizeof(float);
			}
			long pt_num = *(unsigned short*)(doffset); doffset+=sizeof(unsigned short);
			ob->anim.anim = (KCloneData**)GlobalAlloc(GPTR,sizeof(KCloneData*)*pt_num);
			KCloneData **kc,**ekc=&ob->anim.anim[pt_num];
			for(kc=ob->anim.anim; kc!=ekc; kc++){
				(*kc) = (KCloneData*)GlobalAlloc(GPTR,sizeof(KCloneData)*an_num);
				KCloneData *tkc,*etkc=(*kc+an_num);
				unsigned char flag = *(unsigned char*)(doffset++);
				if(flag&1){
					for(tkc=(*kc); tkc!=etkc; tkc++){
						tkc->pos.x = hftof(*(unsigned short*)(doffset)); doffset+=sizeof(unsigned short);
						tkc->pos.y = hftof(*(unsigned short*)(doffset)); doffset+=sizeof(unsigned short);
						tkc->pos.z = hftof(*(unsigned short*)(doffset)); doffset+=sizeof(unsigned short);
					}
				}
				if(flag&2){
					for(tkc=(*kc); tkc!=etkc; tkc++){
						tkc->rot.x = hftof(*(unsigned short*)(doffset)); doffset+=sizeof(unsigned short);
						tkc->rot.y = hftof(*(unsigned short*)(doffset)); doffset+=sizeof(unsigned short);
						tkc->rot.z = hftof(*(unsigned short*)(doffset)); doffset+=sizeof(unsigned short);
					}
				}
				if(flag&4){
					for(tkc=(*kc); tkc!=etkc; tkc++){
						tkc->scale.x = hftof(*(unsigned short*)(doffset)); doffset+=sizeof(unsigned short);
						tkc->scale.y = hftof(*(unsigned short*)(doffset)); doffset+=sizeof(unsigned short);
						tkc->scale.z = hftof(*(unsigned short*)(doffset)); doffset+=sizeof(unsigned short);
					}
				}else{
					for(tkc=(*kc); tkc!=etkc; tkc++){
						tkc->scale.x = 1.0f;
						tkc->scale.y = 1.0f;
						tkc->scale.z = 1.0f;
					}
				}
				if(flag&8){
					for(tkc=(*kc); tkc!=etkc; tkc++){
						tkc->alpha = hftof(*(unsigned short*)(doffset)); doffset+=sizeof(unsigned short);
					}
				}else{
					for(tkc=(*kc); tkc!=etkc; tkc++){
						tkc->alpha = 1.0f;
					}
				}
			}
		}
	}
	
	//search for music
	while(0xCCCCCCCC!=*(unsigned long*)(doffset)){
		//0xCCが出てくるまで探索
		doffset++;
	}doffset+=sizeof(char)*4;
//music
	unsigned char isMusicExists = *(unsigned char*)(doffset++);
#ifdef NOT64K
	unsigned char isKS =   isMusicExists == 1;
	unsigned char isWave = isMusicExists == 2;
	//unsigned char isOgg = byMusicType  == 3;
#endif
	if(isMusicExists){
#ifdef NOT64K
		if( isKS )
#endif
		{
			ks = (KSynth*)GlobalAlloc(GPTR, sizeof(KSynth));
			ks->Init();

			ks->vol = *(float*)doffset; doffset+=sizeof(float);
			char* szTrack = (char*)doffset; doffset+=lstrlen(szTrack)+1;
			ks->SetTrack(szTrack);

			unsigned short i_cnt = *(unsigned short*)doffset; doffset+=sizeof(unsigned short);
			KOsc osc;
			for(i=0; i<i_cnt; i++){
				osc.algorithm = *(unsigned char*)doffset; doffset+=sizeof(unsigned char); //fwrite(&ks->Osc[i].algorithm, sizeof(unsigned char), 1, fp);
				osc.wavetype = *(unsigned char*)doffset; doffset+=sizeof(unsigned char);
				osc.waveamp = *(float*)doffset; doffset+=sizeof(float);
				osc.wavefrq = *(float*)doffset; doffset+=sizeof(float);
				osc.waveendfrq = *(float*)doffset; doffset+=sizeof(float);
				osc.wavefine = *(float*)doffset; doffset+=sizeof(float);
				osc.fmtype = *(unsigned char*)doffset; doffset+=sizeof(unsigned char);
				osc.fmamp = *(float*)doffset; doffset+=sizeof(float);
				osc.fmfrq = *(float*)doffset; doffset+=sizeof(float);
				osc.fmendfrq = *(float*)doffset; doffset+=sizeof(float);
				osc.fmdetune = *(float*)doffset; doffset+=sizeof(float);
				osc.attacktime = *(float*)doffset; doffset+=sizeof(float);
				osc.releaselevel = *(float*)doffset; doffset+=sizeof(float);
				osc.releasetime = *(float*)doffset; doffset+=sizeof(float);
				osc.instvol = *(float*)doffset; doffset+=sizeof(float);

				ks->Osc[i].SetOsc(osc.algorithm, osc.wavetype, osc.waveamp, osc.wavefrq, osc.waveendfrq, osc.wavefine, osc.fmtype, osc.fmamp, osc.fmfrq, osc.fmendfrq, osc.fmdetune, osc.attacktime, osc.releaselevel, osc.releasetime, osc.instvol);
			}
			for(i=0; i<i_cnt; i++){
				osc.delaytime = *(long*)doffset; doffset+=sizeof(long);
				osc.delaylevel = *(float*)doffset; doffset+=sizeof(float);
				osc.conductance = *(float*)doffset; doffset+=sizeof(float);
				osc.hipass = *(float*)doffset; doffset+=sizeof(float);
				osc.inductance = *(float*)doffset; doffset+=sizeof(float);
				ks->Osc[i].SetFilter(osc.delaytime/44100.0f, osc.delaylevel, osc.conductance, osc.hipass, osc.inductance);
			}
			
			//chartime
			i_cnt = *(unsigned short*)doffset; doffset+=sizeof(unsigned short);
			for(i=0; i<i_cnt; i++){
				unsigned short target = *(unsigned short*)doffset; doffset+=sizeof(unsigned short);
				ks->charTime[target] = *(float*)doffset; doffset+=sizeof(float);
			}

			//sequenct
			i_cnt = *(unsigned short*)doffset; doffset+=sizeof(unsigned short);
			for(i=0; i<i_cnt; i++){
				unsigned char track = *(unsigned char*)doffset; doffset+=sizeof(unsigned char);
				unsigned char inst = *(unsigned char*)doffset; doffset+=sizeof(unsigned char);
				unsigned char onum = *(unsigned char*)doffset; doffset+=sizeof(unsigned char);
				char* szSeqence = (char*)doffset; doffset+=lstrlen(szSeqence)+1;
				ks->SetSequence(track, inst, onum, szSeqence);
			}
		}
#ifdef NOT64K
		cs = (CSound*)GlobalAlloc(GPTR, sizeof(CSound));
		cs->Initialize(GetDesktopWindow());
		if( isWave ){
			float fVol = *(float*)doffset; doffset+=sizeof(float);
			unsigned long ulDataSize = *(unsigned long*)doffset; doffset+=sizeof(unsigned long);
			unsigned char* pWavFile = (unsigned char*)GlobalAlloc(GPTR, ulDataSize);
			CopyMemory(pWavFile, doffset, ulDataSize); doffset += ulDataSize;
			this->cs->LoadMemoryImage(pWavFile, ulDataSize);
			
			this->SetVolume(fVol);//ロードしたあとでないと、SetVolumeは失敗する
		}
#endif
	}
//music

	//CreateDOFTexture()
	DepthOfField = (glScreen*)GlobalAlloc(GPTR, sizeof(glScreen)*DOF_TEXTURE_NUM);
	for(i=0; i<DOF_TEXTURE_NUM; i++){
		if (highmode)
			DepthOfField[i].CreateTexture(1024);
		else
			DepthOfField[i].CreateTexture(512);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, DepthOfField[i].GetTextureNum());
		glDisable(GL_TEXTURE_2D);

	}

	ReadyDemo();
	return true;
}

void KDemo::DrawMultiTexturePolygon(float x, float y, float width, float height, int nTexture){
	int i;
	glBegin(GL_TRIANGLE_STRIP);
		for(i=0; i<nTexture; i++){
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, 1.0, 0.0);
		}
		glVertex2f(x+width, y+height);

		for(i=0; i<nTexture; i++){
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, 1.0, 1.0);
		}
		glVertex2f(x+width, y			 );
		
		for(i=0; i<nTexture; i++){
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, 0.0, 0.0);
		}
		glVertex2f(x					 , y+height);

		for(i=0; i<nTexture; i++){
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, 0.0, 1.0);
		}
		glVertex2f(x					 , y			 );
	glEnd();
}

void KDemo::ActivateMultiTexture(const int nTexture, const unsigned int* image, const unsigned int* rgb_op, const unsigned int src_rgb[][3], const unsigned int ope_rgb[][3], const unsigned int* alpha_op, const unsigned int src_alp[][3], const unsigned int ope_alp[][3]){
	int i,k;
	for(i=0; i<nTexture; i++){
		glActiveTextureARB(GL_TEXTURE0_ARB + i);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, image[i]);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, rgb_op[i]);
		for(k=0; k<3; k++){
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB+k, src_rgb[i][k]);
		}
		for(k=0; k<3; k++){
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB+k, ope_rgb[i][k]);
		}

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, alpha_op[i]);
		for(k=0; k<3; k++){
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB+k, src_alp[i][k]);
		}
		for(k=0; k<3; k++){
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB+k, ope_alp[i][k]);
		}
	}
}

void KDemo::DeActivateMultiTexture(int nTexture){
	int i;
	for(i=nTexture-1; i>=0; i--){
		glActiveTextureARB(GL_TEXTURE0_ARB + i);
		//glBindTexture(GL_TEXTURE_2D, 0); //意外に重い
		glDisable(GL_TEXTURE_2D);
	}
}

void KDemo::RenderScene(long sc, float scenetime){
	KCloneData clk = scene[sc].sceneobj[0].anim.GetBoneTrans(1, scenetime, scene[sc].sceneobj[0].interpolate);
	CVector lookat_rot = clk.rot;

	if(lookat_rot.x<=0.0f && lookat_rot.y<=0.0f && lookat_rot.z<=0.0f){	//被写界震度のパラメータが特に指定されていないときは，DOFの無用な演算は避けるべきだ。
		this->scene[sc].RenderScene(scenetime);
		return;
	}

	//
	//DOFの演算は以下のとおり
	//
	/*
		DepthOfField[7].RenderStart();//この中でglClear()が呼ばれてる
	*/

	//RenderToTextureの影響を受ける前のビューポート.
	int original_viewport[4];
	glGetIntegerv(GL_VIEWPORT, original_viewport);

	//DepthOfField[10].RenderStart();
	//	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//	this->scene[sc].RenderScene(scenetime, cameramode, 0, 2);//最後に, is_cameratrans==2のオブジェクトだけDOF抜きで描画
	//DepthOfField[10].RenderEnd();

	DepthOfField[0].RenderStart();//真っ白テクスチャ
	DepthOfField[0].RenderEnd();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	DepthOfField[6].RenderStart();//手前をぼかすためのテクスチャ(手前がしろ．奥がくろ)
	{
		GLfloat fogColor[4]= {0.0f, 0.0f, 0.0f, 1.0f};

		glEnable(GL_FOG);
			//glDisable(GL_LIGHT0);
			glDisable(GL_LIGHTING);
			glFogi(GL_FOG_MODE, GL_LINEAR);
			glFogfv(GL_FOG_COLOR, fogColor);
			glHint(GL_FOG_HINT, GL_DONT_CARE);

			if(lookat_rot.y<=0.0f) lookat_rot.y = 0.01f;
			glFogf(GL_FOG_START, lookat_rot.z - lookat_rot.y*0.1f);// * (lookat_rot.y/360.0f));
			glFogf(GL_FOG_END,   lookat_rot.z);
			//glFogf(GL_FOG_END,   10.0f * (lookat_rot.y/360.0f));
			this->scene[sc].RenderScene(scenetime, 2, 0);
			//this->scene[sc].RenderScene(scenetime, cameramode, 2, 2);
			glEnable(GL_LIGHTING);
			//glEna
		glDisable(GL_FOG);
	}
	DepthOfField[6].RenderEnd();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	DepthOfField[1].RenderStart();//奥をぼかすためのテクスチャ(手前が黒．奥が白)
	{
		GLfloat fogColor[4]= {1.0f, 1.0f, 1.0f, 1.0f};

		glEnable(GL_FOG);
			glDisable(GL_LIGHTING);
			glFogi(GL_FOG_MODE, GL_LINEAR);
			glFogfv(GL_FOG_COLOR, fogColor);
			glHint(GL_FOG_HINT, GL_DONT_CARE);

			if(lookat_rot.z<=0.0f) lookat_rot.z = 0.0f;
			//glFogf(GL_FOG_DENSITY, 10.00 * (rotz/360.0f));
			//glHint(GL_FOG_HINT, GL_DONT_CARE);
			float width = 0.0f;
			glFogf(GL_FOG_START, lookat_rot.z);// * (lookat_rot.z/360.0f));
			glFogf(GL_FOG_END,   lookat_rot.z + lookat_rot.y*0.1f);
			//glFogf(GL_FOG_END,   110.0f * (lookat_rot.z/360.0f));
			this->scene[sc].RenderScene(scenetime, 1, 0);
			//this->scene[sc].RenderScene(scenetime, cameramode, 1, 2);
			glEnable(GL_LIGHTING);
		glDisable(GL_FOG);
	}
	DepthOfField[1].RenderEnd();

	DepthOfField[9].RenderStart();//[1]+[6]
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_CULL_FACE);

		const int nTexture = 2;
		const unsigned int image[] = {DepthOfField[1].GetTextureNum(), DepthOfField[6].GetTextureNum()};

		const unsigned int rgb_op[] = {GL_REPLACE, GL_ADD};
		const unsigned int alp_op[] = {GL_REPLACE, GL_REPLACE};
		const unsigned int src_rgb_param[][3] = {
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
		};
		const unsigned int ope_rgb_param[][3] = {
			{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA},
			{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA}
		};
		const unsigned int src_alpha_param[][3] = {
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
		};
		const unsigned int ope_alpha_param[][3] = {
			{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA},
			{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA}
		};
		glPushAttrib(GL_TEXTURE_BIT);
			ActivateMultiTexture(nTexture, image, rgb_op, src_rgb_param, ope_rgb_param, alp_op, src_alpha_param, ope_alpha_param);
		
			int x=0, y=0;
			DepthOfField[9].ViewOrtho(original_viewport[2],original_viewport[3]);
		
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			DrawMultiTexturePolygon((float)x, (float)y, (float)original_viewport[2], (float)original_viewport[3], nTexture);

			DeActivateMultiTexture(nTexture);
			DepthOfField[9].ViewPerspective();
		glPopAttrib();

		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_ALPHA_TEST);
	}
	DepthOfField[9].RenderEnd();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	DepthOfField[2].RenderStart();//普通
		this->scene[sc].RenderScene(scenetime, 0, 0);
		//this->scene[sc].RenderScene(scenetime, cameramode, 0, 2);
	DepthOfField[2].RenderEnd();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	DepthOfField[7].RenderStart();
		this->scene[sc].RenderScene(scenetime, 0, 0);
	DepthOfField[7].RenderEnd();
	//DepthOfField[8].RenderStart();
	//DepthOfField[8].RenderEnd();

	//DepthOfField[3].RenderStart();//低解像度
	{
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			if(lookat_rot.x<=0.0f) lookat_rot.x = 0.0f;
			int nRadius = 1;
			nRadius = (int)(nRadius + (lookat_rot.x/30.0f));
			int nOffset = 1;
			glEnable(GL_TEXTURE_2D);

			glScreen* pTextureIn = &DepthOfField[8], *pTextureOut = &DepthOfField[7];//&DepthOfField[8];
			while(nOffset <= nRadius){//nOffSetはピクセル単位.
				pTextureIn->RenderStart();
					pTextureOut->DrawScreen(-nOffset/(float)original_viewport[2], 0.0f, 0.5f);
					pTextureOut->DrawScreen(+nOffset/(float)original_viewport[2], 0.0f, 0.5f);
				pTextureIn->RenderEnd();
				
				glScreen* pTextureTmp = pTextureOut;//swap pTextureIn <-> pTextureOut
				pTextureOut = pTextureIn;
				pTextureIn = pTextureTmp;

				pTextureIn->RenderStart();
					pTextureOut->DrawScreen(0.0f, -nOffset/(float)original_viewport[3], 0.5f);
					pTextureOut->DrawScreen(0.0f, +nOffset/(float)original_viewport[3], 0.5f);
				pTextureIn->RenderEnd();

				nOffset = nOffset * 2;

				//pTexture = pTexture==&DepthOfField[7] ? &DepthOfField[8] : &DepthOfField[7];
				pTextureTmp = pTextureOut;//swap pTextureIn <-> pTextureOut
				pTextureOut = pTextureIn;
				pTextureIn = pTextureTmp;
			}
			//
		glDisable(GL_BLEND);
			DepthOfField[3].RenderStart();//低解像度
				//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				pTextureOut->DrawScreen(0.0f, 0.0f);
			DepthOfField[3].RenderEnd();
		glEnable(GL_BLEND);
	}

	DepthOfField[4].RenderStart();//[1]*[3] : 手前が暗く、奥が明るい
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_CULL_FACE);

		const int nTexture = 2;
		const unsigned int image[] = {DepthOfField[9].GetTextureNum() ,DepthOfField[3].GetTextureNum() };

		const unsigned int rgb_op[] = {GL_REPLACE, GL_MODULATE};//マルチテクスチャ用オペレータ
		const unsigned int alp_op[] = {GL_REPLACE, GL_REPLACE};//マルチテクスチャ用オペレータ
		const unsigned int src_rgb_param[][3] = {
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
		};
		const unsigned int ope_rgb_param[][3] = {
			{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA},
			{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA}
		};
		const unsigned int src_alpha_param[][3] = {
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
		};
		const unsigned int ope_alpha_param[][3] = {
			{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA},
			{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA}
		};
		glPushAttrib(GL_TEXTURE_BIT);
			ActivateMultiTexture(nTexture, image, rgb_op, src_rgb_param, ope_rgb_param, alp_op, src_alpha_param, ope_alpha_param);

			int x=0, y=0;
			DepthOfField[4].ViewOrtho(original_viewport[2],original_viewport[3]);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			DrawMultiTexturePolygon((float)x, (float)y, (float)original_viewport[2], (float)original_viewport[3], nTexture);
			DeActivateMultiTexture(nTexture);

			DepthOfField[4].ViewPerspective();
		glPopAttrib();

		glEnable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_ALPHA_TEST);
	}
	DepthOfField[4].RenderEnd();

	DepthOfField[5].RenderStart();//(1-[1])*[3] + [1]*[2] = ([0]-[1])*[3] + [4]
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_CULL_FACE);

		const int nTexture = 3;
		const unsigned int image[] = {DepthOfField[9].GetTextureNum(),
								DepthOfField[2].GetTextureNum(),
								DepthOfField[4].GetTextureNum()};

		const unsigned int rgb_op[] = {GL_REPLACE, GL_MODULATE, GL_ADD};//マルチテクスチャ用オペレータ
		const unsigned int alp_op[] = {GL_REPLACE, GL_REPLACE, GL_REPLACE};//マルチテクスチャ用オペレータ
		const unsigned int src_rgb_param[][3] = {
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
		};
		const unsigned int ope_rgb_param[][3] = {
			{GL_ONE_MINUS_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA},
			{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA},
			{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA}
		};
		const unsigned int src_alpha_param[][3] = {
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
			{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
		};
		const unsigned int ope_alpha_param[][3] = {
			{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA},
			{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA},
			{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA}
		};
		glPushAttrib(GL_TEXTURE_BIT);
			ActivateMultiTexture(nTexture, image, rgb_op, src_rgb_param, ope_rgb_param, alp_op, src_alpha_param, ope_alpha_param);
		
			int x=0, y=0;
			DepthOfField[5].ViewOrtho(original_viewport[2],original_viewport[3]);
		
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			DrawMultiTexturePolygon((float)x, (float)y, (float)original_viewport[2], (float)original_viewport[3], nTexture);

			DeActivateMultiTexture(nTexture);

			DepthOfField[5].ViewPerspective();
		glPopAttrib();

		glEnable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
	}
	DepthOfField[5].RenderEnd();
	
	//output
	DepthOfField[5].DrawScreen(0.0f, 0.0f);
	this->scene[sc].RenderScene(scenetime, 0, 2);
}

int KDemo::RenderDemo(){
	float demotime_ms = 0;
	demotime_ms += (timeGetTime()-dwStartTime)*1.00f;

	long sc_num = scene_num;
	if(scene_endtime[sc_num-1]<demotime_ms){
		return 1;
	}
	long i;
	for(i=0; i<sc_num; i++){
		if(this->scene_endtime[i]>=demotime_ms){
			float sn_tim = (i==0) ? demotime_ms : (demotime_ms - this->scene_endtime[i-1]);
			//float fscene_length = scene[i].fscene_time;
			float fscene_length = (scene[i].fscene_time<=0.0f) ? 0.0000001f : this->scene[i].fscene_time;
			float sn_rate = sn_tim / fscene_length;
			sn_rate *= 1.0011f;

			this->RenderScene(i, sn_rate);
			break;
		}
	}
	return 0;
}

void KDemo::ReadyDemo(){
	//ZeroMemory(this->scene_endtime, sc_num*sizeof(float));
	long i, sc_num = scene_num;
	for(i=0; i<sc_num; i++){
		scene_endtime[i] = (i==0) ? this->scene[i].fscene_time : this->scene[i].fscene_time + scene_endtime[i-1];
	}
}

void KDemo::SeekRelative(long seek_time_ms){
	dwStartTime += seek_time_ms;
	if(this->ks){
		ks->SetSeek((float)(timeGetTime() - dwStartTime));
	}
#ifdef NOT64K
	if(this->cs){
		cs->Play(timeGetTime() - dwStartTime);
	}
#endif
}

void KDemo::SetVolume(float fVolume){
	if(this->ks){
		ks->vol = fVolume;
	}
#ifdef NOT64K
	if(this->cs){
		cs->SetVolume( CS_VOLUME_FIX_SET((LONG)(-(CS_VOLUME_RANGE - fVolume*CS_VOLUME_RANGE))) );
		//cs->SetVolume( CS_VOLUME_DB((LONG)((1.0f-fVolume)*-10.0f)) );
	}
#endif
}

float KDemo::GetVolume(){
#ifdef NOT64K
	return (this->ks) ? ks->vol : (this->cs) ? (CS_VOLUME_RANGE + cs->GetVolume())/(float)CS_VOLUME_RANGE/*((100-(cs->GetVolume()/100))/100.0f)*/ : 0;
#endif
	return (this->ks) ? ks->vol : 0;
}