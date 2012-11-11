#include "stdafx.h"
#include "ks2lib.h"

void KOsc::Clean()
{
	if(delaywave!=NULL) GlobalFree(delaywave);
}

void KOsc::SetOsc(unsigned char Algorithm,
				  unsigned char WaveType, float WaveAmp, float WaveFrq, float WaveEndFrq, float WaveFine,
				  unsigned char FmType,   float FmAmp,   float FmFrq,float FmEndFrq, float FmDetune,
				  float AttackTime, float ReleaseLevel, float ReleaseTime,float ivol)
{
    InitWave();
	Enable=1;
	algorithm = Algorithm;
	wavetype = WaveType;
	waveamp = WaveAmp;
	wavefrq = WaveFrq;
	waveendfrq = WaveEndFrq;
    wavefine = WaveFine;
    finefai=0.0f;
	fmtype = FmType;
	fmamp = FmAmp;
	fmfrq = FmFrq;
	fmendfrq = FmEndFrq;
    fmdetune = FmDetune;
    delaywave=NULL;
	attacktime=AttackTime;
	releasetime=ReleaseTime;
	releaselevel=ReleaseLevel;
   	delaytime=0;
	delaylevel=0.0f;
    conductance=0.0f;
    hipass=0.0f;
    inductance=0.0f;
    instvol=ivol;
}

void KOsc::SetFilter(float DelayTime, float DelayLevel, float cond, float hp, float indac)
{
	delaytime=(long)(DelayTime*44100.0f);
	delaylevel=DelayLevel;
	if(delaywave!=NULL) GlobalFree(delaywave);
    if(delaytime!=0) delaywave=(float*)GlobalAlloc(GPTR,sizeof(float)*delaytime);

    conductance = cond;
    hipass = hp;
    inductance = indac;
}

float KOsc::GetOnkai(char c)
{
    if(c=='A') return 0.5946f;//ド
    else if(c=='W') return 0.6230f;//ド＃
    else if(c=='S') return 0.6674f;//レ
    else if(c=='E') return 0.7071f;//レ＃
    else if(c=='D') return 0.7492f;//ミ
    else if(c=='F') return 0.7937f;//ファ
    else if(c=='T') return 0.8409f;//ファ＃
    else if(c=='G') return 0.8909f;//ソ
    else if(c=='Y') return 0.9439f;//ソ＃
    else if(c=='H') return 1.0f;//ラ
    else if(c=='U') return 1.0595f;//ラ＃
    else if(c=='J') return 1.1225f;//シ
    else if(c=='K') return 1.1892f;//ド
    else if(c=='O') return 1.2599f;//ド＃
    else if(c=='L') return 1.3348f;//レ
    else if(c=='P') return 1.4142f;//レ＃
    else if(c==';') return 1.4983f;//ミ
    return 0.0f;
}

int KOsc::charToCHAR(char* c)
{
    if(('a'<=*c)&&(*c<='z')){//大文字判定
        *c -= 0x20;
        return false;//小文字
    }
    return true;//大文字
}

float KOsc::GetWave_old(char key, float* WaveFai, float* FmFai, float rate)
{
	float onkai=1.0f;
	if(charToCHAR(&key)) onkai*=2.0f;
	onkai *= GetOnkai(key);
	
	if(onkai!=0.0f){
        float fw,w;
        if(fmtype==4) fw = (fmfrq*(1.0f-rate)+fmendfrq*rate)*4.0f*noise(0.0f);//FM基本位相
        else          fw = (fmfrq*(1.0f-rate)+fmendfrq*rate)*4.0f;//FM基本位相

        if(wavetype==4) w = (wavefrq*(1.0f-rate)+waveendfrq*rate)*4.0f*noise(0.0f);//WAVE基本位相
        else           	w = (wavefrq*(1.0f-rate)+waveendfrq*rate)*4.0f;//WAVE基本位相

		if(algorithm!=1)  w*=onkai;//0||2
		if(algorithm!=0) fw*=onkai;//1||2
		*FmFai=(float)((long)(*FmFai+fw)%176400);
		*WaveFai=(float)((long)(*WaveFai+w)%176400);

		const float aw=PI2/176400.0f;
		return (*wfunc[wavetype])(*WaveFai*aw+(*wfunc[fmtype])(*FmFai*aw)*fmamp)*waveamp;
	}else{
		*FmFai=*WaveFai=0.0f;
		return 0.0f;
	}
}

