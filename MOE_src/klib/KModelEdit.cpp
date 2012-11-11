#include "stdafx.h"
#include "KModelEdit.h"
#include "windows.h"


KModelEdit::KModelEdit()
{
	Create();
}

KModelEdit::KModelEdit(const KModelEdit& obj)
{
	//copy clone_alloc
	long trn;
	if(obj.clone_alloc!=NULL) trn = ((long)GlobalSize(obj.clone_alloc)/sizeof(KClone));
	else				  trn = 0;
	clone_alloc = (KClone*)GlobalAlloc(GPTR,sizeof(KClone)*trn);
	CopyMemory(clone_alloc,obj.clone_alloc,sizeof(KClone)*trn);//old to new
	PointerReAddressing(clone_alloc,obj.clone_alloc,trn);
	//copy tree
	if(obj.tree!=NULL)  tree = (obj.tree - obj.clone_alloc) + clone_alloc;
	else				tree = NULL;
	//copy material
	CopyMemory(material,obj.material,sizeof(KMaterial*)*KMD_MATERIAL_NUM);

	//copy pobject
	CopyMemory(pobject,obj.pobject,sizeof(KObject*)*KMD_PRIMITIVE_NUM);

	CopyMemory(pobject_data,obj.pobject_data,sizeof(unsigned char*)*KMD_PRIMITIVE_NUM);

	long mfsize = (long)GlobalSize(obj.mfile);
	mfile = (unsigned char*)GlobalAlloc(GPTR,mfsize);
	CopyMemory(mfile, obj.mfile,mfsize);
}

void KModelEdit::CopyMaterialPrimitive(const KModelEdit &obj)
{
	long i;
	for(i=0; i<KMD_PRIMITIVE_NUM; i++){
		pobject[i] = obj.pobject[i];
		pobject_data[i] = obj.pobject_data[i];
	}
	for(i=0; i<KMD_MATERIAL_NUM; i++){
		material[i] = obj.material[i];
	}
}


void __fastcall KModelEdit::DrawFunc(KClone* kclone, void(*draw)(void))
{
	if(tree!=NULL) tree->DrawFunc(kclone,draw);
}


void __fastcall KModelEdit::SetTree(KClone* clone)
{
	tree = clone;
}


KClone* __fastcall KModelEdit::GetCloneAllocPtr()
{
	return clone_alloc;
}

void __fastcall KModelEdit::PointerReAddressing(KClone* new_tree,KClone* old_tree, long clone_num)
{
	//ReAddressing
	long i;
	for(i=0; i<clone_num; i++){
		if(old_tree[i].child!=NULL)   new_tree[i].child  = (old_tree[i].child - old_tree ) + new_tree;
		else						  new_tree[i].child  = NULL;
		if(old_tree[i].sibling!=NULL) new_tree[i].sibling = (old_tree[i].sibling - old_tree) + new_tree;
		else						  new_tree[i].sibling = NULL;
		if(new_tree[i].clone_data.copyclone!=NULL) new_tree[i].clone_data.copyclone = (old_tree[i].clone_data.copyclone - old_tree) + new_tree;
		else									   new_tree[i].clone_data.copyclone = NULL;
	}
}

KClone* __fastcall KModelEdit::CreateClone(long create_num)
{
	KClone* new_alloc;
	if(clone_alloc!=NULL){
		long trn = GetCloneAllocNum();
		new_alloc = (KClone*)GlobalAlloc(GPTR,sizeof(KClone)*(trn+create_num));
		CopyMemory(new_alloc,clone_alloc,sizeof(KClone)*trn);//old to new
		PointerReAddressing(new_alloc,clone_alloc,trn);
		//tree ReAddress
		if(tree!=NULL)  tree = (tree - clone_alloc) + new_alloc;
		else			tree = NULL;
		GlobalFree(clone_alloc);
		clone_alloc = new_alloc;
		new_alloc = &clone_alloc[trn];//new ptr
	}else{
		new_alloc = (KClone*)GlobalAlloc(GPTR,sizeof(KClone)*create_num);
		clone_alloc = new_alloc;
	}
	return new_alloc;
}

