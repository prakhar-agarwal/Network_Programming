#include<signal.h>
#include<stdio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

main(int argc, char **argv){
	
    int p1[2],p2[2],p3[2],p4[2],child[2],pid,len;
    char buf[256];
    pipe(p1);                 
    pipe(p2);                 
    pipe(p3);                 
    pipe(p4);                 
    int i=0;
    while (i<2){
            pid = fork();
        if(pid==0){            
            break;
        }
        child[i] = pid;
        i++;
    }
    if(pid == 0){               
        if(i==0){               
            close(p1[0]);
            close(p2[1]);
            while(read(p2[0],&buf,sizeof(buf))){
                strcat(buf,"C1");          
                write(p1[1],&buf,sizeof(buf));
            }
            close(p1[1]);
            close(p2[0]);
        }
        if(i==1){               
            close(p3[0]);
            close(p4[1]);
            while(read(p4[0],&buf,sizeof(buf))){
                strcat(buf,"C2");
                write(p3[1],&buf,sizeof(buf));
            }
            close(p3[1]);
            close(p4[0]);
        }
    }
    else{
        close(p2[0]);
        close(p1[1]);
        close(p4[0]);
        close(p3[1]);
        memset(buf,'\0',sizeof(buf));
        while(read(0,&buf,sizeof(buf))){        
            buf[strlen(buf)-1]='\0';           
            write(p2[1],&buf,sizeof(buf));
            read(p1[0],&buf,sizeof(buf));
            write(p4[1],&buf,sizeof(buf));
            read(p3[0],&buf,sizeof(buf));
            printf("%s\n\n",buf);
            memset(buf,'\0',sizeof(buf));
        }
        close(p2[1]);
        close(p1[0]);
        close(p4[1]);
        close(p3[0]);
 
    }
}
