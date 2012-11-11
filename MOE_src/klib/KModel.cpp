//------------------------------------------------------------------------------------
//								KModelImplimatation
//								   coded by kioku
//------------------------------------------------------------------------------------
#include "stdafx.h"
#include "KModel.h"
#include "kmath.h"
#include "kTexture.h"

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

const unsigned int nMultiTextureOperator[] = {
	GL_REPLACE,
	GL_MODULATE,
	GL_ADD,
	GL_ADD_SIGNED_ARB,
	GL_INTERPOLATE_ARB,
	GL_SUBTRACT_ARB
};
const unsigned int nMultiTextureSource[] = {
	GL_TEXTURE,
	GL_CONSTANT_ARB,
	GL_PRIMARY_COLOR_ARB,
	GL_PREVIOUS_ARB
};
const unsigned int nMultiTextureOperand[] = {
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA
};
const float fMultiTextureScale[] = {
1.0f,
2.0f,
4.0f
};

#define OPTIMIZE_GL_NORMALIZE(__x_in_float,__y_in_float,__z_in_float)\
	if( __x_in_float==1.0f && __y_in_float==1.0f && __z_in_float==1.0f){ glDisable(GL_NORMALIZE); }else{ glEnable(GL_NORMALIZE); }


//=================KClone class========================================================
long __fastcall KClone::GetTreeNum()
{
	long cnum=1;
	if(child!=NULL)  cnum += child->GetTreeNum();
	if(sibling!=NULL) cnum += sibling->GetTreeNum();
	return cnum;
}



