enum { KDB_LOADER=0, KDB_MAIN, KDB_NUM };

#pragma pack(push,1)
typedef struct _tagKResourceDemoDat{
	int nYear;
	DWORD dwLength;
	char* szRc_DemoTitle;
	DWORD dwSize[KDB_NUM];
	unsigned char *kdb[KDB_NUM];
}KResourceDemoDat;
#pragma pack(pop)

unsigned long GetKResourceDemoDatSize(KResourceDemoDat* that){
	return (sizeof(*that) - sizeof(that->szRc_DemoTitle) - sizeof(that->kdb) + that->dwLength + that->dwSize[KDB_LOADER] + that->dwSize[KDB_MAIN]);
}
