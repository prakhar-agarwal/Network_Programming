#include<signal.h>
#include<stdio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

main(int argc, char *argv[]){
    int i,fd1r[2],fd1w[2],fd2r[2],fd2w[2],cpid[2],pid,len;
    char buf[256];
    pipe(fd1r);                 //Pipe to read from Child 1
    pipe(fd1w);                 //Pipe to write to Child 1
    pipe(fd2r);                 //Pipe to read from Child 2
    pipe(fd2w);                 //Pipe to write to Child 2
    for(i=0;i<2;i++){
        pid = fork();
        if(pid==0){             //Child breaks out of loop
            break;
        }
        cpid[i] = pid;          //Parent copies child pid into array
    }

    if(pid == 0){               //Child
        if(i==0){               //Child 1
            close(fd1r[0]);
            close(fd1w[1]);
            while(read(fd1w[0],&buf,sizeof(buf))){
                strcat(buf,"C1");            //Child 1 adds " C1" to string
                write(fd1r[1],&buf,sizeof(buf));
            }
            close(fd1r[1]);
            close(fd1w[0]);
        }
        if(i==1){               //Child 2
            close(fd2r[0]);
            close(fd2w[1]);
            while(read(fd2w[0],&buf,sizeof(buf))){
                strcat(buf,"C2");            //Child 2 adds " C2" to string
                write(fd2r[1],&buf,sizeof(buf));
            }
            close(fd2r[1]);
            close(fd2w[0]);
        }
    }
    else{                       //Parent
        close(fd1w[0]);
        close(fd1r[1]);
        close(fd2w[0]);
        close(fd2r[1]);
        memset(buf,'\0',sizeof(buf));
        while(read(0,&buf,sizeof(buf))){        //Read from stdin till user closes pipe by Ctrl+D
            buf[strlen(buf)-1]='\0';            //End the input string before the endline character
            write(fd1w[1],&buf,sizeof(buf));
            read(fd1r[0],&buf,sizeof(buf));
            write(fd2w[1],&buf,sizeof(buf));
            read(fd2r[0],&buf,sizeof(buf));
            printf("%s\n",buf);
            memset(buf,'\0',sizeof(buf));
        }
        close(fd1w[1]);
        close(fd1r[0]);
        close(fd2w[1]);
        close(fd2r[0]);
    }
}