void __fastcall KClone::Draw(KObject** prm, KMaterial** mat, KClone* cln_alloc, int gname, int no_trans, int ninv, int subdiv, int wireframe, KCloneData* anim_data,float anim_alpha)
{
	float tmp_animalpha = anim_alpha;
	long cn = (long)(this-cln_alloc);
	if(gname==1){
		glLoadName((unsigned int)(cn)+10);//0-9はaxis用
	}
	glPushMatrix();
		if(clone_data.clonemode==0){//ノーマルクローン
			
			if(prm[clone_data.primitive_id]!=NULL){
				//平行移動のみ階層ローカル座標
				if(no_trans==0){
					glTranslatef(clone_data.pos.x,clone_data.pos.y,clone_data.pos.z);
					if(anim_data!=NULL){
						glTranslatef(anim_data[cn].pos.x,anim_data[cn].pos.y,anim_data[cn].pos.z);
						glRotatef(anim_data[cn].rot.x,1,0,0);
						glRotatef(anim_data[cn].rot.y,0,1,0);
						glRotatef(anim_data[cn].rot.z,0,0,1);
						OPTIMIZE_GL_NORMALIZE(anim_data[cn].scale.x,anim_data[cn].scale.y,anim_data[cn].scale.z);
						glScalef(anim_data[cn].scale.x,anim_data[cn].scale.y,anim_data[cn].scale.z);

						anim_alpha *= anim_data[cn].alpha;
					}
				}else{
					no_trans--;//引いとく
				}

				if(clone_data.visible==0){
					glPushMatrix(); 
						glRotatef(clone_data.rot.x,1,0,0);
						glRotatef(clone_data.rot.y,0,1,0);
						glRotatef(clone_data.rot.z,0,0,1);
						OPTIMIZE_GL_NORMALIZE(clone_data.scale.x,clone_data.scale.y,clone_data.scale.z);
						glScalef(clone_data.scale.x,clone_data.scale.y,clone_data.scale.z);
					if(mat[clone_data.material_id]!=NULL){
						if(mat[clone_data.material_id]->shade==0x02){//const
							glDisable(GL_LIGHTING);
							glDisable(GL_LIGHT0);
						}else{
							glEnable(GL_LIGHTING);
							glEnable(GL_LIGHT0);
						}
						long mywireframe = wireframe|((mat[clone_data.material_id]->shade&0x10)>>4);//wire
						
						if(mat[clone_data.material_id]->shade&0x04){//smooth flat
							glShadeModel(GL_SMOOTH);
						}else{
							glShadeModel(GL_FLAT);
						}
						//unsigned long gtex = mat[clone_data.material_id]->gltexure_num;
						unsigned int* gtex = mat[clone_data.material_id]->texture_id;
						//gtex = dum;
						long mapping_mode = 0;
						//if(gtex!=0){
						unsigned int i;
						unsigned int nTexNum = mat[clone_data.material_id]->number_of_texture;
						if(gtex!=NULL){
							for(i=0; i<nTexNum; i++){
								if(gtex[i]==0){
									//nop
									//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
									//glBindTexture(GL_TEXTURE_2D, 0); //意外に重い
									glDisable(GL_TEXTURE_2D);
								}else{
									int k;
									glActiveTextureARB(GL_TEXTURE0_ARB+i);
									glEnable(GL_TEXTURE_2D);
									glBindTexture(GL_TEXTURE_2D, gtex[i]);
									glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);

									glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, nMultiTextureOperator[mat[clone_data.material_id]->multi_texture_env[i].op&0x0F]);
									glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, nMultiTextureOperator[mat[clone_data.material_id]->multi_texture_env[i].op>>4]);
									for(k=0; k<3; k++){
										glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB+k,    nMultiTextureSource [mat[clone_data.material_id]->multi_texture_env[i].source_param[k]&0x0F ]);
										glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB+k,   nMultiTextureOperand[mat[clone_data.material_id]->multi_texture_env[i].operand_param[k]&0x0F]);
										glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB+k,  nMultiTextureSource [mat[clone_data.material_id]->multi_texture_env[i].source_param[k]>>4   ]);
										glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB+k, nMultiTextureOperand[mat[clone_data.material_id]->multi_texture_env[i].operand_param[k]>>4  ]);
									}
									glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, fMultiTextureScale[mat[clone_data.material_id]->multi_texture_env[i].fscale&0x0F]);
									glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, fMultiTextureScale[mat[clone_data.material_id]->multi_texture_env[i].fscale>>4]);

									////setup multi-texture on rgb channel.
									//glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, nMultiTextureOperator[mat[clone_data.material_id]->multi_texture_env[i].op&0x0F]);
									//for(k=0; k<3; k++){
									//	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB+k, nMultiTextureSource[mat[clone_data.material_id]->multi_texture_env[i].source_param[k]&0x0F]);
									//}
									//for(k=0; k<3; k++){
									//	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB+k, nMultiTextureOperand[mat[clone_data.material_id]->multi_texture_env[i].operand_param[k]&0x0F]);
									//}
									//glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, fMultiTextureScale[mat[clone_data.material_id]->multi_texture_env[i].fscale&0x0F]);

									////setup multi-texture on alpha channel.
									//glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, nMultiTextureOperator[mat[clone_data.material_id]->multi_texture_env[i].op>>4]);
									//for(k=0; k<3; k++){
									//	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB+k, nMultiTextureSource[mat[clone_data.material_id]->multi_texture_env[i].source_param[k]>>4]);
									//}
									//for(k=0; k<3; k++){
									//	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB+k, nMultiTextureOperand[mat[clone_data.material_id]->multi_texture_env[i].operand_param[k]>>4]);
									//}
									//glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, fMultiTextureScale[mat[clone_data.material_id]->multi_texture_env[i].fscale>>4]);
	
									if(mat[clone_data.material_id]->texenv[i]&0x04){//enviroment mapping
										glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
										glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
										glEnable(GL_TEXTURE_GEN_S);
										glEnable(GL_TEXTURE_GEN_T);
									}else{
										if(mat[clone_data.material_id]->texenv[i]&0x08)		 mapping_mode=2;
										else if(mat[clone_data.material_id]->texenv[i]&0x10) mapping_mode=3;
										glDisable(GL_TEXTURE_GEN_S);
										glDisable(GL_TEXTURE_GEN_T);
										float uvt[3]={mat[clone_data.material_id]->uv_trans.x,mat[clone_data.material_id]->uv_trans.y,mat[clone_data.material_id]->uv_trans.z};
										float uvr[3]={mat[clone_data.material_id]->uv_rot.x,mat[clone_data.material_id]->uv_rot.y,mat[clone_data.material_id]->uv_rot.z};
										float uvs[3]={mat[clone_data.material_id]->uv_scale.x,mat[clone_data.material_id]->uv_scale.y,mat[clone_data.material_id]->uv_scale.z};
										glMatrixMode(GL_TEXTURE);
											glActiveTextureARB(GL_TEXTURE0_ARB+i);
											glLoadIdentity();
											glTranslatef(uvt[0],uvt[1],uvt[2]);
											glRotatef(uvr[0],1,0,0);
											glRotatef(uvr[1],0,1,0);
											glRotatef(uvr[2],0,0,1);
											glScalef(uvs[0],uvs[1],uvs[2]);
										glMatrixMode(GL_MODELVIEW);
									}
								}
							}
						}else{
							//glBindTexture(GL_TEXTURE_2D, 0); //意外に重い
							glDisable(GL_TEXTURE_2D);
						}
						KRGBA rgba = mat[clone_data.material_id]->color;
						//glColor4f(rgba.r,rgba.g,rgba.b,rgba.a*anim_alpha);
						float mcolor[] = {rgba.r,rgba.g,rgba.b,rgba.a*anim_alpha};
						glColor4fv(mcolor);
						glEnable(GL_BLEND);
						glBlendFunc(nBlendFactorList[mat[clone_data.material_id]->blendf&0x0F],nBlendFactorList[(mat[clone_data.material_id]->blendf)>>4]);
						glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,mcolor);
						prm[clone_data.primitive_id]->Draw(mywireframe,ninv,mat[clone_data.material_id]->subdivide + subdiv,mapping_mode, nTexNum);

						//unbind texture and disactivate multi-texture
						if(gtex!=NULL){
							for(i=nTexNum-1; (signed)i>=0; i--){
								if(gtex[i]==0){
									//nop.
									glDisable(GL_TEXTURE_2D);
								}else{
									glActiveTextureARB(GL_TEXTURE0_ARB+i);
									glMatrixMode(GL_TEXTURE);
										glLoadIdentity();
									glMatrixMode(GL_MODELVIEW);
									glActiveTextureARB(GL_TEXTURE0_ARB + i);
									glDisable(GL_TEXTURE_2D);
								}
							}
						}
					}else{
						glEnable(GL_LIGHTING);
						glEnable(GL_LIGHT0);
						glDisable(GL_TEXTURE_2D);
						glDisable(GL_BLEND);
						//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
						//glBindTexture(GL_TEXTURE_2D, 0); //意外に重い
						float mcolor[] = {0.5f,0.5f,0.5f,0.5f};
						glColor4fv(mcolor);
						glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,mcolor);
						prm[clone_data.primitive_id]->Draw(wireframe,ninv,0+subdiv);
					}
					glPopMatrix();
				}

				if(child!=NULL)  child->Draw(prm,mat,cln_alloc,gname,no_trans,ninv,subdiv,wireframe,anim_data,anim_alpha);
			}
		}else if(clone_data.clonemode>0){//コピークローン
			if(clone_data.visible==0){
				glTranslatef(clone_data.pos.x,clone_data.pos.y,clone_data.pos.z);
				if(anim_data!=NULL){
					glTranslatef(anim_data[cn].pos.x,anim_data[cn].pos.y,anim_data[cn].pos.z);
					glRotatef(anim_data[cn].rot.x,1,0,0);
					glRotatef(anim_data[cn].rot.y,0,1,0);
					glRotatef(anim_data[cn].rot.z,0,0,1);

					//optim.
					OPTIMIZE_GL_NORMALIZE(anim_data[cn].scale.x,anim_data[cn].scale.y,anim_data[cn].scale.z);
					glScalef(anim_data[cn].scale.x,anim_data[cn].scale.y,anim_data[cn].scale.z);

					anim_alpha *= anim_data[cn].alpha;
				}
				
				glRotatef(clone_data.rot.x,1,0,0);
				glRotatef(clone_data.rot.y,0,1,0);
				glRotatef(clone_data.rot.z,0,0,1);
				OPTIMIZE_GL_NORMALIZE(clone_data.scale.x,clone_data.scale.y,clone_data.scale.z);
				glScalef(clone_data.scale.x,clone_data.scale.y,clone_data.scale.z);

				if(clone_data.copyclone!=NULL){
					KClone* sib = clone_data.copyclone->sibling;
					clone_data.copyclone->sibling = NULL;//兄をいったん空にして
					if(clone_data.clonemode==2)  clone_data.copyclone->Draw(prm,mat,cln_alloc,0,1,(ninv+1)&1,subdiv,wireframe,anim_data,anim_alpha);//本人と子を描画
					else						 clone_data.copyclone->Draw(prm,mat,cln_alloc,0,1,ninv,subdiv,wireframe,anim_data,anim_alpha);//本人と子を描画
					clone_data.copyclone->sibling = sib;//兄を呼びもす
				}
			}
		}

	glPopMatrix();
	anim_alpha = tmp_animalpha;
	if(sibling!=NULL) sibling->Draw(prm,mat,cln_alloc,gname,no_trans,ninv,subdiv,wireframe,anim_data,anim_alpha);
}

