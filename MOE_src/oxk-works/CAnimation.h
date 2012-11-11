/*
	MQOBone class
	2004/10/23	coded by KIOKU			since 2004/10/23
*/
#pragma once
#include "MQO_File.h"
using namespace std;

#include "../klib/kmodel.h"

  
class CAnimation : private CMQO_File
{
public:
	vector< vector<KCloneData> >	anim;//anim[clone_i][i]
	vector<float> anim_time;

	float GetLastTime();
	CVector Spline2D(CVector* vec, float rate);
	CVector CatmullRom(CVector* vec, float rate,long n);
	KCloneData GetBoneTrans(unsigned long ci, float animation_time, int interpolate);
	CAnimation(void);
	~CAnimation(void);
	CAnimation(const CAnimation &obj);
	CAnimation &operator=(const CAnimation &obj);
};