float KOsc::GetWave(char key, float* WaveFai, float* FmFai, float rate)
{
	float onkai=1.0f;
	if(charToCHAR(&key)) onkai=2.0f;
	onkai *= GetOnkai(key);

	if(onkai!=0.0f){
        float fw,w;
        if(fmtype==4) fw = ((-fmfrq+fmendfrq)*rate-fmfrq)*noise(0.0f);//FM基本位相
        else          fw = ((-fmfrq+fmendfrq)*rate-fmfrq);//FM基本位相

        if(wavetype==4) w = ((-wavefrq+waveendfrq)*rate+wavefrq)*noise(0.0f);//WAVE基本位相
        else           	w = ((-wavefrq+waveendfrq)*rate+wavefrq);//WAVE基本位相

        w += wavefine;//fine

        fw*=5.9443083900f;
        w*=5.9443083900f;
		if(algorithm!=1)  w*=onkai;//0||2
		if(algorithm!=0) fw*=onkai;//1||2
		*FmFai=(float)((long)(*FmFai+fw)&262143);
		*WaveFai=(float)((long)(*WaveFai+w)&262143);

		const float aw=(PI2/262144.0f);
		if(algorithm!=3) return (*wfunc[wavetype])(*WaveFai*aw+(*wfunc[fmtype])(*FmFai*aw)*fmamp)*waveamp;
        else             return (*wfunc[wavetype])(*WaveFai*aw)*waveamp+(*wfunc[fmtype])(*FmFai*aw + 176400.0f*fmdetune)*fmamp*1000.0f;
	}else{
		*FmFai=*WaveFai=0.0f;
		return 0.0f;
	}
}

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, KSynth* syn, WAVEHDR* pwh, DWORD dwParam2)
{
    if(uMsg==MM_WOM_DONE){//MM_WOM_DONE
        if(syn->mode==1){
            syn->SndRender(pwh);
            syn->SndPlay(pwh);
        }
    }
}

#pragma warning( disable : 4311)
KSynth::KSynth(){
    Init();
}

void KSynth::Init(){
	int i,s;
	//init system member
	mode=0;
    ks1mode=0;
	seektime=0;
	seektrack=0;
	trackseq=NULL;

	//init private member
	vol=1.0f;

	for(s=0; s<TRACK_MAX; s++){
		charTime[s]=44100.0f*0.25f;//1/4[sec]
		for(i=0; i<INST_SLOT_MAX; i++){
			Inst[s][i].Inst=NULL;
			Inst[s][i].fmtheata=0.0f;
			Inst[s][i].theata=0.0f;
			Inst[s][i].sequence=NULL;
            Inst[s][i].cond=0.0f;
            Inst[s][i].ind=0.0f;

		}
	}

	//init wave system
	WAVEFORMATEX wf;
	wf.wFormatTag=WAVE_FORMAT_PCM;
    wf.nChannels=1;
    wf.nSamplesPerSec=44100;
    wf.wBitsPerSample=16;
    wf.nBlockAlign=wf.nChannels*wf.wBitsPerSample/8;
    wf.nAvgBytesPerSec=wf.nSamplesPerSec*wf.nBlockAlign;
    wf.cbSize=0;

	waveOutOpen(&hWOut,WAVE_MAPPER,&wf,(DWORD)waveOutProc,(DWORD)this,CALLBACK_FUNCTION);//warning trap

	for(i=0; i<BUFFER_NUM; i++){
		wh[i].lpData=(char*)GlobalAlloc(GPTR,WAVE_BUFFER_SIZE);
		wh[i].dwBufferLength=WAVE_BUFFER_SIZE;
		wh[i].dwBytesRecorded=0;
		wh[i].dwUser=0;
		wh[i].dwFlags=0;
		wh[i].dwLoops=1;
        wh[i].lpNext = (i!=(BUFFER_NUM-1)) ? &wh[i+1] : &wh[0];
		wh[i].reserved=0;
	}
    WaveTemp[0]=(float*)GlobalAlloc(GPTR,BUFFER_TIME*sizeof(float));
	WaveTemp[1]=(float*)GlobalAlloc(GPTR,BUFFER_TIME*sizeof(float));
}
#pragma warning( default : 4311)

