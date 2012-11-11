//------------------------------------------------------------------------------------
//								KObjectImplimatation
//								   coded by kioku
//------------------------------------------------------------------------------------
#include "stdafx.h"
#include "KObject.h"
#include "kmath.h"

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

#define KOBJECT_TYPE_KOF				0
#define KOBJECT_TYPE_KMB				1
#define KOBJECT_TYPE_KPR				2

#define KOF_PRIMITIVE_NUM				256
//=================KObject class========================================================

void __fastcall KObject::FreeKMB()
{
	if(fvertex) GlobalFree(fvertex);
	if(fnormal) GlobalFree(fnormal);
}


void __fastcall KObject::FreeKOF()
{
	
}

void __fastcall KObject::FreeKPR()
{

}

const unsigned char* __fastcall KObject::LoadKMB(const unsigned char* dptr)
{
	objecttype = KOBJECT_TYPE_KMB;
	const unsigned char* st_dptr = dptr;
	//"KMB"
	dptr+=3;
	unsigned char versioninfo = *(unsigned char*)(dptr);dptr++;
	object_name = (char*)(dptr);
	long obj_name_size = (long)lstrlen(object_name)+1;
	dptr += obj_name_size;
	vertex_num  = *(unsigned short*)(dptr); dptr += sizeof(unsigned short);
	face_num[0] = *(unsigned short*)(dptr); dptr += sizeof(unsigned short);
	face_num[1] = *(unsigned short*)(dptr); dptr += sizeof(unsigned short);
				
	//vertex load
	short* svertex;
	svertex = (short*)(dptr); dptr+=sizeof(short)*3*vertex_num;
	face	= (unsigned short*)(dptr); dptr+=(sizeof(unsigned short)*(face_num[0]*3+face_num[1]*4));
			
	//vertex and normal buffer alloc
	fvertex = (CVector*)GlobalAlloc(0,sizeof(CVector)*vertex_num);
	fnormal = (CVector*)GlobalAlloc(0,sizeof(CVector)*vertex_num);

	//compute normal
	long vn,i;
	long vtmax=vertex_num;
	for(i=0; i<vtmax; i++){
		fvertex[i].x = hftof(svertex[i*3+0]);
		fvertex[i].y = hftof(svertex[i*3+1]);
		fvertex[i].z = hftof(svertex[i*3+2]);
		fnormal[i] = CVector(0,0,0);
	}
	long pface=0;
	for(vn=3; vn<=4; vn++){		
		for(i=0; i<face_num[vn-3]; i++){
			CVector vec[3];
			if(vn==3){
				vec[0] = fvertex[face[pface  ]] - fvertex[face[pface+1]];
				vec[1] = fvertex[face[pface+2]] - fvertex[face[pface+1]];
				vec[2] = vec[0].outer(vec[1]);
				vec[2].normalize();
				fnormal[face[pface  ]] += vec[2];
				fnormal[face[pface+1]] += vec[2];
				fnormal[face[pface+2]] += vec[2];
			}else{//vn==4
				vec[0] = fvertex[face[pface+1]] - fvertex[face[pface  ]];
				vec[1] = fvertex[face[pface+3]] - fvertex[face[pface  ]];
				vec[2] = vec[1].outer(vec[0]);
				vec[2].normalize();
				fnormal[face[pface  ]] += vec[2];
				fnormal[face[pface+1]] += vec[2];
				fnormal[face[pface+3]] += vec[2];
				vec[0] = fvertex[face[pface+1]] - fvertex[face[pface+2]];
				vec[1] = fvertex[face[pface+3]] - fvertex[face[pface+2]];
				vec[2] = (vec[0].outer(vec[1]));	
				vec[2].normalize();
				fnormal[face[pface+1]] += vec[2];
				fnormal[face[pface+2]] += vec[2];
				fnormal[face[pface+3]] += vec[2];
			}
			pface+=vn;
		}
	}
	for(i=0; i<vtmax; i++){
		fnormal[i].normalize();
	}

	//size
	kobject_size = (unsigned long)(dptr - st_dptr);
	return dptr;
}