int __fastcall KClone::DrawFunc(KClone* seek_clone, void (*draw)(void))
{
	int ret=0;
	glPushMatrix();
		if(seek_clone==this){
			draw();
			ret=1;
		}
		glTranslatef(clone_data.pos.x,clone_data.pos.y,clone_data.pos.z);
		//glRotatef(clone_data.rot.x,1,0,0);
		//glRotatef(clone_data.rot.y,0,1,0);
		//glRotatef(clone_data.rot.z,0,0,1);
		//glScalef(clone_data.scale.x,clone_data.scale.y,clone_data.scale.z);
		if(child!=NULL){
			if(child->DrawFunc(seek_clone, draw)==1) ret=1;
		}
	glPopMatrix();
	if(ret==1) return 1;
	if(sibling!=NULL){
		if(sibling->DrawFunc(seek_clone, draw)==1) return 1;
	}
	return 0;
}
//int __fastcall KClone::DrawFunc(KClone* seek_clone, void (*draw)(void))
//{
//	int ret=0;
//	glPushMatrix();
//		if(seek_clone==this){
//			glPushMatrix(); 
//				draw();
//				ret=1;
//			glPopMatrix();
//		}
//		if(ret!=1){
//			glTranslatef(clone_data.pos.x,clone_data.pos.y,clone_data.pos.z);
//			if(child!=NULL) ret=child->DrawFunc(seek_clone, draw);
//		}
//	glPopMatrix();
//	if(ret!=1){
//		if(sibling!=NULL) ret=sibling->DrawFunc(seek_clone, draw);
//	}
//	return ret;
//}
//=================KModel class========================================================
long __fastcall KModel::GetCloneAllocNum()
{	
	if(clone_alloc!=NULL) return ((long)GlobalSize(clone_alloc)/sizeof(KClone));
	else				  return 0;
}

