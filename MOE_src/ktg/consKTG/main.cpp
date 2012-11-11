//#include <iostream>
#include <windows.h>
#include "../kLib/kTexture.h"

#pragma comment(linker, "/subsystem:windows")

//浮動小数点使用のため必要
#ifdef __cplusplus
extern "C" { 
#endif
	int _fltused=1; 
	void _cdecl _check_commonlanguageruntime_version(){}
#ifdef __cplusplus
}
#endif

#pragma comment(linker, "/entry:CodeStart")

int main();

void CodeStart()
{
	ExitProcess(main());
}

//using namespace std;
KTexture ktex;

const int nTextures = 1;
const char tf[nTextures][64] = {"F,255,255,255,255;S,0;E,0,0,0,0;C,0,55,255,144;"};
                         
int main()
{
    int i;
    //DWORD dwStart, dwEnd, dwElapsed = 0;
    
	//cout << "KTG in Console" << endl;
    for(i=0; i<nTextures; i++){
        //cout << "Generating texture no." << i << endl;
        
        //dwStart = timeGetTime();
        ktex.GenerateTexture(tf[i]);
        //dwEnd = timeGetTime();
        //dwElapsed += dwEnd - dwStart;    
        
        //cout << "Done in " << dwEnd - dwStart << "ms." << endl;
    }
    //cout << nTextures << " textures are Generated with no error in " << dwElapsed << "ms." << endl;
    return 0;
}