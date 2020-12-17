#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
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
union semun{
	int val;
};
struct data{
	char str[MAXLINE];
	int num;
};

int main(int argc, char **argv)
{
	int listen_fd, client_fd;
	pid_t pid;
	socklen_t addrlen;
	int readn;
	struct sockaddr_in client_addr, server_addr;

	struct data buf;

	struct sembuf semopen={0, -1, SEM_UNDO};
	struct sembuf semclose={0, 1, SEM_UNDO};

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

	signal(SIGCHLD, SIG_IGN);
	while(1)
	{
		addrlen = sizeof(client_addr);
		client_fd = accept(listen_fd,
			(struct sockaddr *)&client_addr, &addrlen);
		if(client_fd == -1)
		{
			printf("accept error\n");
			break;
		}
		pid = fork();
		if(pid == 0)
		{
			//공유 메모리 부분
			void * shared_memory=NULL;
			int ppid=getpid();
			int shmid=shmget((key_t)(int) getpid(), sizeof(buf), 0666|IPC_CREAT); //공유메모리 생성
			shared_memory=(struct data *)shmat(shmid, NULL, 0);
			struct data *temp;
			int fork_cnt=0;

			//세마포어 부분
			union semun sem_union;
			int semid=semget((key_t)(int) getpid(), 1, IPC_CREAT|0666);
			sem_union.val=1;
			semctl(semid, 0, SETVAL, sem_union);

			close( listen_fd );
			memset(&buf, 0x00, sizeof(buf));
				
			readn = read(client_fd, &buf, sizeof(buf));
			
				printf("Read Data %s(%d) : %s and %d\n",
						inet_ntoa(client_addr.sin_addr),
						client_addr.sin_port,
						 buf.str, buf.num);
			
			
			temp=(struct data *)shared_memory;
			*temp=buf;
			while(1){
				
				temp->num++; //1증가

				char temp_num=0;
				for(int i=0; i<strlen(temp->str); i++){//문자 시프트
					if(i==0) temp_num=temp->str[0];
				       if(temp->str[i+1] != NULL){
				       		temp->str[i]=temp->str[i+1];
				       }
				       else{
				       		temp->str[i]=temp_num;
				       }
				       	       
				}
				printf("%d : %c\n", temp->num, temp->str[0]);
				
				if(fork_cnt==0){
					fork_cnt++;
					fork();
					shmid=shmget((key_t)(int)getppid(), sizeof(buf), 0);
					shared_memory=(struct data *)shmat(shmid, NULL, 0);

					struct data *read_data=(struct data *)shared_memory;
					while(ppid==getppid()){
						printf("자식 프로세스 : %s : %d\n", read_data->str, read_data->num);
						write(client_fd, read_data, sizeof(buf));
						sleep(1);
					}
				}

				sleep(1);		
		
			}

				write(client_fd, &buf, sizeof(buf));
				//memset(buf, 0x00, MAXLINE);
			
			close(client_fd);
			return 0;
		}
		else if( pid > 0)
			close(client_fd);
	}
	return 0;
}

