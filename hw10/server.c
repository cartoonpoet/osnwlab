#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAXLINE 1024
#define PORTNUM 3500

//뮤텍스 선언
pthread_mutex_t mutex_lock;

struct data{
	char str[MAXLINE];
	int num;
	int read_check; // 0-> note read, 1-> already read
};
struct MultipleArg{
	int listen_fd;
	int client_fd;
};
void *Producer(void *arg){
	struct MultipleArg *client_info=(struct MultipleArg*)arg;
	struct data buf;

	memset(&buf, 0x00, MAXLINE); //초기화
	read(client_info->client_fd, &buf, sizeof(buf)); //정보 읽기
	printf("Read Data : %s and %d\n", buf.str, buf.num); //출력

	//공유 메모리
	void * shared_memory=NULL;
	int shmid=shmget((key_t)(int) client_info->client_fd, sizeof(buf), 0666|IPC_CREAT); //공유메모리생성
	struct data *temp;
	shared_memory=(struct data *)shmat(shmid, NULL, 0);
	temp=(struct data *)shared_memory;
	*temp=buf;
	temp->read_check=1;
	while(1){
		pthread_mutex_lock(&mutex_lock);
		if(temp->read_check==1){//소비자가 공유메모리를 읽었다면
			temp->num++;

			//문자 시프트
			char temp_num=0;
			for(int i=0; i<strlen(temp->str); i++){
				if(i==0) temp_num=temp->str[0];
				if(temp->str[i+1]!=NULL){
					temp->str[i]=temp->str[i+1];
				}
				else{
					temp->str[i]=temp_num;
				}
			}
			buf.num=temp->num;
			temp->read_check=0;
			printf("생산 %u : %s and %d\n", (int)pthread_self(), temp->str, temp->num);
		}
		pthread_mutex_unlock(&mutex_lock);
		sleep(1);
	}
}

void *Consumer(void *arg){
	struct MultipleArg *client_info=(struct MultipleArg*)arg;
	
	//공유 메모리
	void * shared_memory=NULL;
	int shmid=shmget((key_t)(int) client_info->client_fd, sizeof(struct data), 0666|IPC_CREAT);//공유메모리가져오기
	shared_memory=(struct data *)shmat(shmid, NULL, 0);//공유메모리 첨부
	struct data *read_data=(struct data *)shared_memory;
	int client_fd=client_info->client_fd;

	while(1){
		pthread_mutex_lock(&mutex_lock);
		if(read_data->read_check==0){//아직 안읽은 거면
			printf("소비 %u : %s and %d\n", (int)pthread_self(), read_data->str, read_data->num);
			write(client_fd, read_data, sizeof(struct data));
			read_data->read_check=1;
		}
		pthread_mutex_unlock(&mutex_lock);
		sleep(1);
	}
}

int main(int argc, char **argv)
{
	int listen_fd, client_fd;
	pid_t pid;
	socklen_t addrlen;
	int readn;
	char buf[MAXLINE];
	struct sockaddr_in client_addr, server_addr;

	if( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return 1;
	}
	memset((void *)&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNUM);

	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==-1)
	{
		perror("bind error");
		return 1;
	}
	if(listen(listen_fd, 5) == -1)
	{
		perror("listen error");
		return 1;
	}

	struct MultipleArg *multiple_arg;
	multiple_arg=(struct MultipleArg *)malloc(sizeof(struct MultipleArg));
	pthread_mutex_init(&mutex_lock, NULL); //뮤텍스 초기화
	signal(SIGCHLD, SIG_IGN);
	while(1)
	{
		addrlen = sizeof(client_addr);
		client_fd = accept(listen_fd,
			(struct sockaddr *)&client_addr, &addrlen);
		if(client_fd == -1)
		{
			//printf("accept error\n");
			perror("accept error");
			break;
		}
		
		multiple_arg->client_fd=client_fd;
		multiple_arg->listen_fd=listen_fd;
		
		pthread_t thread[2];
		pthread_create(&thread[0], NULL, Producer, multiple_arg); //생산자
		pthread_create(&thread[1], NULL, Consumer, multiple_arg); //소비자`
	}
	return 0;
}

