#ifndef KSYNTH_H
#define KSYNTH_H
//		Software Synthesizer  KSynth2 Engine
//		Coded by KIOKU	since 2003-2004
//
#include <stdio.h>
#include "kmath.h"
#include <windows.h>
#include <mmsystem.h>
#ifdef _MSC_VER
#pragma comment ( lib, "winmm" )
#endif

#define BUFFER_NUM				2
#define BUFFER_TIME				1764*2
#define WAVE_BUFFER_SIZE		BUFFER_TIME*sizeof(short)
#define OSC_SLOT_MAX				62
#define INST_SLOT_MAX				32
#define TRACK_MAX			      	62
#define TRACK_BASE_CHAR             'A'
#define WAVE_AMPLIFER_MAX		32767.0f*INST_SLOT_MAX

//======================Osc Class=============================================
class KOsc{
	private:
        float (*wfunc[6])(float t);
    public:
		//unsigned char wavetype;//0:sin 1:square 2:saw 3:tri 4:noise 5:null
        unsigned char wavetype;
		float waveamp;
		float wavefrq;
		float waveendfrq;
        float finefai;
        float wavefine;
		//unsigned char fmtype;//0:sin 1:square 2:saw 3:tri 4:noise 5:null
        unsigned char fmtype;
		float fmamp;
		float fmfrq;
		float fmendfrq;
        float fmdetune;

		unsigned char algorithm;
		static float GetOnkai(char c);
		static int charToCHAR(char* c);
		//void GetWavefunc(int wvtype, float(**wavefunc)(float));

		//delay
		long delaytime;
		float delaylevel;
		float* delaywave;
		//env
		float attacktime;
		float releasetime;
		float releaselevel;

        //filter
        float conductance;
        float hipass;
        float inductance;

        float instvol;
        
		unsigned char Enable;
		KOsc(){	Enable=0; };
        void InitWave(){
            wfunc[0]=&my_sin;
            wfunc[1]=&square;
            wfunc[2]=&saw;
            wfunc[3]=&tri;
            wfunc[4]=&my_sin;
            wfunc[5]=&nullfunc;
        }
		float GetWave(char key, float* WaveFai, float* FmFai, float rate);
  		float GetWave_old(char key, float* WaveFai, float* FmFai, float rate);
		void SetOsc(unsigned char Algorithm,
                    unsigned char WaveType, float WaveAmp, float WaveFrq, float WaveEndFrq,float WaveFine,
                    unsigned char FmType, float FmAmp, float FmFrq,float FmEndFrq, float FmDetune,
					float AttackTime, float ReleaseLevel, float ReleaseTime,float ivol);
		void SetFilter(float DelayTime, float DelayLevel, float cond, float hp, float indac);
		void Clean();
};

//=========================================================================
class KInst{
	public:
		KOsc* Inst;
		float fmtheata;
		float theata;
		char key;
		char* sequence;
        float cond;
        float ind;
};
class KSynth{
	private:
		WAVEHDR wh[BUFFER_NUM];
		HWAVEOUT hWOut;
		char* trackseq;
        float* WaveTemp[2];
		unsigned long music_starttime;

	public:
        KInst Inst[TRACK_MAX][INST_SLOT_MAX];
		KOsc Osc[OSC_SLOT_MAX];
		float charTime[TRACK_MAX];
		long seektime;
		long seektrack;
		unsigned char mode;
                unsigned char ks1mode;
		float vol;
		KSynth();
        void Init();
		void SetSequence(unsigned char track, unsigned char slot, unsigned char oscnum, char* seq);
		void SetTrack(char* Track);
		//void SetInst(unsigned char TrackNum, unsigned char InstNum, unsigned char OscNum);
		void SndRender(WAVEHDR* pwh, int nolink=0);
		void SndPlay(WAVEHDR* pwh);
		void Play();
        void InstInit();
		void Clean();
		void MusicOut(HANDLE hd, void (*func)(int));
		unsigned long WaveOut(HANDLE hd, void (*func)(int));
		void SetSeek(float offsettime);//adopted by c.r.v.
};

#endif