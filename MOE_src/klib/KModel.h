#pragma once
/*
	KModel Lib(for 64k intro)
	coded by kioku 2005
	last modified 2005/6/5
*/
#ifndef __KMODEL__
#define __KMODEL__

#include "vector.h"
#include "KObject.h"
#include <windows.h>
#include <gl/gl.h>
#include "ktexture.h"

#define NULL							0
#define KMD_MATERIAL_NUM				256
#define KMD_PRIMITIVE_NUM				256
#define KMD_MATERIAL_RESERVE_VER0		30
#define KMD_MATERIAL_RESERVE_VER1		8

//------------------------------------------------------------------
/*
	              < class structure >

	KModel
	   |-----------------------------------------
	   |                    |                    |
	[Material]			[Clone]				[Primitive]
	KMaterial			KClone				  KObject
	   |                    |                    |
	 KRGBA              KCloneData            
*/
//------------------------------------------------------------------


//-----------Material interface------------------------------
class KRGBA
{
	public:
		float r;
		float g;
		float b;
		float a;
};

//SourceFactor : mat->shade&0x0F
//DestinationFactor : mat->shade>>4
//glBlendFunc(nBlendFactorList[mat->shade&0x0F], nBlendFactorList[mat->shade>>4]); 
const unsigned long nBlendFactorList[] = { GL_ZERO, GL_ONE, GL_DST_COLOR, GL_SRC_COLOR, GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA_SATURATE };

class KMultiTextureEnv{
public:
	unsigned char	op;			//GL_RELPACE, GL_ADD, GL_ADD_SIGNED, ...
	unsigned char	source_param[3];	//RGB GL_PRIMARY_COLOR_ARB, GL_TEXTURE, ...
	unsigned char	operand_param[3];	//RGB SRC_COSRC_ALPHA, ONE_MINUS_SRC_ALPHA
	unsigned char	fscale;				//1.0, 2.0, 4.0

	//unsigned int	operator_rgb;			//GL_RELPACE, GL_ADD, GL_ADD_SIGNED, ...
	//unsigned int	operator_alpha;			//GL_RELPACE, GL_ADD, GL_ADD_SIGNED, ...

	//unsigned int	source_rgb_param[3];	//RGB GL_PRIMARY_COLOR_ARB, GL_TEXTURE, ...
	//unsigned int	source_alpha_param[3];	//ALPHA GL_PRIMARY_COLOR_ARB, GL_TEXTURE, ...

	//unsigned int	operand_rgb_param[3];	//RGB SRC_COSRC_ALPHA, ONE_MINUS_SRC_ALPHA
	//unsigned int	operand_alpha_param[3];	//ALPHA SRC_COSRC_ALPHA, ONE_MINUS_SRC_ALPHA

	//float			fscale_rgb;				//1.0, 2.0, 4.0
	//float			fscale_alpha;			//1.0, 2.0, 4.0
};

class KShader{
public:
	char* ps;
	char* vs;
};

class KMaterial{
	public:
		KRGBA color;//4
		char** texture;//var
		unsigned int gltexure_num;

		unsigned char	number_of_texture;
		unsigned int*	texture_id;
		KMultiTextureEnv*	multi_texture_env;

		char mat_name[8];
		unsigned char subdivide;//1
		unsigned char shade;//1
		unsigned char* texenv;//1
		CVector uv_trans;
		CVector uv_rot;
		CVector uv_scale;
		unsigned char blendf;
		KShader shader;

		unsigned char reserve[KMD_MATERIAL_RESERVE_VER1];
};
//-----------------------------------------------------------
class KClone;
//-----------Clone interface---------------------------------
class KCloneData{
	public:
		unsigned char primitive_id;
		unsigned char material_id;
		CVector pos;
		CVector rot;
		CVector scale;
		float alpha;
		char* clone_name;
		int lock;
		int visible;//0で可視 1で不可視
		int tree_collapse;//0で展開, 1で折りたたみ
		KClone* copyclone;//クローン・・・
		unsigned char clonemode;//0でノーマル 1で単純クローン
		KCloneData()//とりあえずコンストラクタ
		{
			lock=0;
			visible=1;
			alpha = 1.0f;	
			scale = CVector(1.0f,1.0f,1.0f);
		}
};

class KClone{							//	  this---->child--->
	public:							    //		|        |
		KClone* sibling;				//	 sibling
		KClone* child;					//		|
		KCloneData clone_data;
		long __fastcall GetTreeNum();
		void __fastcall Draw(KObject** prm, KMaterial** mat, KClone* cln_alloc, int gname=1, int no_trans=0, int ninv=0, int subdiv=0, int wireframe=0,KCloneData* anim_data=NULL,float anim_alpha=1.0f);
		int __fastcall DrawFunc(KClone* seek_clone, void (*draw)(void));

};
//------------------------------------------------------------

//-------------Primitive Interface----------------------------
//KObject
//-------------------------------------------------------------

//=================KModel======================================
class KModel{
	protected:
		KMaterial* dof_far_material[KMD_MATERIAL_NUM];
		KMaterial* dof_near_material[KMD_MATERIAL_NUM];
		KMaterial* material[KMD_MATERIAL_NUM];
		KObject* pobject[KMD_PRIMITIVE_NUM];
		unsigned char* pobject_data[KMD_PRIMITIVE_NUM];
		KClone* tree;
		KClone* clone_alloc;
	public:
		//Get Interface
		KClone* __fastcall GetCloneCopy(char* clnname);
		long __fastcall GetCloneAllocNum();
		KClone* __fastcall GetTree();
		long __fastcall GetMaterialNum();
		KMaterial* __fastcall GetMaterial(long i);
		long __fastcall GetPrimitiveNum();
		KObject* __fastcall GetPrimitive(long i);

		void __fastcall Load(const unsigned char* dptr);
		void __fastcall Draw(int wireframe_mode,KCloneData* anim_data,int nomat = 0);
};
//==============================================================

#endif