KClone* __fastcall KModel::GetCloneCopy(char* clnname)
{
	long ci=0;
	long cnmax = GetCloneAllocNum();
	while(1){
		if(ci==cnmax) break;
		if((clone_alloc[ci].clone_data.clonemode!=0)
		|| (lstrcmp(clone_alloc[ci].clone_data.clone_name,clnname)!=0)){
			ci++;
		}else{
			break;
		}
	}
	if(ci==cnmax) return NULL;
	else		  return &clone_alloc[ci];
}

void __fastcall KModel::Draw(int wireframe, KCloneData* anim_data, int nomat)
{
	//clone draw
	//glDisable(GL_COLOR_MATERIAL);
	if(tree!=NULL){
		if(nomat!=0){//dof
			tree->Draw(pobject, (nomat==1) ? dof_far_material : dof_near_material, clone_alloc,1,0,0,0,wireframe,anim_data);
		}else{//
			tree->Draw(pobject, material, clone_alloc,1,0,0,0,wireframe,anim_data);
		}
	}
}

KClone*		__fastcall KModel::GetTree()				{ return tree;				}
long		__fastcall KModel::GetMaterialNum()			{ return KMD_MATERIAL_NUM;	}
KMaterial*	__fastcall KModel::GetMaterial(long i)		{ return material[i];		}
long		__fastcall KModel::GetPrimitiveNum()		{ return KMD_PRIMITIVE_NUM;	}
KObject*	__fastcall KModel::GetPrimitive(long i)		{ return pobject[i];		}

