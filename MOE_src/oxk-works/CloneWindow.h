#include "common.h"

/* �O������̌Ăяo���p */
//KClone�̃C���f�b�N�X�ɊY�����鍀�ڂ�I����Ԃɂ���
void CloneSelectClone(int);
//KModelEdit*������,�c���[�r���[���č\�z����
void RefreshCloneTree(KModelEdit*, const char* szFileName, int select);
//���ݑI������Ă��鍀�ڂɊY������KClone�̃C���f�b�N�X��Ԃ�
int CloneGetSelectedItem();
KClone* GetSelectedClone();
void FreeCloneWindow();

LRESULT CALLBACK CloneHookProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* �����Ŏg�p����֐� */
void ClearCloneTree(HWND hTreeView);
void CloneSelectedItemNotify(int nDest, bool isByMouse, int nOrg, bool isDrag);
int CloneNameEdit(int nDest, const TCHAR* szNewName, bool isAboutToEdit);
int CloneKey(UCHAR key, bool isDown);
int CloneMouse(long x, long y, UINT btn);