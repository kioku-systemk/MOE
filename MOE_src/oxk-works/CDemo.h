#pragma once

#include "../kLib/kmodeledit.h"
#include "CAnimation.h"
#include "../klib/glScreen.h"
#include "../clibrary/csound.h"
#include "sync.h"

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
using namespace std;

#define CDEMO_OBJECT_NUMMAX		256

//Object Class
class CSceneObject{
	public:
		KModelEdit* model;
		long model_num;
		CAnimation anim;
		long is_cameratrans;
		long interpolate;
		long is_visible;

		CSceneObject()
		{
			model_num=-1;
			model=NULL;
			is_cameratrans=1;
			is_visible=1;
			interpolate = 0;
		}
		CSceneObject(const CSceneObject &sceneobj_obj)
		{
			anim = sceneobj_obj.anim;
			model = sceneobj_obj.model;
			is_cameratrans = sceneobj_obj.is_cameratrans;
			is_visible = sceneobj_obj.is_visible;
			model_num = sceneobj_obj.model_num;
			interpolate = sceneobj_obj.interpolate;
		}
		CSceneObject& operator=(const CSceneObject &sceneobj_obj)
		{
			anim = sceneobj_obj.anim;
			model = sceneobj_obj.model;
			is_cameratrans = sceneobj_obj.is_cameratrans;
			is_visible = sceneobj_obj.is_visible;
			model_num = sceneobj_obj.model_num;
			interpolate = sceneobj_obj.interpolate;
			return *this;	
		}
		~CSceneObject()
		{
		}

		void CreateKeyFrame(float frame_time)
		{
			if(anim.anim_time.size()!=(long)(find(anim.anim_time.begin(),anim.anim_time.end(),frame_time) - anim.anim_time.begin()))
			{
				return;//すでに指定のキーフレームがある
			}
			if(this->model!=NULL){
				long i,obj_n=model->GetCloneAllocNum();
				KCloneData* ct = (KCloneData*)GlobalAlloc(GPTR,sizeof(KCloneData)*obj_n);
				for(i=0; i<obj_n; i++){
					ct[i] = anim.GetBoneTrans(i,frame_time,this->interpolate);
				}

				anim.anim_time.push_back(frame_time);
				sort(anim.anim_time.begin(),anim.anim_time.end());
				long n=(long)(find(anim.anim_time.begin(),anim.anim_time.end(),frame_time) - anim.anim_time.begin());

				KCloneData bt;
				bt.alpha = 1.0f;
				bt.scale = CVector(1,1,1);
				for(i=0; i<obj_n; i++){
					anim.anim[i].insert(anim.anim[i].begin()+n,ct[i]);
				}
				GlobalFree(ct);			
			}
		}
		void DeleteKeyFrame(float frame_time)
		{
			if((frame_time==0.0f)||(frame_time==1.0f)) return; //cant delete

			vector<float>::iterator tit,teit=anim.anim_time.end();
			for(tit=anim.anim_time.begin(); tit!=teit; tit++){
				if(frame_time==(*tit)){
					if(this->model!=NULL){
						//anim.anim_time.erase(tit);
						long cn = model->GetCloneAllocNum();
						long i;
						for(i=0; i<cn; i++){
							long n = (long)(tit - anim.anim_time.begin());
							anim.anim[i].erase(anim.anim[i].begin()+n);
						}
						anim.anim_time.erase(tit);
						break;
					}
				}
			}
		}
};

class CScene{
	private:
		KCloneData* anim_data;
	public:
		string scene_track;
		float fscene_time;
		string scenename;
		vector<CSceneObject> sceneobj;
		//dof_far = 1
		//dor_near = 2
		void RenderScene(float scenetime, int camera_mode, int nomat=0, int drawis_cameratrans2=1);//dof_far == nomat_as_black, dof_near == nomat_as_white
		CScene(KCloneData* anim_buffer);
		~CScene();
		CScene(const CScene &scene_obj);
		CScene &operator=(const CScene &scene_obj);
};


class CDemo : private CMQO_File{
	private:
		KModelEdit* camera_ptr;
		static glScreen DepthOfField[11];
	public:
		static KSynthLoader* ksl;
		static CSound* cs;

		void InitSoundSystem(HWND hWnd){
#ifdef NOT64K
			cs->Initialize(hWnd);
#endif
		}

		KCloneData* anim_buffer;
		KModelEdit* obj[CDEMO_OBJECT_NUMMAX];
		string obj_name[CDEMO_OBJECT_NUMMAX];
		vector<CScene> scene;
		vector<float> scene_endtime;
		long offsettime;
		KModelEdit* GetCameraPtr();
		void ReadyDemo();
		int RenderDemo(float demotime_ms, float* fRecvCurrentRate = NULL);
		void RenderScene(long sc, float scenetime, long cameramode);
		bool BinarySave(const char* filename);
		bool Load(const char* filename);
		bool Save(const char* filename);
		int LoadObject(long object_number, string filename);
		void FreePrimitive(long object_num);
		CDemo(KModelEdit* camera);
		~CDemo();
		CDemo(const CDemo &dobj);
		void CopyPrimitive(const CDemo &dobj);
		void CreateDOFTexture();
		bool LoadMusic(const char* szFilename);

		string music_file;
};