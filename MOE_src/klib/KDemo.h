#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "kmodel.h"
#include "KAnimation.h"
#include "ks2lib.h"
#include "glScreen.h"
#include "../GL/glext.h"

#ifdef NOT64K
	#include "../clibrary/csound.h"
#endif

#define CDEMO_OBJECT_NUMMAX		256
#define DOF_TEXTURE_NUM			11

//Object Class
class KSceneObject{
	public:
		KModel* model;
		long model_num;
		KAnimation anim;
		long is_cameratrans;
		long interpolate;
		long is_visible;
		KSceneObject()
		{
			model_num=-1;
			model=NULL;
			is_cameratrans=1;
			is_visible=1;
			interpolate = 0;
		}
		KSceneObject(const KSceneObject &sceneobj_obj)
		{
			anim = sceneobj_obj.anim;
			model = sceneobj_obj.model;
			is_cameratrans = sceneobj_obj.is_cameratrans;
			is_visible = sceneobj_obj.is_visible;
			model_num = sceneobj_obj.model_num;
		}
		
};

class KScene{
	private:
		GLint npViewport[4];
		KCloneData* anim_data;
	public:
		//string scene_track;
		float fscene_time;
		//string scenename;
		KSceneObject* sceneobj;
		long sceneobj_num;
		void RenderScene(float scenetime, int nomat=0, int drawis_cameratrans2=1);
		void Init();
		KScene(const KScene &scene_obj);
};


class KDemo{
	private:
		KModel* camera_ptr;
		float ldpersentage;
		glScreen* DepthOfField;//size == DOF_TEXTURE_NUM;
		DWORD dwStartTime;
	public:
		KSynth* ks;
#ifdef NOT64K
		CSound* cs;
#endif
		KModel* obj[CDEMO_OBJECT_NUMMAX];
		//string obj_name[CDEMO_OBJECT_NUMMAX];
		KScene* scene;
		float*  scene_endtime;
		long scene_num;
		int RenderDemo();
		void RenderScene(long sc, float scenetime);
		void Play();
		void ReadyDemo();
		bool Load(const unsigned char* dptr, void (*Loader)(float), bool highmode);
		float GetLoadPersentage();
		KDemo();

		void DrawMultiTexturePolygon(float x, float y, float width, float height, int nTexture);
		void ActivateMultiTexture(const int nTexture, const unsigned int* image, const unsigned int* rgb_op, const unsigned int src_rgb[][3], const unsigned int ope_rgb[][3], const unsigned int* alpha_op, const unsigned int src_alp[][3], const unsigned int ope_alp[][3]);
		void DeActivateMultiTexture(int nTexture);

		void SeekRelative(long seek_time_ms);
		void SetVolume(float fVolume);
		float GetVolume();

		bool highMode;
};