void KSynth::InstInit()
{
    int i,s;
	//init system member
	mode=0;
    for(s=0; s<OSC_SLOT_MAX; s++){
        if(Osc[s].delaywave!=NULL){
            GlobalFree(Osc[s].delaywave);
            Osc[s].delaywave=NULL;
        }
    }


	for(s=0; s<TRACK_MAX; s++){
		for(i=0; i<INST_SLOT_MAX; i++){
			Inst[s][i].Inst=NULL;
			Inst[s][i].fmtheata=0.0f;
			Inst[s][i].theata=0.0f;
			if(Inst[s][i].sequence!=NULL){
                GlobalFree(Inst[s][i].sequence);
                Inst[s][i].sequence=NULL;
                Inst[s][i].cond=0.0f;
                Inst[s][i].ind=0.0f;
            }
		}
	}
}

void KSynth::SetTrack(char* Track)
{
	if(trackseq!=NULL) GlobalFree(trackseq);
	trackseq = (char*)GlobalAlloc(GPTR,lstrlen(Track)+1);
	lstrcpy(trackseq, Track);
}

void KSynth::Clean(){
    int i,s;
    mode=0;
    waveOutReset(hWOut);
    for(i=0;i<BUFFER_NUM; i++){
        waveOutUnprepareHeader(hWOut,&wh[i],sizeof(WAVEHDR));
        GlobalFree(wh[i].lpData);
    }
    waveOutClose(hWOut);
	
	for(i=0; i<OSC_SLOT_MAX; i++){
		Osc[i].Clean();
	}
	for(s=0; s<TRACK_MAX; s++){
		for(i=0; i<INST_SLOT_MAX; i++){
			if(Inst[s][i].sequence!=NULL){
                GlobalFree(Inst[s][i].sequence);
                Inst[s][i].sequence=NULL;
            }
		}
	}
	if(trackseq!=NULL) GlobalFree(trackseq);
    if(WaveTemp[0]!=NULL) GlobalFree(WaveTemp[0]);
	if(WaveTemp[1]!=NULL) GlobalFree(WaveTemp[1]);
}

//void KSynth::SetInst(unsigned char TrackNum, unsigned char InstNum, unsigned char OscNum)
//{
	//Inst[TrackNum][InstNum]=&Osc[OscNum];
//}

void KSynth::SetSequence(unsigned char TrackNum, unsigned char InstNum, unsigned char OscNum, char* seq)
{
    Inst[TrackNum][InstNum].Inst=&Osc[OscNum];//SetInst
	if(Inst[TrackNum][InstNum].sequence!=NULL) GlobalFree(Inst[TrackNum][InstNum].sequence);
    Inst[TrackNum][InstNum].sequence = (char*)GlobalAlloc(GPTR,lstrlen(seq)+1);
	lstrcpy(Inst[TrackNum][InstNum].sequence, seq);
}