void __fastcall KModelEdit::Create()
{
	mfile=NULL;
	long i;
	for(i=0; i<KMD_MATERIAL_NUM; i++){
		material[i] = NULL;
	}
	
	//primitive
	for(i=0; i<KMD_PRIMITIVE_NUM; i++){
		pobject_data[i] = NULL;
		pobject[i] = NULL;
	}
	
	tree=NULL;
	clone_alloc=NULL;
}

void __fastcall KModelEdit::CreateMaterial(long material_number)
{
	long i = material_number;
	if((i>=0)&&(i<KMD_MATERIAL_NUM)){
		material[i] = (KMaterial*)GlobalAlloc(GPTR,sizeof(KMaterial));
		material[i]->color.r = 0.5f;
		material[i]->color.g = 0.5f;
		material[i]->color.b = 0.5f;
		material[i]->color.a = 1.0f;
		//material[i]->gltexure_num = 0;
//<MULTITEXTUIRE>
		material[i]->number_of_texture = 1;
		material[i]->texture_id = (unsigned int*)GlobalAlloc(GPTR, sizeof(unsigned int) * 1);

		material[i]->multi_texture_env =(KMultiTextureEnv*)GlobalAlloc(GPTR, sizeof(KMultiTextureEnv) * 1);
		material[i]->multi_texture_env[0].op |= 0x01;//GL_MODULATE
		material[i]->multi_texture_env[0].op |= 0x01<<4;//GL_MODULATE

		material[i]->multi_texture_env[0].source_param[0] |= 0x00;//GL_TEXTURE
		material[i]->multi_texture_env[0].source_param[0] |= 0x00<<4;//GL_TEXTURE
		material[i]->multi_texture_env[0].source_param[1] |= 0x03;//GL_PREVIOUS
		material[i]->multi_texture_env[0].source_param[1] |= 0x03<<4;//GL_PREVIOUS
		material[i]->multi_texture_env[0].source_param[2] |= 0x01;//GL_CONSTANT
		material[i]->multi_texture_env[0].source_param[2] |= 0x01<<4;//GL_CONSTANT

		material[i]->multi_texture_env[0].operand_param[0] |= 0x00;//GL_COLOR
		material[i]->multi_texture_env[0].operand_param[0] |= 0x02<<4;//GL_ALPHA
		material[i]->multi_texture_env[0].operand_param[1] |= 0x00;//GL_COLOR
		material[i]->multi_texture_env[0].operand_param[1] |= 0x02<<4;//GL_ALPHA
		material[i]->multi_texture_env[0].operand_param[2] |= 0x02;//GL_ALPHA
		material[i]->multi_texture_env[0].operand_param[2] |= 0x02<<4;//GL_ALPHA

		material[i]->multi_texture_env[0].fscale |= 0x00;//1.0f
		material[i]->multi_texture_env[0].fscale |= 0x00<<4;//1.0f

		material[i]->texenv = (unsigned char*)GlobalAlloc(GPTR, sizeof(unsigned char) * 1);//0x01;
		//material[i]->texture_id = NULL;
		//material[i]->multi_texture_env = NULL;
//</MULTITEXTUIRE>
		material[i]->texture = (char**)GlobalAlloc(GPTR,sizeof(char*) * 1);
		material[i]->texture[0] = (char*)GlobalAlloc(GPTR,sizeof(char) * 1);
		material[i]->blendf = 0x01;
		material[i]->subdivide = 0;
		material[i]->shade = 0x04;
		//material[i]->texenv = NULL;//0x01;
		material[i]->uv_trans = CVector(0,0,0);
		material[i]->uv_rot = CVector(0,0,0);
		material[i]->uv_scale = CVector(1,1,1);
		lstrcpy(material[i]->mat_name,"mat");

		material[i]->shader.vs = (char*)GlobalAlloc(GPTR, 1);
		material[i]->shader.ps = (char*)GlobalAlloc(GPTR, 1);
	}	
}

