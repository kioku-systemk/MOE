//Modification history
//--2006/Jan/14
//CDemo::Save(const char* filename) modified by c.r.v.
//using tag "MakeRelative"
//--"next modification date comes here"
#include "stdafx.h"
#include "cdemo.h"
#include "CAnimation.h"
#include "../klib/kmodeledit.h"
//<MakeRelative>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
//</MakeRelative>

#include <GL/gl.h>
#include <GL/glu.h>
#include "../gl/glext.h"
//#include <iostream>
//#include <vector>
//#include <stdlib.h>
#include <stdio.h>
//using namespace std;

extern PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD3FARBPROC		glMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD4FARBPROC		glMultiTexCoord4fARB;
extern PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;

CScene::CScene(KCloneData* anim_buffer)
{
	scene_track = "";
	fscene_time = 0.00f;
	scenename="scene_name";
	//anim_data=(KCloneData*)GlobalAlloc(GPTR,sizeof(KCloneData)*65000);
	anim_data = anim_buffer;
}

CScene::~CScene()
{
	scenename.clear();
	sceneobj.clear();
	//GlobalFree(anim_data);
}

CScene::CScene(const CScene &scene_obj)
{
	scene_track = scene_obj.scene_track;
	fscene_time = scene_obj.fscene_time;
	scenename = scene_obj.scenename;
	sceneobj = scene_obj.sceneobj;
	//anim_data=(KCloneData*)GlobalAlloc(GPTR,sizeof(KCloneData)*65000);
	//CopyMemory(anim_data,scene_obj.anim_data,sizeof(KCloneData)*65000);
	anim_data = scene_obj.anim_data;
}
CScene &CScene::operator=(const CScene &scene_obj)
{
	scene_track = scene_obj.scene_track;
	fscene_time = scene_obj.fscene_time;
	scenename = scene_obj.scenename;
	sceneobj = scene_obj.sceneobj;
	//anim_data=(KCloneData*)GlobalAlloc(GPTR,sizeof(KCloneData)*65000);
	//CopyMemory(anim_data,scene_obj.anim_data,sizeof(KCloneData)*65000);
	anim_data = scene_obj.anim_data;
	return *this;
}