/*
void KSynth::SndRender(WAVEHDR* pwh)
{


	int t,i;
    short* Buf = (short*)pwh->lpData;
	//float wavetemp[BUFFER_TIME];
	float* wavetemp;
	if(&wh[0]==pwh) wavetemp=WaveTemp[0];
	else		   wavetemp=WaveTemp[1];
	int tnum=trackseq[seektrack]-TRACK_BASE_CHAR;
	long maxChar=60000;
	for(i=0;i<INST_SLOT_MAX; i++){
		if(Inst[tnum][i].sequence!=NULL){
			if(maxChar>lstrlen(Inst[tnum][i].sequence)) maxChar=lstrlen(Inst[tnum][i].sequence);
		}
	}


	for(t=0;t<BUFFER_TIME; t++){
        Buf[t]=0;
		//End Check
		float nch = seektime/charTime[tnum];
		int ch = (int)(nch);
		nch-=ch;//separate floating point
		if(ch==maxChar){//last char > end			
			seektrack++;
			if(trackseq[seektrack]=='\0'){
				mode=0;//End
				return;
			}else{
				tnum=trackseq[seektrack]-TRACK_BASE_CHAR;//next track
                int btum = trackseq[seektrack-1] - TRACK_BASE_CHAR;
				seektime=0;
				ch=0;
				nch=nch*charTime[tnum]/charTime[btum];
			}
		}


		float wave=0.0f;
		for(i=0;i<INST_SLOT_MAX; i++){
			KInst* tInst = &Inst[tnum][i];
			KOsc* tOsc = tInst->Inst;
			if(tOsc!=NULL){
                //-key
                if(tInst->sequence==NULL) return;
                char k=tInst->sequence[ch];
                if(k!='-')tInst->key = k;

				//env
                float env=1.0f;
	    	    float bnch=nch;//frq sustaion for frq
                if(k!='-'){
					if(tOsc->attacktime>nch){//attack
			    		env=nch/tOsc->attacktime;
    				}else if(tOsc->releasetime>nch){//decay
	    				env*=((tOsc->releasetime-nch)/(tOsc->releasetime-tOsc->attacktime)
														*(1.0f-tOsc->releaselevel)+tOsc->releaselevel);
		    		}else{
                        env*=((tOsc->releasetime-nch)/(tOsc->releasetime-tOsc->attacktime)
														*(1.0f-tOsc->releaselevel)+tOsc->releaselevel);
                        if(tInst->sequence[ch+1]!='-'){//release
			    	        float fenv=(1.0f-nch)/(1.0f-tOsc->releasetime);
    				        env*=(fenv*tOsc->releaselevel);
                            nch=tOsc->releasetime;//frq sustaion for frq
                        }
	    			}
                }else{
		    		if((tInst->sequence[ch+1]!='-')&&(tOsc->releasetime<nch)){
                        float fenv=(1.0f-nch)/(1.0f-tOsc->releasetime);
	    			    env*=(fenv*tOsc->releaselevel);
    				}else{//continue '-'
                        env*=tOsc->releaselevel;
                        nch=tOsc->releasetime;//frq sustaion for frq

                    }
                }

				float wv = env*tOsc->GetWave(tInst->key, &tInst->theata,&tInst->fmtheata,nch);


                nch=bnch;//reset

				//filter
                tInst->cond=tInst->cond + (1.0f-tOsc->conductance)*(wv-tInst->cond);
                wv = tInst->cond;
                wv -=(tOsc->hipass)*(tInst->cond);
                if(tOsc->inductance>0.0f){
                    tInst->ind = -(wv - tInst->ind)/(1.0f+tOsc->inductance);
                    wv = tInst->ind;

                }
   				wave += wv;

				if(tOsc->delaywave!=NULL){//delay
					long dp = seektime%(tOsc->delaytime);
					wave += tOsc->delaylevel*tOsc->delaywave[dp];
					tOsc->delaywave[dp] = wv;
				}
			}
		}
		seektime++;

		if(wave> WAVE_AMPLIFER_MAX) wave= WAVE_AMPLIFER_MAX;
		else if(wave<-WAVE_AMPLIFER_MAX) wave=-WAVE_AMPLIFER_MAX;
		wavetemp[t] = wave;//*0.25f;

		//filter
		//cutoff

		if(wavetemp[t]> 32767.0f) wavetemp[t]= 32767.0f;
		else if(wavetemp[t]<-32767.0f) wavetemp[t]=-32767.0f;
		Buf[t] = (short int)(vol*wavetemp[t]);
	}
	//GlobalFree(wavetemp);
	/*
   	//filter
    for (t=0; t<BUFFER_TIME-1;t++){
        //fbuf[i+1] = fbuf[i+1] - gbuf[i] - lbuf[i];
    	//gbuf[i+1] = gbuf[i] + (hpass*hpass)*0.5f/44100.0f*(fbuf[i] + fbuf[i+1]);
        //lbuf[i+1] = lbuf[i] + lpass/300.0f*(fbuf[i+1] - fbuf[i]);
        if(fbuf[i] > 32767) fbuf[i]= 32767;
        if(fbuf[i] <-32767) fbuf[i]=-32767;
        //delay
        if(t-Osc[i].delaytime>=0) wavetemp[t] += Osc[i].delaytime/32767.0f*fbuf[t-(int)DTime[SlotNum]];

        Buf[i] = (short int)(vol*fbuf[i]);
    }*/

	//time offset
	//seektime+=BUFFER_TIME;
