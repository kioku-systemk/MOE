/*
	MQOFile class
	2004/10/22	coded by KIOKU			since 2003/2/16
*/
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <windows.h>
#include "../klib/vector.h"

using namespace std;

#define		MQO2DX_RATE		0.01f		//MQOfile vertex*RATE = DX vertex

class stFace{
	public:
	long num_v;
	long v[4];
	long mat;
	float uv[8];
};

class CMQO_File{
	public:
		CMQO_File();
	protected:
		string mqo_filename;
		ifstream* fin;
		string fbuf;
		string GetFileName();
		long Open(const string MQO_FileName);
		void Close();
		void SkipChank();
		void SkipLine();
		string GetData();
		string GetFileDir();
		int CheckPhase();
};