void __fastcall KModelEdit::FreeMaterial(long material_number)
{
	long i = material_number;
	if((i>=0)&&(i<KMD_MATERIAL_NUM)){
		if(material[i]!=NULL){
			int j=0;
			for(j=0; j<material[i]->number_of_texture; j++){
				if(material[i]->texture[j]!=NULL){
					GlobalFree(material[i]->texture[j]);
					material[i]->texture[j]=NULL;
				}
			}
			GlobalFree(material[i]->texture);
			if(material[i]->texture_id!=NULL){
				GlobalFree(material[i]->texture_id);
			}
			if(material[i]->texenv!=NULL){
				GlobalFree(material[i]->texenv);
			}
			if(material[i]->multi_texture_env!=NULL){
				GlobalFree(material[i]->multi_texture_env);
			}
			if(material[i]->shader.vs!=NULL){
				GlobalFree(material[i]->shader.vs);
			}
			if(material[i]->shader.ps!=NULL){
				GlobalFree(material[i]->shader.ps);
			}
			GlobalFree(material[i]);
			material[i]=NULL;
		}
	}
}

long __fastcall KModelEdit::PrimitiveLoadFromFile(long primitive_number, const char* filename)
{
	long i = primitive_number;
	if((i>=0)&&(i<KMD_PRIMITIVE_NUM)){
		FILE* fp;
		if((fp=fopen(filename,"rb"))==NULL) return -1;//open error
		fseek(fp,0,SEEK_END);
		long filesize = ftell(fp);
		fseek(fp,0,SEEK_SET);
		//内部でメモリを共有しているので途中で解放してはならない ->DeInitialize
		unsigned char*  filepointer = (unsigned char*)GlobalAlloc(GPTR,filesize);
		fread(filepointer,filesize,1,fp);
		//primitive load
		pobject[i]=(KObject*)GlobalAlloc(GPTR,sizeof(KObject));
		pobject[i]->Load(filepointer);
		pobject_data[i] = filepointer;
		fclose(fp);
		return 0;
	}
	return -1;
}

void __fastcall KModelEdit::FreePrimitive(long primitive_number)
{
	long i = primitive_number;
    if((i>=0)&&(i<KMD_PRIMITIVE_NUM)){
		if(pobject[i]!=NULL){
			pobject[i]->Free();
			GlobalFree(pobject[i]);
			pobject[i] = NULL;
		}
		if(pobject_data[i]!=NULL){
			GlobalFree(pobject_data[i]);
			pobject_data[i] = NULL;
		}
	}
}