//}


void KSynth::SndRender(WAVEHDR* pwh,int nolink)
{
	//link
	if(nolink==0){
		long tseektime;
		long tseektrack=0;      
		{
			unsigned long ntm=timeGetTime();
			float offsettime = (float)(ntm - music_starttime);
			long i;
			offsettime*=44.1f;
			while(trackseq[tseektrack]!='\0'){
				long amaxChar=60000;
				int tnum=trackseq[tseektrack] - TRACK_BASE_CHAR;
				for(i=0;i<INST_SLOT_MAX; i++){
					if(Inst[tnum][i].sequence!=NULL){
						if(amaxChar>lstrlen(Inst[tnum][i].sequence)) amaxChar=lstrlen(Inst[tnum][i].sequence);
					}
				}
				float tm = amaxChar*charTime[tnum];
				if((offsettime-tm)<0) break;
				else offsettime-=tm;
				tseektrack++;
			}
			tseektime=(long)(offsettime);
		}
		if(tseektrack>seektrack){
			seektime=tseektime;
			seektrack=tseektrack;
		}else if((tseektrack==seektrack)&&(seektime<tseektime)){
			seektime=tseektime;
		}
	}
	int t,i;
    short* Buf = (short*)pwh->lpData;
    float* wavetemp;
	if(&wh[0]==pwh) wavetemp=WaveTemp[0];
	else		    wavetemp=WaveTemp[1];
	int tnum=trackseq[seektrack]-TRACK_BASE_CHAR;
	long maxChar=60000;
	for(i=0;i<INST_SLOT_MAX; i++){
		if(Inst[tnum][i].sequence!=NULL){
			if(maxChar>lstrlen(Inst[tnum][i].sequence)) maxChar=lstrlen(Inst[tnum][i].sequence);
		}
	}

	
	for(t=0;t<BUFFER_TIME; t++){
        Buf[t]=0;
		//End Check
		float nch = seektime/charTime[tnum];
		int ch = (int)(nch);
		nch-=ch;//separate floating point
		if(ch==maxChar){//last char > end			
			seektrack++;
			if(trackseq[seektrack]=='\0'){
				mode=0;//End
				return;
			}else{
				tnum=trackseq[seektrack]-TRACK_BASE_CHAR;//next track
                int btum = trackseq[seektrack-1] - TRACK_BASE_CHAR;
				seektime=0;
				ch=0;
				nch=nch*charTime[tnum]/charTime[btum];
			}
		}


		float wave=0.0f;
		for(i=0;i<INST_SLOT_MAX; i++){
			KInst* tInst = &Inst[tnum][i];
			KOsc* tOsc = tInst->Inst;
			if(tOsc!=NULL){
                //-key
                if(tInst->sequence==NULL) return;
                char k=tInst->sequence[ch];
                if(k!='-')tInst->key = k;


				//env
                float wv;
                float reltime = tOsc->releasetime - 0.0001f;
                float attime = tOsc->attacktime + 0.0001f;
                float env=1.0f;
	    	    float bnch=nch;//frq sustaion for frq
                if(k!=' '){//if no seqence then skip envelope and wave gen
                    if(k=='-'){
                        if((tInst->sequence[ch+1]!='-')&&(reltime<nch)){
                            float fenv=(1.0f-nch)/(1.0f-reltime);
	    			        env*=(fenv*tOsc->releaselevel);
    				    }else{//continue '-'
                            env*=tOsc->releaselevel;
                            nch=reltime;//frq sustaion for frq
                        }
                    }else{
		    		    if(attime>nch){//attack
			    	    	env=nch/attime;
    				    }else if(reltime>nch){//decay
	    				    env*=((reltime-nch)/(reltime-attime)
                                *(1.0f-tOsc->releaselevel)+tOsc->releaselevel);
		    		    }else{
                            env*=((reltime-nch)/(reltime-attime)
								*(1.0f-tOsc->releaselevel)+tOsc->releaselevel);
                           if(tInst->sequence[ch+1]!='-'){//release
			    	            float fenv=(1.0f-nch)/(1.0f-reltime);
    				            env*=(fenv*tOsc->releaselevel);
                                nch=reltime;//frq sustaion for frq
                            }
	    			    }
                    }
                    if(ks1mode==0){
                        wv = env*tOsc->GetWave(tInst->key, &tInst->theata,&tInst->fmtheata,nch);
                    }else{
                        wv = env*tOsc->GetWave_old(tInst->key, &tInst->theata,&tInst->fmtheata,nch);
                    }
                }else{
                    wv = 0;
                }
                
                nch=bnch;//reset

				//filter
                if(ks1mode==0){
                    wv = DENORMALIZE(wv);
                    if(tOsc->inductance>0.0f){
                        tInst->ind = -(wv - tInst->ind)/(1.0f+tOsc->inductance);
                        tInst->ind = DENORMALIZE(tInst->ind);
                        wv = tInst->ind;
                    }

                    tInst->cond=tInst->cond + (1.0f-tOsc->conductance)*(wv-tInst->cond);
                    tInst->cond = DENORMALIZE(tInst->cond);
                    wv = tInst->cond;
                    wv -=((tOsc->hipass)*(tInst->cond));

                    if(tOsc->delaywave!=NULL){//delay
   	    			    long dp = seektime%(tOsc->delaytime);
    	    			wv -= (tOsc->delaylevel*tOsc->delaywave[dp]);
                        wv = DENORMALIZE(wv);
                        tOsc->delaywave[dp] = wv;
                        while((wv>32767.0f)||(wv<-32767.0f)){
                            if(wv> 32767.0f) wv = 2.0f*32767.0f - wv;
    	                    else if(wv<-32767.0f) wv=-2.0f*32767.0f - wv;
                        }
    	    		}
                }else{
                    float dwv=0;
                    wv = DENORMALIZE(wv);
                    tInst->cond = tInst->cond + (1.0f-tOsc->conductance)*(wv-tInst->cond);
                    tInst->cond = DENORMALIZE(tInst->cond);
                    wv = tInst->cond;
                    wv -=(tOsc->hipass)*(tInst->cond);
                    if(tOsc->inductance>0.0f){
                        tInst->ind = -(wv - tInst->ind)/(1.0f+tOsc->inductance);
                        tInst->ind = DENORMALIZE(tInst->ind);
                        wv = tInst->ind;
                    }
                    dwv += wv;

			    	if(tOsc->delaywave!=NULL){//delay
				    	long dp = seektime%(tOsc->delaytime);
					    dwv += tOsc->delaylevel*tOsc->delaywave[dp];
                        dwv = DENORMALIZE(dwv);
    					tOsc->delaywave[dp] = wv;
	    			}

                    wv = dwv;
                }

                wave += (tOsc->instvol*wv);
			}
		}
		seektime++;

		if(wave> WAVE_AMPLIFER_MAX) wave= WAVE_AMPLIFER_MAX;
		else if(wave<-WAVE_AMPLIFER_MAX) wave=-WAVE_AMPLIFER_MAX;
		wavetemp[t] = wave;//*0.25f;

        //while((wavetemp[t]>32767.0f)||(wavetemp[t]<-32767.0f)){
		//    if(wavetemp[t]> 32767.0f) wavetemp[t] = 2.0f*32767.0f - wavetemp[t];
		//    else if(wavetemp[t]<-32767.0f) wavetemp[t]=-2.0f*32767.0f - wavetemp[t];
        //}
        if(wavetemp[t]> 32767.0f) wavetemp[t] = 32767.0f;
        else if(wavetemp[t]<-32767.0f) wavetemp[t]=-32767.0f;
		Buf[t] = (short int)(vol*wavetemp[t]);
	}

}


