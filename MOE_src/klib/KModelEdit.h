#pragma once
#include <stdio.h>
#include "KModel.h"

class KModelEdit : public KModel
{
private:
	unsigned char* mfile;
	void __fastcall PointerReAddressing(KClone* new_tree,KClone* old_tree, long clone_num);
public:
	void __fastcall DrawFunc(KClone* kclone, void(*draw)(void));
	KClone* __fastcall CreateClone(long create_num);
	long __fastcall PrimitiveLoadFromFile(long primitive_number, const char* filename);
	void __fastcall FreePrimitive(long primitive_number);
	void __fastcall Create();
	long __fastcall LoadFromFile(const char* filename);
	void __fastcall Free();
	void __fastcall SaveKMD(FILE* binary_fp);
	void __fastcall CreateMaterial(long material_number);
	void __fastcall FreeMaterial(long material_number);
	KClone* __fastcall GetCloneAllocPtr();
	void __fastcall SetTree(KClone* clone);
	unsigned long GetKMBFileSize();
	unsigned char* GetKMBFilePtr();

	KModelEdit();
	KModelEdit(const KModelEdit& obj);
	void CopyMaterialPrimitive(const KModelEdit& obj);

	void Import(const KModelEdit& kImp, unsigned long master_num, unsigned long dummy_num);
};
