#include <stdio.h>
#include <stdlib.h>
int main()
{
	char c;
	for(c='A'; c<='z'; c++){
		FILE* fp;
		char fn[64];
		sprintf(fn,"%c%d.ktf",c,c);
		if((fp=fopen(fn,"wt"))!=NULL){
			fprintf(fp,"KTF?,64,64;S,15;F,0,0,0,0;],\"Lucida Console\";E,0,0,0,255;D,0,0,250,0,0,0,\"%c\";",c);
			fclose(fp);	
		}
	}
}