void KSynth::SndPlay(WAVEHDR* pwh){
    waveOutPrepareHeader(hWOut,pwh,sizeof(WAVEHDR));
    waveOutWrite(hWOut,pwh,sizeof(WAVEHDR));
}

void KSynth::Play()
{
	if(mode!=1){
		mode=1;
		music_starttime=timeGetTime()-(long)(seektime/44.1f);
        SndRender(&wh[0]);
		SndPlay(&wh[0]);
		SndRender(&wh[1]);
		SndPlay(&wh[1]);
	}
}

struct mks_wavehead{
	char cRIFF[4];
	unsigned long filesize;
	char cWAVE[4];
    char cfmt[4];
	unsigned long fmtsize;
	unsigned short id,ch;
	unsigned long sample_rate,bytepersec;
	unsigned short bytepersamp, bitpersamp;
    char cdata[4];
	unsigned long wavesize;
}whf;

void KSynth::MusicOut(HANDLE hd, void (*func)(int))
{
	unsigned long sz;
	
    lstrcpy(whf.cRIFF,"RIFF");
    lstrcpy(whf.cWAVE,"WAVE");
    lstrcpy(whf.cfmt,"fmt ");
    whf.fmtsize=16;
    whf.id = 1;
    whf.ch = 1;
    whf.sample_rate = 44100;
    whf.bytepersec = 44100*2*1;
    whf.bytepersamp = 2*1;
    whf.bitpersamp = 16;
    lstrcpy(whf.cdata,"data");
    //fwrite(&whf,sizeof(mks_wavehead),1,fp);
	WriteFile(hd,&whf,sizeof(mks_wavehead),&sz,NULL);
    unsigned long wsize = WaveOut(hd,func);
    whf.wavesize = wsize;
    whf.filesize = wsize+sizeof(mks_wavehead)-8;
    //fseek(fp,0,SEEK_SET);
	SetFilePointer(hd,0,NULL,FILE_BEGIN);
    //fwrite(&whf,sizeof(mks_wavehead),1,fp);
	WriteFile(hd,&whf,sizeof(mks_wavehead),&sz,NULL);
}

