#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
int main(int argc, char* argv[])
{        
	char pass[256];
        char fname[256];
	int accept;
	FILE *fptr,*log;
	if(!getenv("SSH_PASSFILE"))
	{
	   printf("can't getenv SSH_PASSFILE\n");
           return 1;
	}
	strcpy(fname,getenv("SSH_PASSFILE"));
	fptr=fopen(fname,"r");
	if(!fptr)
	{
	   printf("can't open %s for reading: %s\n",fname,strerror(errno));
           return 1;
	}
	fscanf(fptr,"%d %s",&accept,pass);
	fclose(fptr);
	if(argc>1)
	{
	   FILE* log=fopen(strcat(fname,".log"),"w");
	   if(log)	   
	   {
	       fprintf(log,"%s",argv[1]);
	       fclose(log);
	   }
	   if(strstr(argv[1],"RSA key"))
	   {
	       (accept)?printf("yes\n"):printf("no\n");
	   }
	}
	printf("%s\n",pass);
	return 0;
}