long __fastcall KModelEdit::LoadFromFile(const char* filename)
{
	__try{
		FILE* fp;
		if((fp=fopen(filename,"rb"))==NULL) return -1;
		fseek(fp,0,SEEK_END);
		long filesize = ftell(fp);
		fseek(fp,0,SEEK_SET);
		//内部でメモリを共有しているので途中で解放してはならない ->DeInitialize
		mfile = (unsigned char*)GlobalAlloc(GPTR,filesize);
		fread(mfile,filesize,1,fp);
		fclose(fp);

		//データロード
		Load(mfile);

		//----------エディタで可変の部分のメモリをコピーしなおす------------------
		long i;
		for(i=0; i<KMD_MATERIAL_NUM; i++){
			if(material[i]!=NULL){
				long j;
				for(j=0; j<material[i]->number_of_texture; j++){
					long tlen = lstrlen(material[i]->texture[j])+1;
					char* tstr = (char*)GlobalAlloc(GPTR,tlen);
					lstrcpy(tstr,material[i]->texture[j]);
					material[i]->texture[j] = tstr;
				}
				if(material[i]->blendf==0) material[i]->blendf = 118;//SRC_ALPHA | ONE_MINUS_SRC_A

				long tlen;
				char* tstr;
				tlen = lstrlen(material[i]->shader.vs)+1;
				tstr= (char*)GlobalAlloc(GPTR, tlen);
				CopyMemory(tstr, material[i]->shader.vs, tlen);
				material[i]->shader.vs = tstr;

				tlen = lstrlen(material[i]->shader.ps)+1;
				tstr= (char*)GlobalAlloc(GPTR, tlen);
				CopyMemory(tstr, material[i]->shader.ps, tlen);
				material[i]->shader.ps = tstr;
			}
		}
		
		for(i=0; i<KMD_PRIMITIVE_NUM; i++){
			if(pobject[i]!=NULL){
				long objsize = pobject[i]->GetSize();
				unsigned char* nobj = (unsigned char*)GlobalAlloc(GPTR,objsize);
				CopyMemory(nobj, pobject_data[i], objsize);
				pobject_data[i] = nobj;
			}
		}

		long trn = 0;
		if(tree!=NULL) trn = tree->GetTreeNum();
		for(i=0; i<trn; i++){
			long cln = lstrlen(tree[i].clone_data.clone_name)+1;
			char* pcln = (char*)GlobalAlloc(GPTR, sizeof(char)*cln);
			CopyMemory(pcln,tree[i].clone_data.clone_name,cln);
			tree[i].clone_data.clone_name = pcln;
		}

		//-------------------------------------------------------------------------------
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		char str[512];
		wsprintf(str, "ファイル読み取り中に例外発生。\nファイルが破損している可能性があります\n%s", filename);
		MessageBox(NULL, str, "kmb読み取りエラー", MB_SYSTEMMODAL);
	}
	return 0;
}

void __fastcall KModelEdit::Free()
{
	if(mfile!=NULL){
		GlobalFree(mfile);
		mfile = NULL;
	}
	long i;
	for(i=0; i<KMD_MATERIAL_NUM; i++) FreeMaterial(i);
	
	for(i=0; i<KMD_PRIMITIVE_NUM; i++) FreePrimitive(i);

	tree=NULL;
	if(clone_alloc!=NULL){
		GlobalFree(clone_alloc);
		clone_alloc=NULL;
	}
}