unsigned long KSynth::WaveOut(HANDLE hd, void (*func)(int))
{
	unsigned long sz;
	unsigned long wsample=0;
    long wn=1;
	mode=1;
	while(mode==1){
        SndRender(&wh[wn], 1);
		WriteFile(hd,wh[wn].lpData, WAVE_BUFFER_SIZE,&sz,NULL);
        //fwrite(wh[wn].lpData, WAVE_BUFFER_SIZE,1,wavefile);
        wn = (++wn)&1;
        wsample += (WAVE_BUFFER_SIZE);

		//int rt = (int)(wsample/(float)MUSIC_SIZE*1000.0f);
		//func(rt);
	}
    return wsample;
}

void KSynth::SetSeek(float offsettime)
{
	long i;
	seektrack=0;
	offsettime*=44.1f;
	while(trackseq[seektrack]!='\0'){
		long maxChar=60000;
		int tnum=trackseq[seektrack] - TRACK_BASE_CHAR;
		for(i=0;i<INST_SLOT_MAX; i++){
			if(Inst[tnum][i].sequence!=NULL){
				if(maxChar>lstrlen(Inst[tnum][i].sequence)) maxChar=lstrlen(Inst[tnum][i].sequence);
			}
		}
		float tm = maxChar*charTime[tnum];
		if((offsettime-tm)<0) break;
		else offsettime-=tm;
		seektrack++;
	}
	seektime=(long)(offsettime);
}