const unsigned char* __fastcall KObject::LoadKOF(const unsigned char* dptr)
{
	objecttype = KOBJECT_TYPE_KOF;
//	objecttype=0;//KOF mode
//	objectsize = (unsigned short*)GlobalAlloc(0,sizeof(unsigned long)*KOF_PRIMITIVE_NUM);//alloc
//	kmo = (KObject*)GlobalAlloc(0,sizeof(KObject)*KOF_PRIMITIVE_NUM);//alloc
//	unsigned char versioninfo = *(unsigned char*)(&dptr[KOF_OFFSET_BYTE_VERSION]);
//	object_name = (char*)(&dptr[KOF_OFFSET_BYTE_OBJECT_NAME]);
//	long KOF_OFFSET_BYTE_NAMELEN= (long)lstrlen(object_name)+1;
//	
//	unsigned short cnum = *(unsigned short*)(&dptr[KOF_OFFSET_BYTE_CLONENUM]);
//	clone_num = cnum;
//	kcln =	(KCloneData*)GlobalAlloc(0,sizeof(KCloneData)*cnum);//alloc
//	long i;
//
//	const unsigned char* dp=&dptr[KOF_OFFSET_BYTE_CLONEDATA];
//	//primitive id
//	for(i=0; i<cnum; i++) kcln[i].primitive_id = *(unsigned char*)(dp++);
//	
//	//translation
//	for(i=0; i<cnum; i++){
//		kcln[i].pos.x = hftof(*(short*)(dp)); dp++;dp++;
//	}
//	for(i=0; i<cnum; i++){
//		kcln[i].pos.y = hftof(*(short*)(dp)); dp++;dp++;
//	}
//	for(i=0; i<cnum; i++){
//		kcln[i].pos.z = hftof(*(short*)(dp)); dp++;dp++;
//	}
//	//rotation
//	for(i=0; i<cnum; i++) kcln[i].rot.x = ctod(*(char*)(dp++));
//	for(i=0; i<cnum; i++) kcln[i].rot.y = ctod(*(char*)(dp++));
//	for(i=0; i<cnum; i++) kcln[i].rot.z = ctod(*(char*)(dp++));
//	//scale
//	for(i=0; i<cnum; i++){
//		kcln[i].scale.x = hftof(*(short*)(dp));	dp++;dp++;
//	}
//	for(i=0; i<cnum; i++){
//		kcln[i].scale.y = hftof(*(short*)(dp)); dp++;dp++;
//	}
//	for(i=0; i<cnum; i++){
//		kcln[i].scale.z = hftof(*(short*)(dp));	dp++;dp++;
//	}
//
//	//primitive model datasize
//	for(i=0; i<256; i++){
//		objectsize[i]=0;
//	}
//	unsigned char pnum = *(dp);
//	dp++;
//	for(i=0; i<pnum; i++){
//		objectsize[i] = *(unsigned short*)(dp);
//		dp++;dp++;dp++;dp++;
//	}
//	//primitive model data
//	for(i=0; i<pnum; i++){
//		if(objectsize[i]>0){
//			kmo[i].Load((unsigned char*)dp);
//			dp+=objectsize[i];
//		}
//	}
	return 0;
}


CVector __fastcall KObject::Spline2D(CVector vec0,CVector vec1,CVector vec2, float rate=0.5f)
{
	CVector r;
	r = vec0*(0.5f*(1.0f-rate)*(1.0f-rate))
		+ vec1*((1.0f-rate)*rate + 0.5f)
		+ vec2*(0.5f*rate*rate);
	return r;
}

