/*
	MQOFile class
	2004/10/22	coded by KIOKU			since 2003/2/16
*/
#include "stdafx.h"
#include "MQO_File.h"
CMQO_File::CMQO_File()
{
	fin = NULL;
}



string CMQO_File::GetFileName()
{
	return mqo_filename;
}

long CMQO_File::Open(const string MQO_FileName)
{
	mqo_filename=MQO_FileName;
	if(fin!=NULL) delete fin;
	fin = new ifstream();
	fin->open(MQO_FileName.c_str());
	if(fin->is_open()==0){
		string mf = "cannot open > " + MQO_FileName;
		MessageBox(NULL,mf.c_str(),"error",MB_OK);
		return 1;
	}
	return 0;
}

void CMQO_File::Close()
{
	fin->close ();
	delete fin;
	fin = NULL;
}

int CMQO_File::CheckPhase()
{
	string buf;
	buf = GetData();
	if((buf=="}")||(buf=="{")){return 0;}
	else{
		buf = "not \"{\",\"}\" data error" + buf;
		MessageBox(NULL, buf.c_str(), "error", MB_OK);
		return 1;
	}
}
void CMQO_File::SkipChank()
{
	string buf;
	int chk=0;
	do{
		(*fin) >> buf;
		if(buf == "}")		chk--;
		else if(buf=="{") 	chk++;
	}while(chk>0);
}

void CMQO_File::SkipLine()
{
	char buf[1024];
	fin->getline(buf,1024);
}

string CMQO_File::GetData()
{
	const string sep = "() "; 
	static string buf;	
	if(buf.length()==0){
		(*fin)>>buf;
	}
	string data;
	long d=0;
	d = (long)buf.find_first_of(sep);
	if(d==-1){
		data = buf;
		buf="";
	}else{
		data = buf;
		data.erase(data.begin()+d,data.end());
		buf = buf.substr(d+1);
	}
	return data;
}

string CMQO_File::GetFileDir()
{
	string mq = mqo_filename;
	long last = (long)mq.find_last_of("/\\") + 1;
	mq.erase(mq.begin()+last,mq.end());
	return mq;
}
