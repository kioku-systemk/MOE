#pragma once

#include "kmodel.h"
  
class KAnimation
{
public:
	KCloneData** anim;//anim[clone_i][i]
	//long anim_num;
	float* anim_time;
	long animation_num;

	float GetLastTime();
	CVector Spline2D(CVector* vec, float rate);
	CVector CatmullRom(CVector* vec, float rate,long n);
	KCloneData GetBoneTrans(unsigned long ci, float animation_time, int interpolate);
	KAnimation(void);
	KAnimation(const KAnimation &obj);
	//KAnimation operator=(const KAnimation obj);
};
