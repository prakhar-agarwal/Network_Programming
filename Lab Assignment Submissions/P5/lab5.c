#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>

#define SHM_KEYSTRING "PrakharAgarwal.c"
#define SEM_KEY_C1 (27220141)
#define SEM_KEY_C2 (27220142)
#define SEM_KEY_C3 (27220123)
#define TRUE 1
#define FALSE 0
#define MAX_STRING_LENGTH 1000

void parentTerminateChildren(int);
void childExit(int);

pid_t c1, c2, c3;

int main() {

	
	int c1Sem = semget(SEM_KEY_C1, 1, IPC_CREAT | 0666);
	int c2Sem = semget(SEM_KEY_C2, 1, IPC_CREAT | 0666);
	int c3Sem = semget(SEM_KEY_C3, 1, IPC_CREAT | 0666);
	struct sembuf sb;

	c3 = fork();
	if(c3 == 0) {
		
		semctl(c3Sem, 0, SETVAL, 1);

	
		key_t key = ftok(SHM_KEYSTRING, 'R');
		int shmid = shmget(key, 1024, IPC_CREAT | 0644);
		char *data = shmat(shmid, NULL, 0); 
		
		while(TRUE) {
			sb.sem_num = 0;
			sb.sem_op = -1;
			sb.sem_flg = 0;
			semop(c3Sem, &sb, 1);

			char string[MAX_STRING_LENGTH];
			if(scanf("%s", string) < 0) {
				break;
			}

			int len = strlen(string);
			string[len] = '\0';
			int i;
			for(i = 0; i <= len; i++) {
				data[i] = string[i];
			}

			sb.sem_num = 0;
			sb.sem_op = 1;
			sb.sem_flg = 0;
			semop(c2Sem, &sb, 1);
		}

		kill(getppid(), SIGUSR1);

		exit(0);
	} else if (c3 > 0) {
		c2 = fork();
		if (c2 == 0) {
			signal(SIGUSR2, childExit);
			semctl(c2Sem, 0, SETVAL, 0);
			key_t key = ftok(SHM_KEYSTRING, 'R');
			int shmid = shmget(key, 1024, IPC_CREAT | 0644);
			char *data = shmat(shmid, NULL, 0);
			while(TRUE) {
				sb.sem_num = 0;
				sb.sem_op = -1;
				sb.sem_flg = 0;
				semop(c2Sem, &sb, 1);

				int len = strlen(data);
				int i;
				for(i = 0; i < len; i++) {
					data[i] = data[i] + ('A' - 'a');
				}
				data[i] = '\0';

				sb.sem_num = 0;
				sb.sem_op = 1;
				sb.sem_flg = 0;
				semop(c1Sem, &sb, 1);
			}

			exit(0);
		} else if (c2 > 0) {
			c1 = fork();
			if(c1 == 0) {

				signal(SIGUSR2, childExit);
				semctl(c1Sem, 0, SETVAL, 0);
				
				key_t key = ftok(SHM_KEYSTRING, 'R');
				int shmid = shmget(key, 1024, IPC_CREAT | 0644);
				char *data = shmat(shmid, NULL, 0);

				while(TRUE) {
					sb.sem_num = 0;
					sb.sem_op = -1;
					sb.sem_flg = 0;
					semop(c1Sem, &sb, 1);

					printf("%s\n", data);

					sb.sem_num = 0;
					sb.sem_op = 1;
					sb.sem_flg = 0;
					semop(c3Sem, &sb, 1);
				}

				exit(0);
			} else {

				signal(SIGUSR1, parentTerminateChildren);
				int status;
				waitpid(c1, &status, 0);
				waitpid(c2, &status, 0);
				waitpid(c3, &status, 0);

				return 0;
			}
		}
	}

	return 0;
}

void parentTerminateChildren(int sigNo) {
	if(sigNo == SIGUSR1) {
		kill(c2, SIGUSR2);
		kill(c3, SIGUSR2);
	}
}

void childExit(int sigNo) {
	exit(0);
}
