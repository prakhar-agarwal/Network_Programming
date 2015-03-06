#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

void main (int argc, char *argv[])
{
	int k,j,pos=1,ret,n,status,level=0;
	n=atoi(argv[1]);
	printf("level\tpid\tppid\tpos\tdots\n");
	for(k=0;k<=n;k++){
        	for(j=1;j<=n-k;j++){
            		ret=fork();
            		if(ret==0)
                	break;		// Check of child process is created or not
        	}
        	if(ret!=0)
            	break;
        	if(k!=n){
            		pos=j;
            		level=k+1;
        	}
	}
	if(ret!=0 || level==n){
        for(k=1;k<=n-level;k++)
            wait(&status);
        printf("%d\t%d\t%d\t%d\t",level,getpid(),getppid(),pos);
        for(k=1;k<=pos;k++)
            printf("#");
        printf("\n");
	}
}