void __fastcall KObject::DrawSDTriangle(CVector vt[3],CVector n[3], long divide, long mapping, unsigned int mt_texcnt)
{
	divide--;
	if(divide>=0){
		CVector v[3];
		long dn = 8;
		
		v[0] = vt[0]-n[0]/((float)(dn<<2))*1.3f;
		v[1] = vt[1]-n[1]/((float)(dn<<2))*1.3f;
		v[2] = vt[2]-n[2]/((float)(dn<<2))*1.3f;
		dn<<=(divide+1);
		CVector v01 = (v[0] + v[1])*0.5f;
		CVector v12 = (v[1] + v[2])*0.5f;
		CVector v20 = (v[2] + v[0])*0.5f;
		CVector n01 = (n[0] + n[1]); n01.normalize();
		CVector n12 = (n[1] + n[2]); n12.normalize();
		CVector n20 = (n[2] + n[0]); n20.normalize();
		v01 = Spline2D(v[0],v01+n01/(float)(dn),v[1]);
		v12 = Spline2D(v[1],v12+n12/(float)(dn),v[2]);
		v20 = Spline2D(v[2],v20+n20/(float)(dn),v[0]);

		CVector nv[4][3] = {{vt[0],v01,v20}
							,{v01,vt[1],v12}
							,{v20,v12,vt[2]}
							,{v01,v12,v20}};
		CVector nn[4][3] = {{n[0],n01,n20}
							,{n01,n[1],n12}
							,{n20,n12,n[2]}
							,{n01,n12,n20}};
		DrawSDTriangle(nv[0],nn[0],divide, mapping, mt_texcnt);
		DrawSDTriangle(nv[1],nn[1],divide, mapping, mt_texcnt);
		DrawSDTriangle(nv[2],nn[2],divide, mapping, mt_texcnt);
		DrawSDTriangle(nv[3],nn[3],divide, mapping, mt_texcnt);
	}else{
		CVector tcd[3];
		if(mapping==1){//cylinder mapping
			CVector vz(0,0,1);
			tcd[0].x = 2.0f*vt[0].z/vt[0].absolute()-1.0f; tcd[0].y=vt[0].y;
			tcd[1].x = 2.0f*vt[1].z/vt[1].absolute()-1.0f; tcd[1].y=vt[1].y;
			tcd[2].x = 2.0f*vt[2].z/vt[2].absolute()-1.0f; tcd[2].y=vt[2].y;
		}else if(mapping==3){//sphere mapping
			float ab;
			//ab=vt[0].absolute(); tcd[0].x = 2.0f*vt[0].z/ab-1.0f; tcd[0].y = 2.0f*vt[0].y/ab-1.0f;
			//ab=vt[1].absolute(); tcd[1].x = 2.0f*vt[1].z/ab-1.0f; tcd[1].y = 2.0f*vt[0].y/ab-1.0f;
			//ab=vt[2].absolute(); tcd[2].x = 2.0f*vt[2].z/ab-1.0f; tcd[2].y = 2.0f*vt[0].y/ab-1.0f;
			//ab=n[0].absolute(); tcd[0].x = 2.0f*n[0].z/ab-1.0f; tcd[0].y = 2.0f*n[0].y/ab-1.0f;
			//ab=n[1].absolute(); tcd[1].x = 2.0f*n[1].z/ab-1.0f; tcd[1].y = 2.0f*n[0].y/ab-1.0f;
			//ab=n[2].absolute(); tcd[2].x = 2.0f*n[2].z/ab-1.0f; tcd[2].y = 2.0f*n[0].y/ab-1.0f;
			tcd[0].x = 2.0f*n[0].x; tcd[0].y = n[0].y;
			tcd[1].x = 2.0f*n[1].x; tcd[1].y = n[1].y;
			tcd[2].x = 2.0f*n[2].x; tcd[2].y = n[2].y;
		}else{//plane mapping
			tcd[0] = vt[0];
			tcd[1] = vt[1];
			tcd[2] = vt[2];
		}
		if(mt_texcnt!=0){
			unsigned int i;
			for(i=0; i<mt_texcnt; i++){ glMultiTexCoord3fARB(GL_TEXTURE0_ARB+i,tcd[0].x,tcd[0].y,tcd[0].z); } glNormal3f(n[0].x,n[0].y,n[0].z); glVertex3f(vt[0].x,vt[0].y,vt[0].z); 
			for(i=0; i<mt_texcnt; i++){ glMultiTexCoord3fARB(GL_TEXTURE0_ARB+i,tcd[1].x,tcd[1].y,tcd[1].z); } glNormal3f(n[1].x,n[1].y,n[1].z); glVertex3f(vt[1].x,vt[1].y,vt[1].z); 
			for(i=0; i<mt_texcnt; i++){ glMultiTexCoord3fARB(GL_TEXTURE0_ARB+i,tcd[2].x,tcd[2].y,tcd[2].z); } glNormal3f(n[2].x,n[2].y,n[2].z); glVertex3f(vt[2].x,vt[2].y,vt[2].z); 
		}else{
			glTexCoord3f(tcd[0].x,tcd[0].y,tcd[0].z); glNormal3f(n[0].x,n[0].y,n[0].z); glVertex3f(vt[0].x,vt[0].y,vt[0].z); 
			glTexCoord3f(tcd[1].x,tcd[1].y,tcd[1].z); glNormal3f(n[1].x,n[1].y,n[1].z); glVertex3f(vt[1].x,vt[1].y,vt[1].z); 
			glTexCoord3f(tcd[2].x,tcd[2].y,tcd[2].z); glNormal3f(n[2].x,n[2].y,n[2].z); glVertex3f(vt[2].x,vt[2].y,vt[2].z); 
		}
	}
}