void __fastcall KModelEdit::SaveKMD(FILE* binary_fp)
{
	long i;
 	FILE* fp = binary_fp;
	fwrite("KMD",sizeof(unsigned char)*3,1,fp);
	//unsigned char versioninfo=0;
	unsigned char versioninfo=1;//multi-texture support.
	fwrite(&versioninfo,sizeof(unsigned char),1,fp);
	unsigned char mtnum=0;
	for(i=0; i<KMD_MATERIAL_NUM; i++){
		if(material[i]!=NULL) mtnum++;
	}
	fwrite(&mtnum,sizeof(unsigned char),1,fp);
	//material loop
	
	for(i=0; i<KMD_MATERIAL_NUM; i++){
		if(material[i]!=NULL){
			unsigned char mtn= (unsigned char)i;
			fwrite(&mtn,sizeof(unsigned char),1,fp);
			unsigned char scolor[] = {  (unsigned char)(material[i]->color.r*255),
										(unsigned char)(material[i]->color.g*255),
										(unsigned char)(material[i]->color.b*255),
										(unsigned char)(material[i]->color.a*255)};
			fwrite(&(scolor[0]),sizeof(unsigned char),1,fp);
			fwrite(&(scolor[1]),sizeof(unsigned char),1,fp);
			fwrite(&(scolor[2]),sizeof(unsigned char),1,fp);
			fwrite(&(scolor[3]),sizeof(unsigned char),1,fp);
			int j;

			fwrite(&(material[i]->number_of_texture),sizeof(unsigned char),1,fp);
			for(j=0; j<material[i]->number_of_texture; j++){
				fwrite(material[i]->texture[j],sizeof(unsigned char)*lstrlen(material[i]->texture[j])+1,1,fp);
			}
			fwrite(&(material[i]->subdivide),sizeof(unsigned char),1,fp);

			for(j=0; j<material[i]->number_of_texture; j++){
				fwrite(&(material[i]->multi_texture_env[j].op),sizeof(unsigned char),1,fp);
				int l;
				for(l=0; l<3; l++){
					fwrite(&(material[i]->multi_texture_env[j].source_param[l]),sizeof(unsigned char),1,fp);
					fwrite(&(material[i]->multi_texture_env[j].operand_param[l]),sizeof(unsigned char),1,fp);
				}
				fwrite(&(material[i]->multi_texture_env[j].fscale),sizeof(unsigned char),1,fp);
			}

			fwrite(&(material[i]->mat_name),8*sizeof(unsigned char),1,fp);
			fwrite(&(material[i]->shade),sizeof(unsigned char),1,fp);
			for(j=0; j<material[i]->number_of_texture; j++){
				fwrite(&(material[i]->texenv[j]),sizeof(unsigned char),1,fp);
			}
			short sh[3];
			sh[0] = ftohf(material[i]->uv_trans.x);
			sh[1] = ftohf(material[i]->uv_trans.y);
			sh[2] = ftohf(material[i]->uv_trans.z);
			fwrite(&sh,3*sizeof(short),1,fp);
			sh[0] = ftohf(material[i]->uv_rot.x);
			sh[1] = ftohf(material[i]->uv_rot.y);
			sh[2] = ftohf(material[i]->uv_rot.z);
			fwrite(&sh,3*sizeof(short),1,fp);
			sh[0] = ftohf(material[i]->uv_scale.x);
			sh[1] = ftohf(material[i]->uv_scale.y);
			sh[2] = ftohf(material[i]->uv_scale.z);
			fwrite(&sh,3*sizeof(short),1,fp);
			fwrite(&(material[i]->blendf),sizeof(unsigned char),1,fp);
			//fwrite(&(material[i]->tex_operator),sizeof(unsigned char)*material[i]->texcnt,1,fp);

			fwrite(material[i]->shader.vs,sizeof(unsigned char)*lstrlen(material[i]->shader.vs)+1,1,fp);
			fwrite(material[i]->shader.ps,sizeof(unsigned char)*lstrlen(material[i]->shader.ps)+1,1,fp);

			unsigned char reserve[KMD_MATERIAL_RESERVE_VER1] = {0};
			fwrite(&reserve,sizeof(unsigned char)*(KMD_MATERIAL_RESERVE_VER1),1,fp);
		}
	}

	//primitive loop
	unsigned char obnum=0;
	for(i=0; i<KMD_PRIMITIVE_NUM; i++){
		if(pobject[i]!=NULL) obnum++;
	}
	fwrite(&obnum,sizeof(unsigned char),1,fp);
	for(i=0; i<KMD_PRIMITIVE_NUM; i++){
		if(pobject[i]!=NULL){
			unsigned char obn = (unsigned char)i;
			fwrite(&obn,sizeof(unsigned char),1,fp);
			fwrite(pobject_data[i], pobject[i]->GetSize(),1,fp);
		}
	}

	//tree loop
	unsigned short clone_num;
	if(tree!=NULL) clone_num = (unsigned short)tree->GetTreeNum();	
	else		   clone_num=0;
		
	fwrite(&clone_num,sizeof(unsigned short),1,fp);
	//------tree to list-------------------------------------------------
	KClone** listed_clone = NULL;
	unsigned char* listed_clone_node = NULL;
	if(clone_num>0){
		listed_clone = (KClone**)GlobalAlloc(GPTR,clone_num*sizeof(KClone*));
		listed_clone_node = (unsigned char*)GlobalAlloc(GPTR,clone_num);
		KClone** node_ptr=(KClone**)GlobalAlloc(GPTR,256*sizeof(KClone*));
		KClone* kpt=GetTree();
		unsigned char node=0;
		node_ptr[node]=kpt;
		long child_end=0;

		long cn=1;
		listed_clone[0] = kpt;
		listed_clone_node[0] = node;
		while(node!=255){
			if((child_end!=1)&&(kpt->child!=NULL)){
				node++;
				kpt = kpt->child;
				node_ptr[node]=kpt;
				child_end=0;
				//--copy--------------------
					listed_clone[cn] = kpt;
					listed_clone_node[cn] = node;
					cn++;
				//--------------------------
			}else if(kpt->sibling!=NULL){
				kpt = kpt->sibling;
				node_ptr[node]=kpt;
				child_end=0;
				//--copy--------------------
					listed_clone[cn] = kpt;
					listed_clone_node[cn] = node;
					cn++;
				//--------------------------
			}else{
				node--;
				kpt = node_ptr[node];
				child_end=1;
			}
		}
		GlobalFree(node_ptr);
		//----------------------------------------------------------------------
	}
	for(i=0; i<clone_num; i++) fwrite(&listed_clone_node[i],sizeof(unsigned char),1,fp);
	for(i=0; i<clone_num; i++) fwrite(&(listed_clone[i]->clone_data.primitive_id),sizeof(unsigned char),1,fp);
	for(i=0; i<clone_num; i++) fwrite(&(listed_clone[i]->clone_data.material_id ),sizeof(unsigned char),1,fp);
	//position
	short spos;
	for(i=0; i<clone_num; i++){
		spos = ftohf(listed_clone[i]->clone_data.pos.x);
		fwrite(&spos,sizeof(short),1,fp);
	}
	for(i=0; i<clone_num; i++){
		spos = ftohf(listed_clone[i]->clone_data.pos.y);
		fwrite(&spos,sizeof(short),1,fp);
	}
	for(i=0; i<clone_num; i++){
		spos = ftohf(listed_clone[i]->clone_data.pos.z);
		fwrite(&spos,sizeof(short),1,fp);
	}
	//rotation
	char crot;
	for(i=0; i<clone_num; i++){
		crot = dtoc(listed_clone[i]->clone_data.rot.x);
		fwrite(&crot,sizeof(char),1,fp);
	}
	for(i=0; i<clone_num; i++){
		crot = dtoc(listed_clone[i]->clone_data.rot.y);
		fwrite(&crot,sizeof(char),1,fp);
	}
	for(i=0; i<clone_num; i++){
		crot = dtoc(listed_clone[i]->clone_data.rot.z);
		fwrite(&crot,sizeof(char),1,fp);
	}
	//scale
	short sscal;
	for(i=0; i<clone_num; i++){
		sscal = ftohf(listed_clone[i]->clone_data.scale.x);
		fwrite(&sscal,sizeof(short),1,fp);
	}
	for(i=0; i<clone_num; i++){
		sscal = ftohf(listed_clone[i]->clone_data.scale.y);
		fwrite(&sscal,sizeof(short),1,fp);
	}
	for(i=0; i<clone_num; i++){
		sscal = ftohf(listed_clone[i]->clone_data.scale.z);
		fwrite(&sscal,sizeof(short),1,fp);
	}
	for(i=0; i<clone_num; i++){
		long cln = lstrlen(listed_clone[i]->clone_data.clone_name)+1;
		fwrite(listed_clone[i]->clone_data.clone_name,cln,1,fp);
	}
	for(i=0; i<clone_num; i++){
		fwrite(&(listed_clone[i]->clone_data.clonemode),sizeof(unsigned char),1,fp);
	}
	
	if(listed_clone != NULL)	 GlobalFree(listed_clone);
	if(listed_clone_node != NULL)GlobalFree(listed_clone_node);
	return;	
}

