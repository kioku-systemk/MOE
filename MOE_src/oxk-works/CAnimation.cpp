/*
	MQOBone class
	2004/11/4	coded by KIOKU			since 2004/10/23
*/
#include "stdafx.h"
#include "CAnimation.h"
using namespace std;
CAnimation::CAnimation(void)
{

}

CAnimation::~CAnimation(void)
{
	anim_time.clear();
	anim.clear();
}

CAnimation::CAnimation(const CAnimation &obj)
{
 	anim = obj.anim;
	anim_time = obj.anim_time;
}

CAnimation &CAnimation::operator=(const CAnimation &obj)
{
	anim = obj.anim;
	anim_time = obj.anim_time;
	return *this;
}

float CAnimation::GetLastTime()
{
	return *(anim_time.end()-1);
}

CVector CAnimation::Spline2D(CVector* vec, float rate)
{
	CVector r;
	r = vec[0]*(0.5f*(1.0f-rate)*(1.0f-rate))
		+ vec[1]*((1.0f-rate)*rate + 0.5f)
		+ vec[2]*(0.5f*rate*rate);
	return r;
}

CVector CAnimation::CatmullRom(CVector* vec, float rate, long n)
{
	float t,t2;
	t=rate;
	t2=rate*rate;
	if(n==0){
		float rp[3];
		rp[0] =    t2-3*t+2;
		rp[1] = -2*t2+4*t  ;
		rp[2] =    t2-  t  ;
		CVector pos;
		pos = (vec[0]*rp[0]+vec[1]*rp[1]+vec[2]*rp[2])*0.5f;
		return pos;
	}else{
		float rp[3];
		rp[0] =    t2-t  ;
		rp[1] = -2*t2    +2;
		rp[2] =    t2+t  ;
		CVector pos;
		pos = (vec[0]*rp[0]+vec[1]*rp[1]+vec[2]*rp[2])*0.5f;
		return pos;
	}
}

KCloneData CAnimation::GetBoneTrans(unsigned long ci, float animation_time, int interpolate)
{
	KCloneData b;
	long tn=-1;
	if(animation_time<0) animation_time=0;
	long animation_num = (long)anim_time.size();
	if(animation_num==1){
		tn=0;
		b = anim[ci][tn];
	}else{
		vector<float>::iterator tit,tit_end=anim_time.end()-1;
		for(tit=anim_time.begin(); tit!=tit_end; tit++){
			if(((*tit)<=animation_time)&&(animation_time<=(*(tit+1)))){
				tn = (long)(tit - anim_time.begin());
				float rate = (animation_time - *tit)/(*(tit+1) - *tit);
				int db = anim.size();
				vector<KCloneData>::iterator it = anim[ci].begin();
				b.rot = (*(it+tn)).rot*(1.0f-rate) + (*(it+tn+1)).rot*rate;
				b.alpha = (*(it+tn)).alpha*(1.0f-rate) + (*(it+tn+1)).alpha*rate;
				b.scale = (*(it+tn)).scale*(1.0f-rate) + (*(it+tn+1)).scale*rate;
				if(animation_num==2 || interpolate==2){//linear
					b.pos = (*(it+tn)).pos*(1.0f-rate) + (*(it+tn+1)).pos*rate;
				}else{
					if(interpolate == 0){//catmul
						if(tn==0){
							CVector vec[3];
							vec[0] = (it)->pos;
							vec[1] = (it+1)->pos;
							vec[2] = (it+2)->pos;
							b.pos = CatmullRom(vec,rate,0);
						}else{
							CVector vec[3];
							vec[0] = (it+tn-1)->pos;
							vec[1] = (it+tn)->pos;
							vec[2] = (it+tn+1)->pos;
							b.pos = CatmullRom(vec,rate,2);
						}
					}else if(interpolate == 1){//spline
						float fn = (animation_num-1)*animation_time*2;//double rate
						long n = (long)fn;
						float mrate = fn - n;//cut floating point
						
						if( n == 0 ){//start linear
							CVector vec[3];
							vec[0] = (it+tn  )->pos;
							vec[1] = (it+tn+1)->pos;
							b.pos = vec[0]*(1.0f-mrate*0.5f) + vec[1]*mrate*0.5f;
						}else if (n == 2*animation_num-3){//end linear
							CVector vec[3];
							vec[0] = (it+tn  )->pos;
							vec[1] = (it+tn+1)->pos;
							b.pos = vec[0]*(0.5f-mrate*0.5f) + vec[1]*(mrate*0.5f + 0.5f);
						}else if (n >= 2*animation_num-2){//end
							b.pos = (it+tn+1)->pos;
						}else{//splines
							long odd = (n-1)/2;//odd point 
							CVector vec[3];
							vec[0] = (it+odd  )->pos;
							vec[1] = (it+odd+1)->pos;
							vec[2] = (it+odd+2)->pos;
						
							if((n%2)==1) b.pos = Spline2D(vec, mrate*0.5f);
							else		 b.pos = Spline2D(vec, mrate*0.5f+0.5f);
						}
					}
				}
				break;
			}
		}
	}
	if(tn==-1){//not found
		long ntim = (long)anim_time.size()-1;
		b.rot = (anim[ci][ntim]).rot;
		b.alpha = (anim[ci][ntim]).alpha;
		b.scale = (anim[ci][ntim]).scale;
		b.pos = (anim[ci][ntim]).pos;
	}
	return b;
}
