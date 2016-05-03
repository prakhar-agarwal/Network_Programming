#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<pthread.h>
#include<string.h>
#include<signal.h>

int SERVER_PORT;


struct thread_node{
	int thread_count;
	int thread_conn;
	pthread_t thread_id;
	char client_name[100];
}Thread_node;

socklen_t addr_len;
int Nthreads;
struct thread_node thread_connfd[100];
int listenfd;
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t access_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t thread_id;


void signal_handler(int signal_number){
	int i;
	for(i=0;i<Nthreads;i++){
		if(thread_connfd[i].thread_count == 1){
			pthread_kill(thread_connfd[i].thread_id, SIGKILL);
			thread_connfd[i].thread_count = 0;
			thread_connfd[i].thread_id = -1;
			sprintf(thread_connfd[i].client_name,"null");
		}
	}
	printf("\n\nAll the Threads have been terminated. All data structures cleaned.\nServer Shutting Down. Thank you for using PreThreaded Chat.\n\n");
	close(listenfd);
	exit(0);
	return;
}

int print_err(int connfd){
	char error_msg[] = "Sorry, the command you enetered is not supported by the server.\n";
	write(connfd, error_msg, strlen(error_msg));	
	printf("%s\n",error_msg);
	return 0;
}

int join(int connfd, char *cmd, int index){
	if(*cmd != ' '){
		char error_msg[] = "Sorry, the command you enetered is not supported by the server.\nFORMAT FOR JOIN: JOIN <name>\n";
	        write(connfd, error_msg, strlen(error_msg));
		printf("%s", error_msg);
		return 0;	
	}
	char *name = (cmd+1);
	//access to shared data
	pthread_mutex_lock(&access_mutex);
	sprintf(thread_connfd[index].client_name, "%s", name);
	pthread_mutex_unlock(&access_mutex);
	char resp[1000];
	char response[] = "Your nick name for the chat is: ";
	sprintf(resp, "%s%s\n", response, cmd+1);
	printf("%s",resp);
	write(connfd, resp, strlen(resp));
	return 0;
}

int list(int connfd, char *cmd){
        int i;
	
	char response[] = "Following users are currently online.\n";
	write(connfd, response, strlen(response));
	char names[100];
	printf("%s", response);
	pthread_mutex_lock(&access_mutex);
	for(i=0;i<Nthreads;i++){
		if(thread_connfd[i].thread_count == 1){
			sprintf(names, "%s\n", thread_connfd[i].client_name);
			write(connfd, names, strlen(names));
			memset(names,0,sizeof(names));
		}	
	}
	pthread_mutex_unlock(&access_mutex);
	return 0;
}

int umsg(int connfd, char *cmd, int index){
        if(*cmd != ' '){
                char error_msg[] = "Sorry, the command you enetered is not supported by the server.\nFORMAT FOR UMSG (User Message): UMSG <name>\\r\\n<message>\n";
                write(connfd, error_msg, strlen(error_msg));
                printf("%s", error_msg);
                return 0;
        }
	char *tname;
	tname = cmd+1;//strtok(cmd+1, "\\r\\n");
        char response[] = "Please type the msg to be sent to user: ";
	char resp[1000];
	sprintf(resp, "%s%s\n", response, tname);
	write(connfd, resp, strlen(resp));
        //read msg
	char msg[1000];
	int n = read(connfd, msg, sizeof(msg)-1);
	msg[n-1] = '\0';
	msg[n-2] = '\0';
	//char *msg;
	//msg = strtok(NULL, "\r\n");
	char msg_to_usr[1000];
	pthread_mutex_lock(&access_mutex);
	sprintf(msg_to_usr, "%s: %s\n", thread_connfd[index].client_name, msg);
	char sent[] = "Message sent. :)\n";
	char failed[] = "Message failed to send. User requested is offline. :(\n";
	
        printf("%s", msg_to_usr);
        int i;
	//pthread_mutex_lock(&access_mutex);
	for(i=0;i<Nthreads;i++){
                if(thread_connfd[i].thread_count == 1){
                        if(strcmp(thread_connfd[i].client_name, tname) == 0){
				write(thread_connfd[i].thread_conn, msg_to_usr, strlen(msg_to_usr));
				write(connfd, sent, strlen(sent));
				break;
			}
		}
        }
	pthread_mutex_unlock(&access_mutex);
        if(i == Nthreads){
		write(connfd, failed, strlen(failed));	
	}
	return 0;
}

int bmsg(int connfd, char *cmd, int index){
        if(*cmd != ' '){
                char error_msg[] = "Sorry, the command you enetered is not supported by the server.\nFORMAT FOR BMSG (Broadcast Message): BMSG <message>\n";
                write(connfd, error_msg, strlen(error_msg));
                printf("%s", error_msg);
                return 0;
        }

	char *msg;
	msg = cmd + 1;
	char msg_to_broadcast[1000];
	pthread_mutex_lock(&access_mutex);
	sprintf(msg_to_broadcast, "Broadcast message from user %s: %s\n", thread_connfd[index].client_name, msg);
	pthread_mutex_unlock(&access_mutex);
	int i;
        pthread_mutex_lock(&access_mutex);
	for(i=0;i<Nthreads;i++){
                if(thread_connfd[i].thread_count == 1){
                        write(thread_connfd[i].thread_conn, msg_to_broadcast, strlen(msg_to_broadcast));
                }
        }
	pthread_mutex_unlock(&access_mutex);
	return 0;
}