unsigned long KModelEdit::GetKMBFileSize()
{
	return (unsigned long)GlobalSize(mfile);
}

unsigned char* KModelEdit::GetKMBFilePtr()
{
	return mfile;
}

void KModelEdit::Import(const KModelEdit& kImp, unsigned long master_num , unsigned long dummy_num)
{
	int i,j;
	int mat_table[256] = {0};
	int obj_table[256] = {0};

	//material
	for(i=0; i<KMD_MATERIAL_NUM; i++){ //インポートしてきたやつのマテリアル
		KMaterial* impmat = kImp.material[i];
		if(impmat==NULL) continue;
		
		for(j=0; j<KMD_MATERIAL_NUM; j++){//既存のマテリアル
			KMaterial* defmat = GetMaterial(j);
			if(defmat==NULL){	//j番目にマテリアルが存在しなかったら
				CreateMaterial(j); //j番目にマテリアルを新規作成
				
				KMaterial* tgmat = GetMaterial(j); //新規作成したマテリアルのアドレスを取得して
				*tgmat = *impmat;	//インポートしてきたマテリアルの内容を全てコピーする
				
				//テクスチャは可変長なので、例外的に処理する必要がある
				unsigned int k;
				unsigned int nTex = impmat->number_of_texture;
				tgmat->texture = (char**)GlobalAlloc(GPTR, sizeof(char*) * nTex);
				for(k=0; k<nTex; k++){
					tgmat->texture[k] = (char*)GlobalAlloc(GPTR, sizeof(char) * (lstrlen(impmat->texture[k])+1));
					lstrcpy(tgmat->texture[k], impmat->texture[k]);
				}

				mat_table[i] = j; //インポートしてきたモデルのi番目のマテリアルが, 統合先の何番目のマテリアルに上書きされたか保存する
				break;
			}else if(lstrcmp(defmat->mat_name, impmat->mat_name)==0){ //名称が同一の場合, 統合先のマテリアルを継承する
				mat_table[i] = j; //統合先の何番目のマテリアルを継承するかを保存する
				break;
			}
		}
	}

	//primitive
	for(i=0; i<KMD_PRIMITIVE_NUM; i++){
		KObject* impobj = kImp.pobject[i];
		if(impobj==NULL) continue;
		
		for(j=0; j<KMD_PRIMITIVE_NUM; j++){
			KObject* defobj = GetPrimitive(j);
			if(defobj==NULL){
				pobject[j] = kImp.pobject[i];
				pobject_data[j] = kImp.pobject_data[i];

				obj_table[i] = j;
				break;
			}else if(lstrcmp(defobj->GetName(), impobj->GetName())==0){
				obj_table[i] = j;
				break;
			}
		}
	}

	//clone
	int inum = ((long)GlobalSize(kImp.clone_alloc)/sizeof(KClone)); //インポートしてきたクローンの数
	KClone* kcln = CreateClone(inum); //インポートしてきたクローンの数だけ、統合先にクローンを作成する
	CopyMemory(kcln, kImp.clone_alloc, sizeof(KClone)*inum); //インポートしたクローンを統合先にコピー
	PointerReAddressing(kcln,kImp.clone_alloc,inum); //
    
	//re-numbering primitive,material
	for(i=0; i<inum; i++){
		kcln[i].clone_data.primitive_id = (unsigned char)obj_table[kcln[i].clone_data.primitive_id];
		kcln[i].clone_data.material_id  = (unsigned char)mat_table[kcln[i].clone_data.material_id];
	}

	//connect
	KClone* master = &clone_alloc[master_num];
	KClone* dummy_cln = &clone_alloc[dummy_num];
	if(master!=dummy_cln){
		if(master->child==dummy_cln){
			master->child=kcln;
		}else if(master->sibling==dummy_cln){
			master->sibling=kcln;
		}else{
			MessageBox(NULL, "アドレスが変です。", 0, MB_ICONEXCLAMATION);	
		}
	}else{ //master == Tree
		//master=kcln;
		SetTree(kcln);
		MessageBox(NULL, "追加されたダミーがGetTree(); //普通にOpenしろよ・・・", "Warning", MB_ICONEXCLAMATION|MB_SYSTEMMODAL);	
	}


//	return kcln;
	
}