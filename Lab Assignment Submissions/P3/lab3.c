#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <signal.h>
#define M_SIZE     1256
typedef struct msgbuf {
         long    mtype;
         char    mtext[M_SIZE];
         } message_buf;
void message( int k, char *buf);
void recv ( int k, char *buf);
void childProcess(int i);
void handler(int sig);
void parentHandler(int sig);
int totalSignals = 0;
pid_t parent;
int L;
int main () {
	char tempBuff[50];
	int i,j;
	pid_t pid;
	scanf("%d",&L);
	signal (SIGINT, parentHandler);

	parent = getpid();
	for(i=0;i<3;i++) {
		if(pid=fork()==0) {
			tempBuff[0] = '\0';
			sprintf(tempBuff,"%d",getpid());
			j = 5660 + i;
			message(j,tempBuff);
			childProcess(i);
			exit(0);
		}
	}
	while(1) {
		pause();
	}
	return 0;
}
void message( int k, char *buf)
{
	int msqid;
	int msgflg = IPC_CREAT | 0666;
	key_t key;
	key = k;
	message_buf sbuf;
	size_t buf_length;
	if ((msqid = msgget(key, msgflg )) < 0) {
  		exit(1);
    	}
	else {      
	    sbuf.mtype = 1;
    	}
	strcpy(sbuf.mtext,buf);
	buf_length = strlen(sbuf.mtext) + 1 ;
	if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {        	
        	perror("msgsnd");
        	exit(1);
    	}
}
void recv ( int k , char * buf) {
	int i = 0;
   	int msqid;
   	key_t key;
   	key = k;
   	message_buf  rbuf;
    
    while(1) {
    	if ((msqid = msgget(key, 0666)) < 0) {    		
			//perror("msgget");
    	}
		if (msgrcv(msqid, &rbuf, M_SIZE, 1, 0) < 0)
		{
     		//perror("msgrcv");
		}
     	else 
     		break;
     	}
		
	strcpy(buf,rbuf.mtext);

	}

void childProcess(int childNum) {

	if( childNum == 0 ) {
		signal (SIGUSR1, handler);
		while(1){
			int pid;
			char tempBuff[50];
			recv(5661,tempBuff);
			pid = atoi(tempBuff);
			sprintf(tempBuff,"%d",pid);
			message(5661,tempBuff);
			//printf("CHILD 1 %d %d %d\n",pid,getpid(),getppid());
			sleep(3);
			kill(getpid(),SIGUSR1);
			//printf("%d %d %d\n",childNum,getpid(),getppid());
			//signal (SIGINT, int_handler);
		}
	}

	else if ( childNum == 1 ) {
		signal (SIGUSR1, handler);
		while(1){
			int pid;
			char tempBuff[50];
			recv(5662,tempBuff);
			pid = atoi(tempBuff);
			sprintf(tempBuff,"%d",pid);
			message(5662,tempBuff);
			//printf("CHILD 2 %d %d %d\n",pid,getpid(),getppid());
			sleep(6);
			kill(getpid(),SIGUSR1);			
			//printf("%d %d %d\n",childNum,getpid(),getppid());
			//signal (SIGINT, int_handler);
		}
	}

	else if ( childNum == 2 ) {
		signal (SIGUSR2, handler);
		while(1){
			int pid;
			char tempBuff[50];
			recv(5660,tempBuff);
			pid = atoi(tempBuff);
			sprintf(tempBuff,"%d",pid);
			message(5660,tempBuff);
			//printf("CHILD 56 %d %d %d\n",child[0],getpid(),getppid());
			sleep(9);
			kill(getpid(),SIGUSR2);			
			//printf("%d %d %d\n",childNum,getpid(),getppid());
		}
	}
}

void handler(int signo) {
	printf("SIG NO. %d  PID - %d\n",signo,getpid());
	kill(parent,SIGINT);
}

void parentHandler(int signo){
	totalSignals++;
	char tempBuff[100];
	int pid;
	//printf("Total Signals : %d\n",totalSignals);
	if(totalSignals==L+3) {
		tempBuff[0] = '\0';
		recv(5661,tempBuff);
		pid = atoi(tempBuff);
		sprintf(tempBuff,"%d",getpid());
		message(5661,tempBuff);
		printf("Killing Child 2 %d\n",pid);
		kill(pid,SIGTERM);
		recv(5660,tempBuff);
		pid = atoi(tempBuff);
		sprintf(tempBuff,"%d",getpid());
		message(5660,tempBuff);
		printf("Killing Child 1 %d\n",pid);
		kill(pid,SIGTERM);
		recv(5662,tempBuff);
		pid = atoi(tempBuff);
		sprintf(tempBuff,"%d",getpid());
		message(5662,tempBuff);
		printf("Killing Child 3 %d\n",pid);
		kill(pid,SIGTERM);
		//sleep(1);
		kill(parent,SIGKILL);
	}
}