void CScene::RenderScene( float scenetime, int camera_mode, int nomat, int drawis_cameratrans2)
{	
	if(camera_mode==1){
		glPushMatrix();
		KCloneData bps = sceneobj[0].anim.GetBoneTrans(0,scenetime, sceneobj[0].interpolate);
		KCloneData blk = sceneobj[0].anim.GetBoneTrans(1,scenetime, sceneobj[0].interpolate);
		CVector pos = bps.pos;
		CVector lookat = blk.pos;
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective (60.0f+bps.rot.y,1.333f,0.1f,1000.0f);

		glMatrixMode(GL_MODELVIEW);
		glRotatef(bps.rot.z,0,0,1);
		gluLookAt(pos.x, pos.y, pos.z, lookat.x, lookat.y, lookat.z, 0, 1, 0);
	}

	vector<CSceneObject>::iterator it, eit=sceneobj.end();
	for(it=sceneobj.begin()+camera_mode; it!=eit; it++){//camera_mode�Ȃ�J�������X�L�b�v
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

				//if((it->is_cameratrans!=0)||(camera_mode!=0)){//�J�����̍��W�ϊ����󂯂邩�C�܂��́C��σ��[�h�̏ꍇ�B�E�E�E���āC�S������ˁH�J�����̍��W�ϊ����󂯂Ȃ��Ă��C�J�������[�h�Ȃ�\�������킯�����B�t�ɁC�J�������[�h����Ȃ��Ă��C���W�ϊ����󂯂�I�u�W�F�N�g�͑S���`�悳���B
				//	it->model->Draw(0,anim_data);
				//}

				if(drawis_cameratrans2<=1){//NO_DOF�̃I�u�W�F�N�g���܂߂ĕ`�悷�邩�C��������NO_DOF�̃I�u�W�F�N�g�����ŕ`�悷�邩
					if((it->is_cameratrans!=0)||(camera_mode!=0)){//���̂Ƃ��낷�ׂĂ̏���
						if(it->is_cameratrans==2 && drawis_cameratrans2==0){//NO_DOF�ȃ��f�� && NO_DOF�̃I�u�W�F�N�g��`�悵�Ȃ�
							//nop
						}else{//��σ��[�h�C�܂��́C�J�����̍��W�ϊ����󂯂�ꍇ�ŁC�Ȃ�����NO_DOF�ȃI�u�W�F�N�g�ȏꍇ�ŁC�Ȃ�����NO_DOF�ȃI�u�W�F�N�g��`�悵�Ȃ��ݒ�ł͂Ȃ��ꍇ
							it->model->Draw(0,anim_data,nomat);
						}
					}
				}else if(drawis_cameratrans2==2){//NO_DOF�̃I�u�W�F�N�g�̂ݕ`��
					if(it->is_cameratrans==2){//NO_DOF�ȃ��f���Ȃ�`��
					//if(camera_mode!=0 && it->is_cameratrans==2){//NO_DOF�ȃ��f���ŁC�J�������[�h�̏ꍇ
						it->model->Draw(0,anim_data,nomat);
					}
				}
				
				////if((it->is_cameratrans!=0)||(camera_mode!=0)){
				////	if(it->is_cameratrans==2){//no DOF
				////		if(drawis_cameratrans2<=1){//
				////			//nop
				////		}else{
				////			it->model->Draw(0,anim_data,nomat);
				////		}
				////	}else{//��σ��[�h�łȂ����C�J�����̍��W�Ԋ҂��󂯂�ꍇ�B
				////		it->model->Draw(0,anim_data,nomat);
				////	}
				////}
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

KSynthLoader* CDemo::ksl = NULL;
CSound*       CDemo::cs  = NULL;
CDemo::CDemo(KModelEdit* camera)
{
	camera_ptr = camera;
	offsettime= 0;
	long i;
	for(i=0; i<CDEMO_OBJECT_NUMMAX; i++) obj[i]=NULL;
	anim_buffer=(KCloneData*)GlobalAlloc(GPTR,sizeof(KCloneData)*65000);
	music_file = "";

	if( !ksl){
		ksl = new KSynthLoader();
	}
#ifdef NOT64K
	if( !cs ){
		cs  = new CSound();
	}
#endif
}

CDemo::~CDemo()
{
	camera_ptr = NULL;
	offsettime = 0;
	scene.clear();
	long i;
	scene_endtime.clear();
	for(i=0; i<CDEMO_OBJECT_NUMMAX; i++){
		if(obj[i]!=NULL){
			//delete  obj[i];
			obj[i]=NULL;
		}
	}
	music_file.clear();

	//delete ksl;
	//delete cs;
}

glScreen CDemo::DepthOfField[11];
void CDemo::CreateDOFTexture(){
	//DepthOfField[0].CreateTexture(512);//���i�Ō�̌��Z�p�j//�����

	DepthOfField[1].CreateTexture(512);//�t�H�O�i���j
	DepthOfField[6].CreateTexture(512);//�t�H�O�i��O�p�j
	DepthOfField[9].CreateTexture(512);//�t�H�O�i��O�{���j

	DepthOfField[2].CreateTexture(512);//�ʏ�`��

	DepthOfField[3].CreateTexture(512);//�u���[�o�͐�
	DepthOfField[7].CreateTexture(512);//�u���[�p�e���|����1/2
	DepthOfField[8].CreateTexture(512);//�u���[�p�e���|����2/2

	DepthOfField[4].CreateTexture(512);//�t�H�O���u���[
	DepthOfField[5].CreateTexture(512);//�i�P�|�t�H�O�j���ʏ�`��{�i�t�H�O���u���[�j

	DepthOfField[10].CreateTexture(512);//
}

CDemo::CDemo(const CDemo &dobj)
{
	camera_ptr = dobj.camera_ptr;
	offsettime = dobj.offsettime;
	scene = dobj.scene;
	scene_endtime = dobj.scene_endtime ;
	anim_buffer=dobj.anim_buffer;
	music_file = dobj.music_file;

	//ksl = dobj.ksl;
	//cs  = dobj.cs;
}

void CDemo::CopyPrimitive(const CDemo &dobj)
{
	long i;
	for(i=0; i<CDEMO_OBJECT_NUMMAX; i++){
		  obj[i] = dobj.obj[i];
		  obj_name[i] = dobj.obj_name[i];
	}
}

void CDemo::ReadyDemo()
{
	//camera_time -> scenetime
	vector<CScene>::iterator it, eit=scene.end();
	scene_endtime.clear();
	for(it=scene.begin(); it!=eit; it++){
		if(it==scene.begin()) scene_endtime.push_back(it->fscene_time);
		else				  scene_endtime.push_back(it->fscene_time + scene_endtime[(long)scene_endtime.size()-1]);
	}
}
extern CWindow win;
int CDemo::RenderDemo(float demotime_ms, float* fRecvCurrentRate)
{
	if(scene_endtime.size()==0){
		return 1;
	}else{
		if(scene_endtime[scene_endtime.size()-1]<demotime_ms){
			return 1;
		}
		vector<float>::iterator sit=scene_endtime.begin(),esit=scene_endtime.end();
		for(sit=scene_endtime.begin(); sit!=esit; sit++){
			if((*sit)>=demotime_ms){
				long sn = (long)(sit - scene_endtime.begin());
				float sn_tim = (sn==0) ? demotime_ms : (demotime_ms - *(sit-1));
				float fEndTime = (this->scene[sn].fscene_time<=0.0f) ? 0.0000001f : this->scene[sn].fscene_time;//�[�����Z���
				float sn_rate = sn_tim / fEndTime;
				sn_rate *= 1.0011f;
				//win.CSetWindowText(win.CGethWnd(), "%f", sn_rate);
				//float sn_rate = sn_tim / ((sn==0) ? fEndTime : (fEndTime - *(sit-1)));
				if(sn_rate<0.000000f) return 1;//�V�[�����[�g�̃}�C�i�X���

				if(fRecvCurrentRate!=NULL) *fRecvCurrentRate = sn_rate;
				this->RenderScene(sn, sn_rate, 1);
				break;
			}
		}
	}
	return 0;
}


bool CDemo::BinarySave(const char* filename)
{
	long i;
	FILE* fp;
	if((fp=fopen(filename,"wb"))==NULL){
		MessageBox(NULL,"Can't save file","error",MB_OK);
	}else{
		fwrite("KDB",3,1,fp);
	
		unsigned char jcnt=0;
		for(i=0; i<CDEMO_OBJECT_NUMMAX; i++){
			if(obj[i]!=NULL) jcnt++;
		}
		fwrite(&jcnt,sizeof(unsigned char),1,fp);//kmb_num
		for(i=0; i<CDEMO_OBJECT_NUMMAX; i++){
			if(obj[i]!=NULL){
				//fout << "\t" << i << " \"" << obj_name[i] << "\"" << endl;
				unsigned char id = (unsigned char)i;
				unsigned short siz = (unsigned short)obj[i]->GetKMBFileSize();
				fwrite(&id,sizeof(unsigned char), 1, fp);
				fwrite(&siz,sizeof(unsigned short), 1, fp);
				fwrite(obj[i]->GetKMBFilePtr(), sizeof(unsigned char), siz, fp);
				//fwrite((void*)obj[i]->GetKMBFilePtr(),siz,1,fp);
			}
		}

		unsigned short scnum = (unsigned short)scene.size();
		fwrite(&scnum,sizeof(unsigned short),1,fp);

		vector<CScene>::iterator sit,seit=scene.end();
		for(sit=scene.begin(); sit!=seit; sit++){
			//fout << "Scene \"" << sit->scenename << "\" {" << endl;
//fscene_time
			float fscenetime = (float)sit->fscene_time;
			fwrite(&fscenetime,sizeof(float),1,fp);
//fscene_time
			unsigned short obnum = (unsigned short)sit->sceneobj.size();
			fwrite(&obnum,sizeof(unsigned short),1,fp);

			vector<CSceneObject>::iterator oit,oeit=sit->sceneobj.end();
			for(oit=sit->sceneobj.begin(); oit!=oeit; oit++){
				//fout << "\tObject " << oit->model_num << " {" << endl;
					//fout << "\t\tCameraTrans " << oit->is_cameratrans << endl;
					//fout << "\t\tVisible " << oit->is_visible << endl;
					//fout << "\t\tDataNum " << (long)oit->anim.anim_time.size() << endl;
					unsigned char prm = (unsigned char)oit->model_num;
					fwrite(&prm,sizeof(unsigned char),1,fp);
//iscameratrans and interpolate
					unsigned char optn = (unsigned char)oit->is_cameratrans;
					optn |= ((unsigned char)oit->interpolate)<<4;
					fwrite(&optn,sizeof(unsigned char),1,fp);
					//unsigned char optn = (unsigned char)oit->is_cameratrans;
					//fwrite(&optn,sizeof(unsigned char),1,fp);
//iscameratrans and interpolate
					unsigned short dnum = (unsigned short)oit->anim.anim_time.size();
					fwrite(&dnum,sizeof(unsigned short),1,fp);

					//fout << "\t\tTime {" << endl;
						vector<float>::iterator tit,teit=oit->anim.anim_time.end();
						for(tit=oit->anim.anim_time.begin(); tit!=teit; tit++){
							//fout << "\t\t\t" << (*tit) << endl;
							float tmd = (*tit);
							fwrite(&tmd,sizeof(float),1,fp);
						}
					//fout << "\t\t}" << endl;//Time
					if(oit->model!=NULL){
						long i,obj_n=oit->model->GetCloneAllocNum();
						unsigned short pnum = (unsigned short)obj_n;
						fwrite(&pnum,sizeof(unsigned short),1,fp);
						for(i=0; i<obj_n; i++){
							KClone* kcn = oit->model->GetCloneAllocPtr ();
							string objname = kcn[i].clone_data.clone_name;
							//fout << "\t\tPart \""<< objname << "\" {" << endl;
								long t,tmax=(long)oit->anim.anim_time.size();
								long rs_pos,rs_rot,rs_scale,rs_alpha;
								//--------------Pos---------------------------------------
								rs_pos=0;
								for(t=0; t<tmax; t++){
									CVector* p = &oit->anim.anim[i][t].pos;
									if((p->x!=0.0f)||(p->y!=0.0f)||(p->z!=0.0f)) rs_pos=1;
								}
								//---------------Rot---------------------------------------
								rs_rot=0;
								for(t=0; t<tmax; t++){
									CVector* p = &oit->anim.anim[i][t].rot;
									if((p->x!=0.0f)||(p->y!=0.0f)||(p->z!=0.0f)) rs_rot=1;
								}
								//---------------Scale-------------------------------------
								rs_scale=0;
								for(t=0; t<tmax; t++){
									CVector* p = &oit->anim.anim[i][t].scale;
									if((p->x!=1.0f)||(p->y!=1.0f)||(p->z!=1.0f)) rs_scale=1;
								}
								//--------------Alpha--------------------------------------
								rs_alpha=0;
								for(t=0; t<tmax; t++){
									if(oit->anim.anim[i][t].alpha!=1.0f) rs_alpha=1;
								}
								
								//---------------------------------------------------------
								//flag
								unsigned char flg = (unsigned char)(rs_pos|(rs_rot<<1)|(rs_scale<<2)|(rs_alpha<<3));
								fwrite(&flg,sizeof(unsigned char),1,fp);

								if(rs_pos>0){
									//fout << "\t\t\tPosition {" << endl;
									for(t=0; t<tmax; t++){
										CVector* p = &oit->anim.anim[i][t].pos;
										short sp[3];
										sp[0] = ftohf(p->x);
										sp[1] = ftohf(p->y);
										sp[2] = ftohf(p->z);
										fwrite(&sp,sizeof(short),3,fp);
										//fout << "\t\t\t\t" << p->x << " "<< p->y << " "<< p->z << endl;
									}
									//fout << "\t\t\t}" << endl;
								}
								if(rs_rot>0){
									//fout << "\t\t\tRotation {" << endl;
									for(t=0; t<tmax; t++){
										CVector* p = &oit->anim.anim[i][t].rot;
										short sp[3];
										sp[0] = ftohf(p->x);
										sp[1] = ftohf(p->y);
										sp[2] = ftohf(p->z);
										fwrite(&sp,sizeof(short),3,fp);
										//fout << "\t\t\t\t" << p->x << " "<< p->y << " "<< p->z << endl;
									}
									//fout << "\t\t\t}" << endl;
								}
								if(rs_scale>0){
									//fout << "\t\t\tScale {" << endl;
									for(t=0; t<tmax; t++){
										CVector* p = &oit->anim.anim[i][t].scale;
										short sp[3];
										sp[0] = ftohf(p->x);
										sp[1] = ftohf(p->y);
										sp[2] = ftohf(p->z);
										fwrite(&sp,sizeof(short),3,fp);
										//fout << "\t\t\t\t" << p->x << " "<< p->y << " "<< p->z << endl;
									}
									//fout << "\t\t\t}" << endl;
								}
								if(rs_alpha>0){
									//fout << "\t\t\tAlpha {" << endl;
									for(t=0; t<tmax; t++){
										short al;
										al = ftohf(oit->anim.anim[i][t].alpha);
										fwrite(&al,sizeof(short),1,fp);
										//fout << "\t\t\t\t" << oit->anim.anim[i][t].alpha << endl;
									}
									//fout << "\t\t\t}" << endl;
								}
							//fout << "\t\t}" << endl;//Part
						}
					}
				//fout << "\t}" << endl;//Object
			}
			//fout << "}" << endl;//Scene
		}

		for(i=0; i<4; i++){
			unsigned char kslsimbol = 0xCC;
			fwrite((unsigned char*)&kslsimbol, sizeof(unsigned char), 1, fp); 
		}

		//music
		KSynth* ks = this->ksl->GetKSynthPtr();
//#ifdef 
		unsigned char isKsInside = (ks!=NULL) ? 1 : (cs!=NULL) ? 2 : 0;
		fwrite((unsigned char*)&isKsInside, sizeof(unsigned char), 1, fp);
		if(isKsInside==1){
			//short volume = (short)ftohf(ks->vol);
			//fwrite(&volume, sizeof(short), 1, fp);
			fwrite(&ks->vol, sizeof(float), 1, fp);

			char szTrack[1024*100] = {'\0'};
			this->ksl->GetKSynthTrackList(szTrack, sizeof(szTrack));
			fwrite(szTrack, sizeof(char)*(lstrlen(szTrack)+1), 1, fp);
			
			int j,k;
			int i_cnt=OSC_SLOT_MAX;
			int i_check=0;
			for(i=OSC_SLOT_MAX-1; i>=0; i--){//count
				if(i_check==0){
					if((ks->Osc[i].fmfrq==0)&&(ks->Osc[i].fmendfrq==0)&&(ks->Osc[i].fmamp==2.0f)
					&& (ks->Osc[i].fmtype==5)&& (ks->Osc[i].wavefrq==440)&&(ks->Osc[i].waveendfrq==440)
					&& (ks->Osc[i].waveamp==16383.0f)&&(ks->Osc[i].wavetype==0)){
						i_cnt--;
					}else{
						i_check=1;
					}
				}
			}
			fwrite((unsigned short*)&i_cnt, sizeof(unsigned short), 1, fp);
			for(i=0; i<i_cnt; i++){
				//osc
				fwrite(&ks->Osc[i].algorithm, sizeof(unsigned char), 1, fp);
				fwrite(&ks->Osc[i].wavetype, sizeof(unsigned char), 1, fp);
				fwrite(&ks->Osc[i].waveamp, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].wavefrq, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].waveendfrq, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].wavefine, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].fmtype, sizeof(unsigned char), 1, fp);
				fwrite(&ks->Osc[i].fmamp, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].fmfrq, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].fmendfrq, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].fmdetune, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].attacktime, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].releaselevel, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].releasetime, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].instvol, sizeof(float), 1, fp);
			}
			for(i=0; i<i_cnt; i++){
				//filter
				fwrite(&ks->Osc[i].delaytime, sizeof(long), 1, fp);
				fwrite(&ks->Osc[i].delaylevel, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].conductance, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].hipass, sizeof(float), 1, fp);
				fwrite(&ks->Osc[i].inductance, sizeof(float), 1, fp);
			}

			i_cnt = 0;
			for(i=0;i<TRACK_MAX; i++){
				int al;
				for(al=0; al<INST_SLOT_MAX; al++){
					if(ks->Inst[i][al].sequence!=NULL){
						i_cnt++;
						break;
					}
				}
			}
			fwrite((unsigned short*)&i_cnt, sizeof(unsigned short), 1, fp);
			for(i=0;i<TRACK_MAX; i++){
				int al;
				for(al=0; al<INST_SLOT_MAX; al++){
					if(ks->Inst[i][al].sequence!=NULL){
						fwrite((unsigned short*)&i, sizeof(unsigned short), 1, fp);
						fwrite((float*)&ks->charTime[i], sizeof(float), 1, fp);
						//fprintf(fp,"\tks->charTime[%d] = %ff;\n",i,ks2.charTime[i]);
						break;
					}
				}
			}


			i_cnt = 0;
			for(i=0;i<TRACK_MAX; i++){
				for(j=0;j<INST_SLOT_MAX; j++){
					int onum=-1;
					for(k=0; k<OSC_SLOT_MAX; k++){
						if((ks->Inst[i][j].Inst)==&(ks->Osc[k])) onum=k;
					}
					if(onum!=-1){
						i_cnt++;
					}
				}
			}
			fwrite((unsigned short*)&i_cnt, sizeof(unsigned short), 1, fp);
			for(i=0;i<TRACK_MAX; i++){
				for(j=0;j<INST_SLOT_MAX; j++){
					int onum=-1;
					for(k=0; k<OSC_SLOT_MAX; k++){
						if((ks->Inst[i][j].Inst)==&(ks->Osc[k])) onum=k;
					}
					if(onum!=-1){
						fwrite((unsigned char*)&i, sizeof(unsigned char), 1, fp);
						fwrite((unsigned char*)&j, sizeof(unsigned char), 1, fp);
						fwrite((unsigned char*)&onum, sizeof(unsigned char), 1, fp);
						fwrite(ks->Inst[i][j].sequence, sizeof(char)*(lstrlen(ks->Inst[i][j].sequence)+1), 1, fp);
						//fprintf(fp,"\tks->SetSequence(%d,%d,%d,\"%s\");\n",i,j,onum,ks2.Inst[i][j].sequence);
					}
				}
			}
		}else if(isKsInside==2){
			HANDLE hFile = CreateFile(this->music_file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				float fvol = (CS_VOLUME_RANGE + cs->GetVolume())/(float)CS_VOLUME_RANGE;
				fwrite(&fvol, sizeof(float), 1, fp);

				unsigned long dwSize = GetFileSize(hFile, NULL);
				fwrite((unsigned char*)&dwSize, sizeof(unsigned long), 1, fp);

				unsigned char pBuffer[4096];
				DWORD dwRead;
				for(unsigned long dwI=0; dwI<dwSize; ){
					ReadFile(hFile, pBuffer, sizeof(pBuffer), &dwRead, NULL);
					fwrite((unsigned char*)&pBuffer, dwRead, 1, fp);
					dwI+=dwRead;
				}
			CloseHandle(hFile);
		}

		//fout << "Eof";
		fclose(fp);
	}
	return true;
}