void __fastcall KModel::Load(const unsigned char* dptr){
	long i;
	//init
	//'K''M''D'
	const unsigned char* doffset=(&dptr[3]);
	unsigned char versioninfo = *(unsigned char*)(doffset++);
	long material_num  = *(unsigned char*)(doffset++);
	for(i=0; i<material_num; i++){
		unsigned long mnum = *(unsigned char*)(doffset++);
		material[mnum] = (KMaterial*)GlobalAlloc(GPTR,sizeof(KMaterial));
		material[mnum]->color.r = (*(unsigned char*)(doffset++))/255.0f;
		material[mnum]->color.g = (*(unsigned char*)(doffset++))/255.0f;
		material[mnum]->color.b = (*(unsigned char*)(doffset++))/255.0f;
		material[mnum]->color.a = (*(unsigned char*)(doffset++))/255.0f;
		material[mnum]->number_of_texture = (versioninfo==0) ? 1 : (*(unsigned char*)(doffset++));

		const int ntex = material[mnum]->number_of_texture;
		material[mnum]->texture =(char**)GlobalAlloc(GPTR, sizeof(char*) * ntex);
		int k;
		for(k=0; k<ntex; k++){
			material[mnum]->texture[k] = (char*)(doffset);
			doffset += ((long)lstrlen(material[mnum]->texture[k])+1);
		}
		material[mnum]->subdivide = *(unsigned char*)(doffset++);

		material[mnum]->texture_id = (unsigned int*)GlobalAlloc(GPTR, sizeof(unsigned int)*ntex);
		material[mnum]->multi_texture_env = (KMultiTextureEnv*)GlobalAlloc(GPTR, sizeof(KMultiTextureEnv)*ntex);
		for(k=0; k<ntex; k++){
			KTexture ktex;
			ktex.GenerateTextureIndirect(&material[mnum]->texture_id[k], material[mnum]->texture[k]);

			if(versioninfo>0){
				material[mnum]->multi_texture_env[k].op = *(unsigned char*)(doffset++);
				int l;
				for(l=0; l<3; l++){
					material[mnum]->multi_texture_env[k].source_param[l] = *(unsigned char*)(doffset++);
					material[mnum]->multi_texture_env[k].operand_param[l] = *(unsigned char*)(doffset++);
				}
				material[mnum]->multi_texture_env[k].fscale = *(unsigned char*)(doffset++);
				//material[mnum]->multi_texture_env[k].fscale_rgb = hftof(*(short*)(doffset)); doffset+=2;
				//material[mnum]->multi_texture_env[k].fscale_alpha = hftof(*(short*)(doffset)); doffset+=2;
			}else{
				unsigned char* write;
				write = &material[mnum]->multi_texture_env[k].op;
				*write |= 0x01;//GL_MODULATE
				*write |= 0x01<<4;//GL_MODULATE

				write = &material[mnum]->multi_texture_env[k].source_param[0];
				*write |= 0x00;//GL_TEXTURE
				*write |= 0x00<<4;//GL_TEXTURE

				write = &material[mnum]->multi_texture_env[k].source_param[1];
				*write |= 0x03;//GL_PREVIOUS
				*write |= 0x03<<4;//GL_PREVIOUS
				
				write = &material[mnum]->multi_texture_env[k].source_param[2];
				*write |= 0x01;//GL_CONSTANT
				*write |= 0x01<<4;//GL_CONSTANT

				write = &material[mnum]->multi_texture_env[k].operand_param[0];
				*write |= 0x00;//GL_COLOR
				*write |= 0x02<<4;//GL_ALPHA

				write = &material[mnum]->multi_texture_env[k].operand_param[1];
				*write |= 0x00;//GL_COLOR
				*write |= 0x02<<4;//GL_ALPHA

				write = &material[mnum]->multi_texture_env[k].operand_param[2];
				*write |= 0x02;//GL_ALPHA
				*write |= 0x02<<4;//GL_ALPHA

				write = &material[mnum]->multi_texture_env[k].fscale;
				*write |= 0x00;//1.0f
				*write |= 0x00<<4;//1.0f
			}
		}

		kmemcpy(material[mnum]->mat_name, (char*)(doffset),8); doffset+=8;
		material[mnum]->shade = *(unsigned char*)(doffset++);
		
		material[mnum]->texenv = (unsigned char*)GlobalAlloc(GPTR, sizeof(unsigned char) * ntex);
		for(k=0; k<ntex; k++){
			material[mnum]->texenv[k] = *(unsigned char*)(doffset++);
		}

		material[mnum]->uv_trans.x = hftof(*(short*)(doffset)); doffset+=2;
		material[mnum]->uv_trans.y = hftof(*(short*)(doffset)); doffset+=2;
		material[mnum]->uv_trans.z = hftof(*(short*)(doffset)); doffset+=2;
		material[mnum]->uv_rot.x = hftof(*(short*)(doffset)); doffset+=2;
		material[mnum]->uv_rot.y = hftof(*(short*)(doffset)); doffset+=2;
		material[mnum]->uv_rot.z = hftof(*(short*)(doffset)); doffset+=2;
		material[mnum]->uv_scale.x = hftof(*(short*)(doffset)); doffset+=2;
		material[mnum]->uv_scale.y = hftof(*(short*)(doffset)); doffset+=2;
		material[mnum]->uv_scale.z = hftof(*(short*)(doffset)); doffset+=2;
		material[mnum]->blendf = *(unsigned char*)(doffset++);

		if(versioninfo>0){
			material[mnum]->shader.vs = (char*)(doffset);
			doffset += ((long)lstrlen(material[mnum]->shader.vs)+1);
			material[mnum]->shader.ps = (char*)(doffset);
			doffset += ((long)lstrlen(material[mnum]->shader.ps)+1);
		}else{
			material[mnum]->shader.vs = "";
			material[mnum]->shader.ps = "";
		}

		//material[mnum]->
		//material[mnum].reserve;
		if(versioninfo==0){
			doffset += KMD_MATERIAL_RESERVE_VER0;
		}else{
			doffset += KMD_MATERIAL_RESERVE_VER1;
		}

		//Create DOF Material
		//const int prev=0;
		//if(prev){
		//	//dof_far_material[mnum] = (KMaterial*)GlobalAlloc(GPTR,sizeof(KMaterial));
		//	//kmemcpy(dof_far_material[mnum], material[mnum], sizeof(KMaterial));
		//	//dof_far_material[mnum]->color.r = 0.0f;
		//	//dof_far_material[mnum]->color.g = 0.0f;
		//	//dof_far_material[mnum]->color.b = 0.0f;
		//	//dof_far_material[mnum]->color.a = 1.0f;
		//	//dof_far_material[mnum]->shade	= 0x02;
		//	//dof_far_material[mnum]->texture_id = NULL;
		//	//dof_far_material[mnum]->number_of_texture = 0;
		//	//dof_far_material[mnum]->texenv = NULL;
		//	//dof_far_material[mnum]->multi_texture_env = NULL;
		//	//dof_far_material[mnum]->blendf  = 0x01;

		//	//dof_near_material[mnum] = (KMaterial*)GlobalAlloc(GPTR,sizeof(KMaterial));
		//	//kmemcpy(dof_near_material[mnum], material[mnum], sizeof(KMaterial));
		//	//dof_near_material[mnum]->color.r = 1.0f;
		//	//dof_near_material[mnum]->color.g = 1.0f;
		//	//dof_near_material[mnum]->color.b = 1.0f;
		//	//dof_near_material[mnum]->color.a = 1.0f;
		//	//dof_near_material[mnum]->shade	 = 0x02;
		//	//dof_near_material[mnum]->texture_id = NULL;
		//	//dof_near_material[mnum]->number_of_texture = 0;
		//	//dof_near_material[mnum]->texenv = NULL;
		//	//dof_near_material[mnum]->multi_texture_env = NULL;
		//	//dof_near_material[mnum]->blendf  = 0x01;

		//	dof_far_material[mnum] = (KMaterial*)GlobalAlloc(GPTR,sizeof(KMaterial));
		//	kmemcpy(dof_far_material[mnum], material[mnum], sizeof(KMaterial));
		//	dof_far_material[mnum]->color.r = 0.0f;
		//	dof_far_material[mnum]->color.g = 0.0f;
		//	dof_far_material[mnum]->color.b = 0.0f;
		//	dof_far_material[mnum]->color.a = 1.0f;
		//	dof_far_material[mnum]->shade	= 0x02;
		//	dof_far_material[mnum]->texture_id = NULL;
		//	dof_far_material[mnum]->number_of_texture = 0;
		//	//dof_far_material[mnum]->blendf  = 0x01;

		//	dof_near_material[mnum] = (KMaterial*)GlobalAlloc(GPTR,sizeof(KMaterial));
		//	kmemcpy(dof_near_material[mnum], material[mnum], sizeof(KMaterial));
		//	dof_near_material[mnum]->color.r = 1.0f;
		//	dof_near_material[mnum]->color.g = 1.0f;
		//	dof_near_material[mnum]->color.b = 1.0f;
		//	dof_near_material[mnum]->color.a = 1.0f;
		//	dof_near_material[mnum]->shade	 = 0x02;
		//	dof_near_material[mnum]->texture_id = NULL;
		//	dof_near_material[mnum]->number_of_texture = 0;
		//	//dof_near_material[mnum]->blendf  = 0x01;
		//}else{
			dof_far_material[mnum] = (KMaterial*)GlobalAlloc(GPTR,sizeof(KMaterial));
			kmemcpy(dof_far_material[mnum], material[mnum], sizeof(KMaterial));
			dof_far_material[mnum]->color.r = 0.0f;
			dof_far_material[mnum]->color.g = 0.0f;
			dof_far_material[mnum]->color.b = 0.0f;
			//dof_far_material[mnum]->color.a = material[mnum]->color.a;
			dof_far_material[mnum]->shade	= 0x02;

			//dof_far_material[mnum]->texture_id = material[mnum]->texture_id;
			//dof_far_material[mnum]->number_of_texture = material[mnum]->number_of_texture;
			//dof_far_material[mnum]->texenv = material[mnum]->texenv;
			dof_far_material[mnum]->multi_texture_env = (KMultiTextureEnv*)GlobalAlloc(GPTR, sizeof(KMultiTextureEnv)*ntex);
			//dof_far_material[mnum]->multi_texture_env = material[mnum]->multi_texture_env;
			//dof_far_material[mnum]->blendf  = material[mnum]->blendf;
			for(k=0; k<ntex; k++){
				kmemcpy(&(dof_far_material[mnum]->multi_texture_env[k]), &(material[mnum]->multi_texture_env[k]), sizeof(KMultiTextureEnv));

				unsigned char* write;
				unsigned char alp;
				
				write = &(dof_far_material[mnum]->multi_texture_env[k].op);
				alp = *write>>4;
				*write = 0x00;//REPLACE
				*write |= alp<<4;
				
				write = &(dof_far_material[mnum]->multi_texture_env[k].source_param[0]);
				alp = *write>>4;
				*write = 0x02;//PRIMARY
				*write |= alp<<4;

				write = &(dof_far_material[mnum]->multi_texture_env[k].operand_param[0]);
				alp = *write>>4;
				*write = 0x00;//SRC_COLOR
				*write |= alp<<4;

				write = &(dof_far_material[mnum]->multi_texture_env[k].fscale);
				alp = *write>>4;
				*write = 0x00;//1.0f
				*write |= alp<<4;
			}

			dof_near_material[mnum] = (KMaterial*)GlobalAlloc(GPTR,sizeof(KMaterial));
			kmemcpy(dof_near_material[mnum], material[mnum], sizeof(KMaterial));
			dof_near_material[mnum]->color.r = 1.0f;
			dof_near_material[mnum]->color.g = 1.0f;
			dof_near_material[mnum]->color.b = 1.0f;
			//dof_near_material[mnum]->color.a = material[mnum]->color.a;
			dof_near_material[mnum]->shade	 = 0x02;

			//dof_near_material[mnum]->texture_id = material[mnum]->texture_id;
			//dof_near_material[mnum]->number_of_texture = material[mnum]->number_of_texture;
			//dof_near_material[mnum]->texenv = material[mnum]->texenv;
			dof_near_material[mnum]->multi_texture_env = (KMultiTextureEnv*)GlobalAlloc(GPTR, sizeof(KMultiTextureEnv)*ntex);
			//dof_near_material[mnum]->multi_texture_env = material[mnum]->multi_texture_env;
			//dof_near_material[mnum]->blendf  = material[mnum]->blendf;
			for(k=0; k<ntex; k++){
				kmemcpy(&(dof_near_material[mnum]->multi_texture_env[k]), &(material[mnum]->multi_texture_env[k]), sizeof(KMultiTextureEnv));

				unsigned char alp;
				unsigned char* write;
				write = &(dof_near_material[mnum]->multi_texture_env[k].op);
				
				alp = *write>>4;
				*write = 0x00;//REPLACE
				*write |= alp<<4;
				
				write = &(dof_near_material[mnum]->multi_texture_env[k].source_param[0]);
				alp = *write>>4;
				*write = 0x02;//PRIMARY
				*write |= alp<<4;

				write = &(dof_near_material[mnum]->multi_texture_env[k].operand_param[0]);
				alp = *write>>4;
				*write = 0x00;//SRC_COLOR
				*write |= alp<<4;

				write = &(dof_near_material[mnum]->multi_texture_env[k].fscale);
				alp = *write>>4;
				*write = 0x00;//1.0f
				*write |= alp<<4;
			}
		//}
	}
	
	//primitive
	long primitive_num = *(unsigned char*)(doffset++);
	for(i=0; i<KMD_PRIMITIVE_NUM; i++){
		pobject_data[i] = NULL;
		pobject[i] = NULL;
	}
	for(i=0; i<primitive_num; i++){
		long object_number = *(unsigned char*)(doffset++);
		pobject[object_number] = (KObject*)GlobalAlloc(GPTR,sizeof(KObject));
		pobject_data[object_number] = (unsigned char*)doffset;
		doffset = pobject[object_number]->Load(doffset);
	}
	
	//tree
	unsigned short tree_num  = *(unsigned short*)(doffset); 
	doffset+=2;
	KClone* temp_tree;
	if(tree_num!=0) temp_tree = (KClone*)GlobalAlloc(GPTR, sizeof(KClone)*tree_num);
	else			temp_tree = NULL;
	clone_alloc = temp_tree;
	unsigned char bef_node=0;
	KClone** node_ptr = (KClone**)GlobalAlloc(GPTR,256*sizeof(KClone*));
	for(i=0; i<tree_num; i++){
		unsigned char node = *(unsigned char*)(doffset++);
		if(bef_node< node){
			if(node_ptr[node]!=NULL) node_ptr[bef_node]->child = &temp_tree[i];
			if(bef_node>=0) node_ptr[bef_node]->child = &temp_tree[i];
			node_ptr[node] = &temp_tree[i];
		}else{
			if(node_ptr[node]!=NULL) node_ptr[node]->sibling = &temp_tree[i];
			node_ptr[node] = &temp_tree[i];
		}
		bef_node = node;
	}
	
	//data
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.primitive_id = *(unsigned char*)(doffset);
		doffset++;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.material_id = *(unsigned char*)(doffset);
		doffset++;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.pos.x = hftof(*(short*)(doffset));
		doffset+=2;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.pos.y = hftof(*(short*)(doffset));
		doffset+=2;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.pos.z = hftof(*(short*)(doffset));
		doffset+=2;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.rot.x = ctod (*(char*)(doffset));
		doffset++;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.rot.y = ctod (*(char*)(doffset));
		doffset++;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.rot.z = ctod (*(char*)(doffset)); 
		doffset++;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.scale.x = hftof(*(short*)(doffset));
		doffset+=2;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.scale.y = hftof(*(short*)(doffset));
		doffset+=2;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.scale.z = hftof(*(short*)(doffset)); 
		doffset+=2;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.clone_name = (char*)(doffset);
		doffset+=lstrlen(temp_tree[i].clone_data.clone_name)+1;
	}
	for(i=0; i<tree_num; i++){
		temp_tree[i].clone_data.clonemode = *(unsigned char*)(doffset);
		doffset++;
	}
	//set
	tree = temp_tree;
	GlobalFree(node_ptr);

	//CopyCloneSet
	for(i=0; i<tree_num; i++){
		if(temp_tree[i].clone_data.clonemode!=0){
			KClone* kcl = GetCloneCopy(temp_tree[i].clone_data.clone_name);
			temp_tree[i].clone_data.copyclone = kcl;
		}else{
			temp_tree[i].clone_data.copyclone=NULL;
		}
	}
}

//=====================================================================================