void __fastcall KObject::DrawSDSquare(CVector vt[4],CVector n[4], long divide, long mapping, unsigned int mt_texcnt)
{
	divide--;
	if(divide>=0){
		CVector v[4];
		long dn = 8;
		v[0] = vt[0]-n[0]/((float)(dn<<2))*1.3f;
		v[1] = vt[1]-n[1]/((float)(dn<<2))*1.3f;
		v[2] = vt[2]-n[2]/((float)(dn<<2))*1.3f;
		v[3] = vt[3]-n[3]/((float)(dn<<2))*1.3f;
		dn<<=(divide+1);
		CVector v01 = (v[0] + v[1])*0.5f;
		CVector v12 = (v[1] + v[2])*0.5f;
		CVector v23 = (v[2] + v[3])*0.5f; 
		CVector v30 = (v[3] + v[0])*0.5f; 
		CVector v4 =  (v01 + v12 + v23 + v30)*0.25f;
		CVector n01 = (n[0] + n[1]); n01.normalize();
		CVector n12 = (n[1] + n[2]); n12.normalize();
		CVector n23 = (n[2] + n[3]); n23.normalize();
		CVector n30 = (n[3] + n[0]); n30.normalize();
		CVector n4 =  (n01 + n12 + n23 + n30); n4.normalize();
		
		v01 = Spline2D(v[0],v01+n01/(float)(dn),v[1]);
		v12 = Spline2D(v[1],v12+n12/(float)(dn),v[2]);
		v23 = Spline2D(v[2],v23+n23/(float)(dn),v[3]);
		v30 = Spline2D(v[3],v30+n30/(float)(dn),v[0]);
		//v4  = (Spline2D(v01,v4+n4/(float)(dn),v23)+Spline2D(v12,v4+n4/(float)(dn),v30))*0.5f;
		v4  = (v01 + v12 + v23 + v30)*0.25f;
		CVector nv[4][4] = {{vt[0],v01,v4,v30}
							,{v01,vt[1],v12,v4}
							,{v4,v12,vt[2],v23}
							,{v30,v4,v23,vt[3]}};
		CVector nn[4][4] = {{n[0],n01,n4,n30}
							,{n01,n[1],n12,n4}
							,{n4,n12,n[2],n23}
							,{n30,n4,n23,n[3]}};
		DrawSDSquare(nv[0],nn[0],divide, mapping, mt_texcnt);
		DrawSDSquare(nv[1],nn[1],divide, mapping, mt_texcnt);
		DrawSDSquare(nv[2],nn[2],divide, mapping, mt_texcnt);
		DrawSDSquare(nv[3],nn[3],divide, mapping, mt_texcnt);
	}else{
		CVector tcd[4];
		if(mapping==1){//cylinder mapping
			CVector vz(0,0,1);
			tcd[0].x = 2.0f*vt[0].z/vt[0].absolute()-1.0f; tcd[0].y=vt[0].y;
			tcd[1].x = 2.0f*vt[1].z/vt[1].absolute()-1.0f; tcd[1].y=vt[1].y;
			tcd[2].x = 2.0f*vt[2].z/vt[2].absolute()-1.0f; tcd[2].y=vt[2].y;
			tcd[3].x = 2.0f*vt[2].z/vt[3].absolute()-1.0f; tcd[3].y=vt[3].y;
		}else if(mapping==3){//sphere mapping
			//float ab;
			/*ab=vt[0].absolute(); tcd[0].x = 2.0f*vt[0].z/ab-1.0f; tcd[0].y = 2.0f*vt[0].y/ab-1.0f;
			ab=vt[1].absolute(); tcd[1].x = 2.0f*vt[1].z/ab-1.0f; tcd[1].y = 2.0f*vt[0].y/ab-1.0f;
			ab=vt[2].absolute(); tcd[2].x = 2.0f*vt[2].z/ab-1.0f; tcd[2].y = 2.0f*vt[0].y/ab-1.0f;
			ab=vt[3].absolute(); tcd[3].x = 2.0f*vt[2].z/ab-1.0f; tcd[3].y = 2.0f*vt[0].y/ab-1.0f;*/
			tcd[0].x = 2.0f*n[0].x/(n[0].z+0.0001f); tcd[0].y = n[0].y;
			tcd[1].x = 2.0f*n[1].x; tcd[1].y = n[1].y;
			tcd[2].x = 2.0f*n[2].x; tcd[2].y = n[2].y;
			tcd[3].x = 2.0f*n[3].x; tcd[3].y = n[3].y;
		}else{//plane mapping
			tcd[0] = vt[0];
			tcd[1] = vt[1];
			tcd[2] = vt[2];
			tcd[3] = vt[3];
		}
		if(mt_texcnt!=0){
			unsigned int i;
			for(i=0; i<mt_texcnt; i++){ glMultiTexCoord3fARB(GL_TEXTURE0_ARB+i,tcd[0].x,tcd[0].y,tcd[0].z); } glNormal3f(n[0].x,n[0].y,n[0].z); glVertex3f(vt[0].x,vt[0].y,vt[0].z); 
			for(i=0; i<mt_texcnt; i++){ glMultiTexCoord3fARB(GL_TEXTURE0_ARB+i,tcd[1].x,tcd[1].y,tcd[1].z); } glNormal3f(n[1].x,n[1].y,n[1].z); glVertex3f(vt[1].x,vt[1].y,vt[1].z); 
			for(i=0; i<mt_texcnt; i++){ glMultiTexCoord3fARB(GL_TEXTURE0_ARB+i,tcd[2].x,tcd[2].y,tcd[2].z); } glNormal3f(n[2].x,n[2].y,n[2].z); glVertex3f(vt[2].x,vt[2].y,vt[2].z); 
			for(i=0; i<mt_texcnt; i++){ glMultiTexCoord3fARB(GL_TEXTURE0_ARB+i,tcd[3].x,tcd[3].y,tcd[3].z); } glNormal3f(n[3].x,n[3].y,n[3].z); glVertex3f(vt[3].x,vt[3].y,vt[3].z);
		}else{
			glTexCoord3f(tcd[0].x,tcd[0].y,tcd[0].z); glNormal3f(n[0].x,n[0].y,n[0].z); glVertex3f(vt[0].x,vt[0].y,vt[0].z); 
			glTexCoord3f(tcd[1].x,tcd[1].y,tcd[1].z); glNormal3f(n[1].x,n[1].y,n[1].z); glVertex3f(vt[1].x,vt[1].y,vt[1].z); 
			glTexCoord3f(tcd[2].x,tcd[2].y,tcd[2].z); glNormal3f(n[2].x,n[2].y,n[2].z); glVertex3f(vt[2].x,vt[2].y,vt[2].z); 
			glTexCoord3f(tcd[3].x,tcd[3].y,tcd[3].z); glNormal3f(n[3].x,n[3].y,n[3].z); glVertex3f(vt[3].x,vt[3].y,vt[3].z);
		}
	}
}