bool CDemo::Save(const char* filename)
{
	long i;
	ofstream fout(filename);
	if(fout.is_open()==0){
		MessageBox(NULL,"Can't save file","error",MB_OK);
		return false;
	}else{
		fout << "KScene Demo File" << endl;
		fout << "Format Text Ver 1.0" << endl;
		fout << "copyright kioku@System K" << endl << endl;

		fout << "KMD {" << endl;
			for(i=0; i<CDEMO_OBJECT_NUMMAX; i++){
				if(obj[i]!=NULL){
//<MakeRelative>
				BOOL isRelativePath = PathIsRelative(obj_name[i].c_str());
				if(!isRelativePath){
					char* szRelativeName = (char*)GlobalAlloc(GPTR, sizeof(char) * MAX_PATH);
					BOOL isSaveAsRelativePath;
					isSaveAsRelativePath = PathRelativePathTo(szRelativeName, filename, GetFileAttributes(filename), obj_name[i].c_str(), GetFileAttributes(obj_name[i].c_str()));
					if(isSaveAsRelativePath){//�֐����������đ��΃p�X��������ꂽ�瑊�΃p�X�ŕۑ�
						fout << "\t" << i << " \"" << szRelativeName << "\"" << endl;
					}else{//���s�������΃p�X�ŕۑ�����i�l�b�g���[�N�z���̕ۑ��͏o���Ȃ��C������̂ŏ����Ă����j
						fout << "\t" << i << " \"" << obj_name[i] << "\"" << endl;
					}
					GlobalFree(szRelativeName);
				}else{//���łɑ��΃p�X�ł���ꍇ�͐����������s��Ȃ�
					fout << "\t" << i << " \"" << obj_name[i] << "\"" << endl;
				}
//</MakeRelative>
				//fout << "\t" << i << " \"" << obj_name[i] << "\"" << endl;
				}
			}
		fout << "}" << endl;

//music
		if(this->music_file.size()>0){
			fout << "MUSIC {" <<endl;
				//volume
					KSynth* ks = this->ksl->GetKSynthPtr();
					fout << "\tVolume " << ((ks) ? ks->vol : (this->cs) ? ((CS_VOLUME_RANGE+this->cs->GetVolume())/(float)CS_VOLUME_RANGE) : 1.0f) << endl;
				//volume

				BOOL isRelativePath = PathIsRelative(this->music_file.c_str());
				if(!isRelativePath){
					char* szRelativeName = (char*)GlobalAlloc(GPTR, sizeof(char) * MAX_PATH);
					BOOL isSaveAsRelativePath;
					isSaveAsRelativePath = PathRelativePathTo(szRelativeName, filename, GetFileAttributes(filename), music_file.c_str(), GetFileAttributes(music_file.c_str()));
					if(isSaveAsRelativePath){//�֐����������đ��΃p�X��������ꂽ�瑊�΃p�X�ŕۑ�
						fout << "\t" << " \"" << szRelativeName << "\"" << endl;
					}else{//���s�������΃p�X�ŕۑ�����i�l�b�g���[�N�z���̕ۑ��͏o���Ȃ��C������̂ŏ����Ă����j
						fout << "\t" << " \"" << music_file << "\"" << endl;
					}
					GlobalFree(szRelativeName);
				}else{//���łɑ��΃p�X�ł���ꍇ�͐����������s��Ȃ�
					fout << "\t" << i << " \"" << music_file << "\"" << endl;
				}
			fout << "}" <<endl;
		}
//music
		vector<CScene>::iterator sit,seit=scene.end();
		for(sit=scene.begin(); sit!=seit; sit++){
			fout << "Scene \"" << sit->scenename << "\" {" << endl;
//scene_track
			fout << "\tSceneTrack \"" << sit->scene_track << "\"" << endl;
//scene_track
//scene_time
			fout << "\tSceneTime " << sit->fscene_time << endl;
//scene_time			
			vector<CSceneObject>::iterator oit,oeit=sit->sceneobj.end();
			for(oit=sit->sceneobj.begin(); oit!=oeit; oit++){
				fout << "\tObject " << oit->model_num << " {" << endl;
					fout << "\t\tCameraTrans " << oit->is_cameratrans << endl;
//interpolate
					fout << "\t\tInterpolate " << oit->interpolate << endl;
//interpolate
					fout << "\t\tVisible " << oit->is_visible << endl;
					fout << "\t\tDataNum " << (long)oit->anim.anim_time.size() << endl;
					fout << "\t\tTime {" << endl;
						vector<float>::iterator tit,teit=oit->anim.anim_time.end();
						for(tit=oit->anim.anim_time.begin(); tit!=teit; tit++){
							fout << "\t\t\t" << (*tit) << endl;
						}
					fout << "\t\t}" << endl;//Time
					if(oit->model!=NULL){
						long i,obj_n=oit->model->GetCloneAllocNum();
						for(i=0; i<obj_n; i++){
							KClone* kcn = oit->model->GetCloneAllocPtr ();
							string objname = kcn[i].clone_data.clone_name;
							fout << "\t\tPart \""<< objname << "\" {" << endl;
								long t,tmax=(long)oit->anim.anim_time.size();
								long rs;
								//--------------Pos---------------------------------------
								rs=0;
								for(t=0; t<tmax; t++){
									CVector* p = &oit->anim.anim[i][t].pos;
									if((p->x!=0.0f)||(p->y!=0.0f)||(p->z!=0.0f)) rs++;
								}
								if(rs>0){
									fout << "\t\t\tPosition {" << endl;
									for(t=0; t<tmax; t++){
										CVector* p = &oit->anim.anim[i][t].pos;
										fout << "\t\t\t\t" << p->x << " "<< p->y << " "<< p->z << endl;
									}
									fout << "\t\t\t}" << endl;
								}
								//---------------Rot---------------------------------------
								rs=0;
								for(t=0; t<tmax; t++){
									CVector* p = &oit->anim.anim[i][t].rot;
									if((p->x!=0.0f)||(p->y!=0.0f)||(p->z!=0.0f)) rs++;
								}
								if(rs>0){
									fout << "\t\t\tRotation {" << endl;
									for(t=0; t<tmax; t++){
										CVector* p = &oit->anim.anim[i][t].rot;
										fout << "\t\t\t\t" << p->x << " "<< p->y << " "<< p->z << endl;
									}
									fout << "\t\t\t}" << endl;
								}
								//---------------Scale-------------------------------------
								rs=0;
								for(t=0; t<tmax; t++){
									CVector* p = &oit->anim.anim[i][t].scale;
									if((p->x!=1.0f)||(p->y!=1.0f)||(p->z!=1.0f)) rs++;
								}
								if(rs>0){
									fout << "\t\t\tScale {" << endl;
									for(t=0; t<tmax; t++){
										CVector* p = &oit->anim.anim[i][t].scale;
										fout << "\t\t\t\t" << p->x << " "<< p->y << " "<< p->z << endl;
									}
									fout << "\t\t\t}" << endl;
								}
								//--------------Alpha--------------------------------------
								rs=0;
								for(t=0; t<tmax; t++){
									if(oit->anim.anim[i][t].alpha!=1.0f) rs++;
								}
								if(rs>0){
									fout << "\t\t\tAlpha {" << endl;
									for(t=0; t<tmax; t++){
										fout << "\t\t\t\t" << oit->anim.anim[i][t].alpha << endl;
									}
									fout << "\t\t\t}" << endl;
								}
								//---------------------------------------------------------
							fout << "\t\t}" << endl;//Part
						}
					}
				fout << "\t}" << endl;//Object
			}
			fout << "}" << endl;//Scene
		}
		fout << "Eof";
		fout.close();
	}
	return true;
}