int leave(int connfd, int index){
        pthread_mutex_lock(&access_mutex);
	strcpy(thread_connfd[index].client_name,"");
	thread_connfd[index].thread_count = 0;
	thread_connfd[index].thread_conn = -1;
	pthread_mutex_unlock(&access_mutex);
	char exit_msg[] = "Thank you for joining the PreThreaded Chat! You are Welcome anytime. Its free and always will be, unless we have more than N members. :P\n";
	write(connfd, exit_msg, strlen(exit_msg));
	printf("%s", exit_msg);
	return 1;
}


int process_cmd(char *cmd, int connfd, int index){
	int n = 0;
	if(cmd == NULL){
		return 0;
	}
	if(strncmp(cmd, "JOIN", 4)==0 || strncmp(cmd, "join", 4)==0){
		n = join(connfd, cmd+4, index);
	}
	else if(strncmp(cmd, "LEAV", 4)==0 || strncmp(cmd, "leav", 4)==0){
		n = leave(connfd, index);
	}
	else{
		if(strlen(thread_connfd[index].client_name)>0){

			if(strncmp(cmd, "LIST", 4)==0 || strncmp(cmd, "list", 4)==0){
				n = list(connfd, cmd+4);
			}
			else if((strncmp(cmd, "UMSG", 4)==0 || strncmp(cmd, "umsg", 4)==0)){
				n = umsg(connfd, cmd+4, index);
			}
			else if((strncmp(cmd, "BMSG", 4)==0 || strncmp(cmd, "bmsg", 4)==0)){
				n = bmsg(connfd, cmd+4, index);
			}
			else if((strncmp(cmd, "LEAV", 4)==0 || strncmp(cmd, "leav", 4)==0)){
				n = leave(connfd, index);	
			}
			else{
				n = print_err(connfd);
			}
		}
		else{
			char pls_join[1000];
			sprintf(pls_join, "You have not joined the Prethreaded ChatServer. Please join the chat to continue. Join using following command:\nJOIN <name>\n");
			write(connfd, pls_join, strlen(pls_join));
			return 0;
		}
	}
	return n;
		
}
void process(int connfd, int index){
	//printf("In process for thread: %d. Connfd = %d\n", (int) pthread_self(), connfd);
	char buf[100];
	char *cmd;
	int n;
	int ret_val = 0;
	while(n = read(connfd, buf, sizeof(buf)-1)){
		buf[n] = '\0';
		printf("BUF READ: %s\n N = %d\n", buf,n);
		cmd = strtok(buf,"\r\n");
		printf("CMD 1 %s\n", cmd);
		ret_val = process_cmd(cmd,connfd,index);
		memset(buf,0,sizeof(buf));
		if(ret_val == 1){
			return;
		}
	}
	return;
}
void *thread_main(void *arg){
	//thread executing here.
	char welcome[1000];
	sprintf(welcome, "\nWelcome to PreThreaded Chat! The following are instructions to get you started.\n1. Join Chat: JOIN <name>\n2. View All users: LIST\n3. Send Private Message: UMSG <username>\r\n<message to send>\n4. Broadcast a message: BMSG <msg>\n5. Leave Chat: LEAV\n\nIts free and always will be.\n\n"); 
	int connfd;
	int thread_number = *((int *) arg);
	printf("In the thread: %d:: %d\n", (int) pthread_self(), thread_number);
	socklen_t client_addr_len;
	struct sockaddr *client_addr;
	client_addr = (struct sockaddr *) malloc(addr_len);
	printf("Listen FD: %d, Len: %d\n", (int)listenfd, addr_len);
	for(;;){
		pthread_mutex_lock(&mlock);
		connfd = accept(listenfd, client_addr, &addr_len);
		if(connfd == -1){
			printf("Cannot accept.\n");
			exit(1);
		}
		printf("Acc\n");
		pthread_mutex_unlock(&mlock);
		thread_connfd[thread_number].thread_count = 1;
		thread_connfd[thread_number].thread_conn = connfd;
		thread_connfd[thread_number].thread_id = pthread_self();
		write(connfd, welcome, strlen(welcome));
		process(connfd,thread_number);
		close(connfd);
	}	
}



void make_thread(int i){
	//create a thread in this function.
	printf("Creating Thread Number: %d.\n", i);	
	int *temp_ptr = (int *) malloc(sizeof(int));
	*temp_ptr = i;
	pthread_create(&thread_id, NULL, &thread_main, temp_ptr);
	return;
}


int main(int argc, char *argv[]){
	if(argc != 3){
		printf("RUNNING INSTRUCTIONS: ./a.out <N> <PORT NUMBER>. N is the number of threads. (N<100). Port Number should be a valid number. (Port Number > 1024)\n");
		exit(1);
	}
	printf("This server is running on port 5000. \n\n**********************RUNNING INSTRUCTIONS FOR CLIENT******************************\n\nRun this command on a seperate terminal: telnet localhost 5000\n\n");
	addr_len = sizeof(struct sockaddr);
	Nthreads = atoi(argv[1]);
	SERVER_PORT = atoi(argv[2]);
	printf("Number of threads: %d\n", Nthreads);
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGTSTP, signal_handler);
	if(listenfd == -1){
		printf("Sorry, cannot create a socket listenfd.\n");
		exit(1);
	}
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);
	
	int option =1;
	setsockopt(listenfd,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option));
	
	if(bind(listenfd,(struct sockaddr *) &server_addr,sizeof(server_addr) )==-1){
		printf("ERROR: %s\n", strerror(errno));
		exit(1);
	}
	listen(listenfd, 100);
	//create Nthreads;
	int i;
	for(i=0;i<Nthreads;i++){
		make_thread(i);	
	}
	for(;;){
		//pause();
	}
	return 0;
}