void __fastcall KObject::DrawKMB(int wireframe,int ninv, int subdiv, long mapping, unsigned int mt_texcnt){
	long vn,i;
	long vcnt=0;
	
	for(vn=3; vn<=4; vn++){
		for(i=0; i<face_num[vn-3]; i++){
			if(wireframe!=0){
				glBegin(GL_LINE_LOOP);
			}else{
				if(vn==4) glBegin(GL_QUADS);
				else      glBegin(GL_TRIANGLES);
			}	
			//new
			if(vn==3){
				CVector v[3] = {fvertex[face[vcnt]],fvertex[face[vcnt+1]],fvertex[face[vcnt+2]]};
				CVector n[3];
				if(ninv==0){
					n[0] = fnormal[face[vcnt]];
					n[1] = fnormal[face[vcnt+1]];
					n[2] = fnormal[face[vcnt+2]];
				}else{
					n[0] = fnormal[face[vcnt]]*-1;
					n[1] = fnormal[face[vcnt+1]]*-1;
					n[2] = fnormal[face[vcnt+2]]*-1;
				}
				DrawSDTriangle(v,n,subdiv, mapping, mt_texcnt);
				vcnt+=3;
			}else{
				CVector v[4] = {fvertex[face[vcnt]],fvertex[face[vcnt+1]],fvertex[face[vcnt+2]],fvertex[face[vcnt+3]]};
				CVector n[4];
				if(ninv==0){
					n[0] = fnormal[face[vcnt]];
					n[1] = fnormal[face[vcnt+1]];
					n[2] = fnormal[face[vcnt+2]];
					n[3] = fnormal[face[vcnt+3]];
				}else{
					n[0] = fnormal[face[vcnt]]*-1;
					n[1] = fnormal[face[vcnt+1]]*-1;
					n[2] = fnormal[face[vcnt+2]]*-1;
					n[3] = fnormal[face[vcnt+3]]*-1;
				}
				DrawSDSquare(v,n,subdiv, mapping, mt_texcnt);
				vcnt+=4;
			}
			glEnd();
		}
	}
}//DrawKMB