bool CDemo::LoadMusic(const char* szFilename){
	if(szFilename==NULL || szFilename[0]=='\0'){
		return FALSE;
	}

	KSynthLoader* oldksl = this->ksl;
	this->ksl = new KSynthLoader();
#ifdef NOT64K
	//CSound* oldcs = this->cs;
	//this->cs = new CSound();
#endif

#ifdef NOT64K
	if(this->ksl->LoadKSF(szFilename, NULL, 0) || this->cs->Load(szFilename)){
#else
	if(this->ksl->LoadKSF(szFilename, NULL, 0)){
#endif
		this->music_file = szFilename;

		delete oldksl;
#ifdef NOT64K
		//delete oldcs;
#endif
	}else{
		//char str[512];
		//wsprintf(str, "CDemo::LoadMusic()��\n%s\n���J�����Ƃ��ł��܂���ł���.", szFilename);
		//MessageBox(NULL, str, 0, MB_SYSTEMMODAL);
		//this->music_file = "";
		//this->music_file.clear();
		this->ksl = oldksl;
#ifdef NOT64K
		//this->cs  = oldcs;
#endif
		return FALSE;
	}
	return TRUE;
}

bool CDemo::Load(const char* filename)
{
	if(Open(filename)==1){
		return false;
	}else{
		//all data clear
		scene.clear();
		scene_endtime.clear();
		long i;
		for(i=0; i<CDEMO_OBJECT_NUMMAX; i++) obj[i]=NULL;

		//data load
		string buf;
		do{
			buf = GetData();
			if(buf=="KMD"){//�I�u�W�F�N�g���[�h
				CheckPhase();
				string sb;
				sb = GetData();
				while(sb!="}"){
					long n_object = atol(sb.c_str());
					string object_name = GetData();
					while(object_name[object_name.size()-1]!='\"') object_name += GetData();
					object_name.erase(0,1);//delete first"
					object_name.erase (object_name.find_last_not_of("\"")+1,1);//delete last "
//<MakeRelative>
					BOOL isRelativePath = PathIsRelative(object_name.c_str());
					if(!isRelativePath){//��΃p�X�ŕۑ�����Ă���ꍇ�͑f���ɓǂ�

						////�d�l�̌Â�KDF�ł������ꍇ�̂��߂ɑ��΃p�X�ɂ��Ă݂�
						//char* szRelativeName = (char*)GlobalAlloc(GPTR, sizeof(char) * MAX_PATH);
						//BOOL isMadeRelativePath;
						//isMadeRelativePath = PathRelativePathTo(szRelativeName, filename, GetFileAttributes(filename), object_name.c_str(), GetFileAttributes(object_name.c_str()));
						//if(isMadeRelativePath){//���΃p�X���������ꂽ
						//	object_name = szRelativeName;
						//}else{
						//	//nop
						//}
						//GlobalFree(szRelativeName);
					}else{
						LONG soFullName = sizeof(char) * MAX_PATH * 2;//�p�X��255�ȏ�̏ꍇ�����肦��̂Ŕ{�m�ۂ��Ă����B
						char* szFullName = (char*)GlobalAlloc(GPTR, soFullName);

						//�t���p�X���T�[�`����ۂ̓J�����g�f�B���N�g�����猟������邽�߁AKDF�����݂���f�B���N�g�����ꎞ�I�ɃJ�����g�f�B���N�g���Ƃ���B
						char* szOldDir = (char*)GlobalAlloc(GPTR, sizeof(char) * MAX_PATH);
						char* szNewDir = (char*)GlobalAlloc(GPTR, sizeof(char)*(lstrlen(filename)+1));
						GetCurrentDirectory(MAX_PATH, szOldDir);
							lstrcpy(szNewDir, filename);
							PathRemoveFileSpec(szNewDir);
							SetCurrentDirectory(szNewDir);
								PathSearchAndQualify(object_name.c_str(), szFullName, soFullName);
							SetCurrentDirectory(szOldDir);
						GlobalFree(szNewDir);
						GlobalFree(szOldDir);

						object_name = szFullName;//LoadObject(n_object, szFullName);
						GlobalFree(szFullName);
					}
					//__try{
						if(-1==LoadObject(n_object, object_name)){
							//char str[512];
							//wsprintf(str, "���L�̃t�@�C���̃I�[�v���Ɏ��s���܂���.�t�@�C���������������ǂ����C���̃A�v���P�[�V�����Ŏg�p���łȂ����m�F���Ă�������.\n%s",filename.c_str());
							//MessageBox(NULL, str, "kmb�ǂݎ��G���[", MB_SYSTEMMODAL);
						}
					//}
					//__except(EXCEPTION_EXECUTE_HANDLER)
					//{
					//}
//</MakeRelative>
					//LoadObject(n_object,object_name);
					sb = GetData();
				}
			}

			if(buf=="MUSIC"){
				CheckPhase();

				buf = GetData();
				float volume = 1.00f;
				if(buf=="Volume"){
					volume = atof(GetData().c_str());
					buf = GetData();//next
				}

				string music_file = buf;
				while(music_file[music_file.size()-1]!='\"') music_file += GetData();
				music_file.erase(0,1);//delete first"
				music_file.erase (music_file.find_last_not_of("\"")+1,1);//delete last "
				BOOL isRelativePath = PathIsRelative(music_file.c_str());
				if(!isRelativePath){//��΃p�X�ŕۑ�����Ă���ꍇ�͑f���ɓǂ�
					this->music_file = music_file;
				}else{
					LONG soFullName = sizeof(char) * MAX_PATH * 2;//�p�X��255�ȏ�̏ꍇ�����肦��̂Ŕ{�m�ۂ��Ă����B
					char* szFullName = (char*)GlobalAlloc(GPTR, soFullName);

					//�t���p�X���T�[�`����ۂ̓J�����g�f�B���N�g�����猟������邽�߁AKDF�����݂���f�B���N�g�����ꎞ�I�ɃJ�����g�f�B���N�g���Ƃ���B
					char* szOldDir = (char*)GlobalAlloc(GPTR, sizeof(char) * MAX_PATH);
					char* szNewDir = (char*)GlobalAlloc(GPTR, sizeof(char)*(lstrlen(filename)+1));
					GetCurrentDirectory(MAX_PATH, szOldDir);
						lstrcpy(szNewDir, filename);
						PathRemoveFileSpec(szNewDir);
						SetCurrentDirectory(szNewDir);
							PathSearchAndQualify(music_file.c_str(), szFullName, soFullName);
						SetCurrentDirectory(szOldDir);
					GlobalFree(szNewDir);
					GlobalFree(szOldDir);

					this->music_file = szFullName;
					GlobalFree(szFullName);
				}
				if(lstrlen(this->music_file.c_str())>0){
					if(this->LoadMusic(this->music_file.c_str())){
						KSynth* ks = this->ksl->GetKSynthPtr();
						if( ks ){
							ks->vol = volume;
						}
						if( cs ){
							cs->SetVolume(CS_VOLUME_FIX_SET(-(CS_VOLUME_RANGE - volume*CS_VOLUME_RANGE)));
						}
					}
				}

				CheckPhase();
				buf = GetData();//next
			}

			if(buf=="Scene"){//�V�[�����
				CScene scn(anim_buffer);
				string scene_name = GetData();
				while(scene_name[scene_name.size()-1]!='\"') scene_name += GetData();
				scene_name.erase(0,1);//delete first"
				scene_name.erase (scene_name.find_last_not_of("\"")+1,1);//delete last "
				scn.scenename = scene_name;
				CheckPhase();
				buf=GetData();
//scene_track
				if(buf=="SceneTrack"){
					string scene_track = GetData();
					while(scene_track[scene_track.size()-1]!='\"') scene_track += GetData();
					scene_track.erase(0,1);//delete first"
					scene_track.erase (scene_track.find_last_not_of("\"")+1,1);//delete last "
					scn.scene_track = scene_track;
					buf = GetData();//next
				}
//scene_track
//scene_time
				if(buf=="SceneTime"){
					float fSceneTime = atof(GetData().c_str());
					scn.fscene_time = fSceneTime;
					buf = GetData();//next
				}
//scene_time
				while(buf=="Object"){
					long data_num=0;//BoneTrans num
					long n_obj = atol(GetData().c_str());
					CSceneObject* scobj;
					{
						CSceneObject sco;
						scn.sceneobj.push_back(sco);//�V�[���I�u�W�F�N�g�ǉ�
						scobj = &scn.sceneobj[(long)scn.sceneobj.size()-1];
					}
					if(n_obj==-1) scobj->model = camera_ptr;//camera
					else		  scobj->model = obj[n_obj];
					scobj->model_num=n_obj;
					CheckPhase();
					buf = GetData();
					if(buf=="CameraTrans"){
						scobj->is_cameratrans = atol(GetData().c_str());
						buf = GetData();//next
					}
//interpolate
					if(buf=="Interpolate"){
						scobj->interpolate = atol(GetData().c_str());
						buf = GetData();//next
					}
//interpolate

					if(buf=="Visible"){
						scobj->is_visible = atol(GetData().c_str());
						buf = GetData();//next
					}

					if(buf=="DataNum"){
						data_num = atol(GetData().c_str());
						buf = GetData();//next

						//create init BoneTrans data
						long i,j,obj_n=(scobj->model!=NULL) ? scobj->model->GetCloneAllocNum() : 0;
						scobj->anim.anim.resize(obj_n);
						for(i=0; i<obj_n; i++){
							// -------------------------------------------------motion clone init
							KCloneData bt;
							bt.alpha = 1.0f;
							bt.scale = CVector(1,1,1);
							bt.lock = 0;
							for(j=0; j<data_num; j++){
								scobj->anim.anim[i].push_back(bt);
							}
						}
					}
					
					if(buf=="Time"){
						CheckPhase();
						long i;
						for(i=0; i<data_num; i++){
							scobj->anim.anim_time.push_back((float)atof(GetData().c_str()));
						}
						CheckPhase();
						buf = GetData();//next
					}//Time
					
					long cln_i=-1;
					while(buf=="Part"){
						string dname=GetData();
						while(dname[dname.size()-1]!='\"') dname += GetData();
						dname.erase(0,1);//delete first"
						dname.erase (dname.find_last_not_of("\"")+1,1);//delete last "
						cln_i++;
						if(scobj->anim.anim.size()==0){//�I�u�W�F�N�g�ɂȂ�Part
							SkipChank();
						}else{
							CheckPhase();
							buf=GetData();
							if(buf=="Position"){
								CheckPhase();
								long d;
								for(d=0; d<data_num; d++){
									CVector pos;
									pos.x = (float)atof(GetData().c_str());
									pos.y = (float)atof(GetData().c_str());
									pos.z = (float)atof(GetData().c_str());
									scobj->anim.anim[cln_i][d].pos = pos;
								}
								CheckPhase();
								buf=GetData();
							}
							if(buf=="Rotation"){
								CheckPhase();
								long d;
								for(d=0; d<data_num; d++){
									CVector rot;
									rot.x = (float)atof(GetData().c_str());
									rot.y = (float)atof(GetData().c_str());
									rot.z = (float)atof(GetData().c_str());
									scobj->anim.anim[cln_i][d].rot = rot;
								}
								CheckPhase();
								buf=GetData();
							}
							if(buf=="Scale"){
								CheckPhase();
								long d;
								for(d=0; d<data_num; d++){
									CVector scale;
									scale.x = (float)atof(GetData().c_str());
									scale.y = (float)atof(GetData().c_str());
									scale.z = (float)atof(GetData().c_str());
									scobj->anim.anim[cln_i][d].scale = scale;
								}
								CheckPhase();
								buf=GetData();
							}
							if(buf=="Alpha"){
								CheckPhase();
								long d;
								for(d=0; d<data_num; d++){
									CVector pos;
									scobj->anim.anim[cln_i][d].alpha = (float)atof(GetData().c_str());
								}
								CheckPhase();
								buf=GetData();
							}
						}//anim.size()>0
						buf = GetData();
						if(buf=="}") buf = GetData();//end of object
					}//Part
				}//object
				scene.push_back(scn);
			}//Scene
			if(buf=="Eof") break;//EOF
		}while(1);
		Close();
	}//Open()==1
	return true;
}

void DrawMultiTexturePolygon(float x, float y, float width, float height, int nTexture){
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

void ActivateMultiTexture(int nTexture, unsigned int* image, unsigned int* rgb_op, unsigned int src_rgb[][3], unsigned int ope_rgb[][3], unsigned int* alpha_op, unsigned int src_alp[][3], unsigned int ope_alp[][3]){
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

void DeActivateMultiTexture(int nTexture){
	int i;
	for(i=nTexture-1; i>=0; i--){
		glActiveTextureARB(GL_TEXTURE0_ARB + i);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

void CDemo::RenderScene(long sc, float scenetime, long cameramode){
	if(cameramode==0){
		//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);//������glClear������ƁC���b�V�����`�悳��Ȃ��B���Ă������C���܂ŕ`�悳��Ă������̂����ׂĖ��ʂɂȂ�B
			this->scene[sc].RenderScene(scenetime, cameramode, 0, 0);
	}else{
		KCloneData clk = scene[sc].sceneobj[0].anim.GetBoneTrans(1, scenetime, scene[sc].sceneobj[0].interpolate);
		CVector lookat_rot = clk.rot;

		if(lookat_rot.x<=0.0f && lookat_rot.y<=0.0f && lookat_rot.z<=0.0f){	//��ʊE�k�x�̃p�����[�^�����Ɏw�肳��Ă��Ȃ��Ƃ��́CDOF�̖��p�ȉ��Z�͔�����ׂ����B
			//this->RenderScene(sc, scenetime, 0); return;
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			this->scene[sc].RenderScene(scenetime, cameramode, 0);
			return ;
		}

		//
		//DOF�̉��Z�͈ȉ��̂Ƃ���
		//

		//RenderToTexture�̉e�����󂯂�O�̃r���[�|�[�g.
		int original_viewport[4];
		glGetIntegerv(GL_VIEWPORT, original_viewport);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		//DepthOfField[10].RenderStart();
		//	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		//	this->scene[sc].RenderScene(scenetime, cameramode, 0, 2);//�Ō��, is_cameratrans==2�̃I�u�W�F�N�g����DOF�����ŕ`��
		//DepthOfField[10].RenderEnd();

		DepthOfField[0].RenderStart();//�^�����e�N�X�`��
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		DepthOfField[0].RenderEnd();

		DepthOfField[6].RenderStart();//��O���ڂ������߂̃e�N�X�`��(��O������D��������)
		{
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
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
				this->scene[sc].RenderScene(scenetime, cameramode, 2, 0);
				//this->scene[sc].RenderScene(scenetime, cameramode, 2, 2);
				glEnable(GL_LIGHTING);
				//glEna
			glDisable(GL_FOG);
		}
		DepthOfField[6].RenderEnd();

		DepthOfField[1].RenderStart();//�����ڂ������߂̃e�N�X�`��(��O�����D������)
		{
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
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
				this->scene[sc].RenderScene(scenetime, cameramode, 1, 0);
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

			int nTexture = 2;
			unsigned int image[] = {DepthOfField[1].GetTextureNum(), DepthOfField[6].GetTextureNum()};

			unsigned int rgb_op[] = {GL_REPLACE, GL_ADD};
			unsigned int alp_op[] = {GL_REPLACE, GL_REPLACE};
			unsigned int src_rgb_param[][3] = {
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
			};
			unsigned int ope_rgb_param[][3] = {
				{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA},
				{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA}
			};
			unsigned int src_alpha_param[][3] = {
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
			};
			unsigned int ope_alpha_param[][3] = {
				{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA},
				{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA}
			};
			glPushAttrib(GL_TEXTURE_BIT);
				ActivateMultiTexture(nTexture, image, rgb_op, src_rgb_param, ope_rgb_param, alp_op, src_alpha_param, ope_alpha_param);
			
				int x=0, y=0;
				DepthOfField[9].ViewOrtho(original_viewport[2],original_viewport[3]);
			
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				DrawMultiTexturePolygon(x, y, original_viewport[2], original_viewport[3], nTexture);

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

		DepthOfField[2].RenderStart();//����
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
				this->scene[sc].RenderScene(scenetime, cameramode, 0, 0);
				//this->scene[sc].RenderScene(scenetime, cameramode, 0, 2);
		DepthOfField[2].RenderEnd();

		DepthOfField[7].RenderStart();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			this->scene[sc].RenderScene(scenetime, cameramode, 0, 0);
		DepthOfField[7].RenderEnd();
		DepthOfField[8].RenderStart();
		DepthOfField[8].RenderEnd();

		//DepthOfField[3].RenderStart();//��𑜓x
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
				if(lookat_rot.x<=0.0f) lookat_rot.x = 0.0f;
				int nRadius = 1;
				nRadius += (lookat_rot.x/30.0f);
				int nOffset = 1;
				glEnable(GL_TEXTURE_2D);

				glScreen* pTextureIn = &DepthOfField[8], *pTextureOut = &DepthOfField[7];//&DepthOfField[8];
				while(nOffset <= nRadius){//nOffSet�̓s�N�Z���P��.
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
		
			DepthOfField[3].RenderStart();//��𑜓x
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				pTextureOut->DrawScreen(0.0f, 0.0f);
			DepthOfField[3].RenderEnd();
			glEnable(GL_BLEND);
		}

		DepthOfField[4].RenderStart();//[1]*[3] : ��O���Â��A�������邢
		{
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_LIGHTING);
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_CULL_FACE);

			int nTexture = 2;
			unsigned int image[] = {DepthOfField[9].GetTextureNum() ,DepthOfField[3].GetTextureNum() };

			unsigned int rgb_op[] = {GL_REPLACE, GL_MODULATE};//�}���`�e�N�X�`���p�I�y���[�^
			unsigned int alp_op[] = {GL_REPLACE, GL_REPLACE};//�}���`�e�N�X�`���p�I�y���[�^
			unsigned int src_rgb_param[][3] = {
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
			};
			unsigned int ope_rgb_param[][3] = {
				{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA},
				{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA}
			};
			unsigned int src_alpha_param[][3] = {
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
			};
			unsigned int ope_alpha_param[][3] = {
				{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA},
				{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA}
			};
			glPushAttrib(GL_TEXTURE_BIT);
				ActivateMultiTexture(nTexture, image, rgb_op, src_rgb_param, ope_rgb_param, alp_op, src_alpha_param, ope_alpha_param);

				int x=0, y=0;
				DepthOfField[4].ViewOrtho(original_viewport[2],original_viewport[3]);

				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				DrawMultiTexturePolygon(x, y, original_viewport[2], original_viewport[3], nTexture);
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

			int nTexture = 3;
			unsigned int image[] = {DepthOfField[9].GetTextureNum(),
									DepthOfField[2].GetTextureNum(),
									DepthOfField[4].GetTextureNum()};

			unsigned int rgb_op[] = {GL_REPLACE, GL_MODULATE, GL_ADD};//�}���`�e�N�X�`���p�I�y���[�^
			unsigned int alp_op[] = {GL_REPLACE, GL_REPLACE, GL_REPLACE};//�}���`�e�N�X�`���p�I�y���[�^
			unsigned int src_rgb_param[][3] = {
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
			};
			unsigned int ope_rgb_param[][3] = {
				{GL_ONE_MINUS_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA},
				{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA},
				{GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA}
			};
			unsigned int src_alpha_param[][3] = {
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB},
				{GL_TEXTURE, GL_PREVIOUS_ARB, GL_CONSTANT_ARB}
			};
			unsigned int ope_alpha_param[][3] = {
				{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA},
				{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA},
				{GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA}
			};
			glPushAttrib(GL_TEXTURE_BIT);
				ActivateMultiTexture(nTexture, image, rgb_op, src_rgb_param, ope_rgb_param, alp_op, src_alpha_param, ope_alpha_param);
			
				int x=0, y=0;
				DepthOfField[5].ViewOrtho(original_viewport[2],original_viewport[3]);
			
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				DrawMultiTexturePolygon(x, y, original_viewport[2], original_viewport[3], nTexture);

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
		
		BOOL isFound = FALSE;
		int nFound = -1;
		int n = sizeof(DepthOfField)/sizeof(glScreen);
		for(int i=0; i<n; i++){
			if((GetAsyncKeyState('0'+i)&0x8000)==0x8000){
				nFound = i;
				isFound = TRUE;
			}
		}
		if((GetAsyncKeyState('F')&0x8000)==0x8000){
			nFound = 9;
			isFound = TRUE;
		}
		if(!isFound){
			DepthOfField[5].DrawScreen(0.0f, 0.0f);
			this->scene[sc].RenderScene(scenetime, cameramode, 0, 2);
		}else{
			DepthOfField[nFound].DrawScreen(0.0f, 0.0f);
		}
	}
}


int CDemo::LoadObject(long object_num, string filename)
{
	obj_name[object_num] = filename;
	obj[object_num] = new KModelEdit();
	
	obj[object_num]->Create();
	//�f�[�^���[�h
	int error = 0;
	if(-1==obj[object_num]->LoadFromFile(filename.c_str())){
		char str[512];
		wsprintf(str, "���L�̃t�@�C���̃I�[�v���Ɏ��s���܂���.\n�t�@�C���������������ǂ����C���̃A�v���P�[�V�����Ŏg�p���łȂ����m�F���Ă�������.\n%s",filename.c_str());
		MessageBox(NULL, str, "fopen�G���[", MB_SYSTEMMODAL);
		error = -1;
	}
	if(error!=0){
		obj_name[object_num].clear();
		obj[object_num]->Free();
		delete obj[object_num];
		obj[object_num] = NULL;
	}
	return error;
}

void CDemo::FreePrimitive(long object_num)
{
	long i = object_num;
    if((i>=0)&&(i<CDEMO_OBJECT_NUMMAX)){
		if(obj[i]!=NULL){
			obj[i]->Free();
			obj[i] = NULL;
			obj_name[i] = "";
		}
	}
}

KModelEdit* CDemo::GetCameraPtr()
{
	return camera_ptr;
}