void __fastcall KObject::DrawKOF(int wireframe,int ninv){
//	long i;
//	unsigned short cnum = clone_num;
//	for(i=0; i<cnum; i++){
//		long pninv=ninv;
//		if(objectsize[kcln[i].primitive_id]>0){
//			glPushMatrix();
//				glTranslatef(kcln[i].pos.x,kcln[i].pos.y,kcln[i].pos.z);
//				glRotatef(kcln[i].rot.x,1,0,0);
//				glRotatef(kcln[i].rot.y,0,1,0);
//				glRotatef(kcln[i].rot.z,0,0,1);
//				glScalef(kcln[i].scale.x,kcln[i].scale.y,kcln[i].scale.z);
//				if((kcln[i].scale.x*kcln[i].scale.y*kcln[i].scale.z)<0){
//					if(pninv!=0) pninv=0;
//					else		pninv=1;
//				}
//				kmo[kcln[i].primitive_id].Draw(wireframe,pninv);
//			glPopMatrix();
//		}
//	}
}
		
void __fastcall KObject::DrawKPR(int wireframe,int ninv)
{
	objecttype = KOBJECT_TYPE_KPR;
}
const unsigned char* __fastcall KObject::LoadKPR(const unsigned char* dptr)
{
	return 0;
}

void __fastcall KObject::Free()
{
	if(objecttype==KOBJECT_TYPE_KMB) FreeKOF();
	if(objecttype==KOBJECT_TYPE_KMB) FreeKPR();
	if(objecttype==KOBJECT_TYPE_KMB) FreeKMB();
}

const unsigned char* __fastcall KObject::Load(const unsigned char* dptr)
{
	if((dptr[0]=='K')&&(dptr[1]=='O')&&(dptr[2]=='F'))		dptr = LoadKOF(dptr);
	else if((dptr[0]=='K')&&(dptr[1]=='M')&&(dptr[2]=='B')) dptr = LoadKMB(dptr);
	else													dptr = LoadKPR(dptr);
	return dptr;
}
void __fastcall KObject::Draw(int wireframe,int ninv, int subdiv, long mapping, unsigned int mt_texcnt)
{
	//if(matnum==0)      DrawKOF(wireframe,ninv);
	//else if(matnum==1) DrawKPR(wireframe,ninv);
	//else		       DrawKMB(wireframe,ninv);
	DrawKMB(wireframe,ninv,subdiv, mapping, mt_texcnt);
}

unsigned long __fastcall KObject::GetSize()
{
	return kobject_size;
}

//=====================================================================================



