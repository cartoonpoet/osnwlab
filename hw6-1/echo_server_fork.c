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
#define PORTNUM 3600

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

	signal(SIGCHLD, SIG_IGN);
	int accept_cnt=0;
	char str_add[30];
	int client_ary[3];
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
		memset(buf, 0x00, MAXLINE);
		if((readn=read(client_fd, buf, MAXLINE))<=0){
			close(client_fd);
			continue;
		}
		client_ary[accept_cnt++]=client_fd;
		buf[strlen(buf)-1]='\0';
		strcat(str_add, buf);
		printf("Read Data : %s\n", buf);
		printf("%d프로세스가 합친 문자열 : %s\n", getpid() ,str_add);
		if(accept_cnt==3){
		pid = fork();
		if(pid == 0)
		{
			close( listen_fd );
			//write(client_fd, str_add, strlen(str_add));
			printf("%d프로세스가 문자열: %s : 전송\n ", getpid(),str_add);
			for(int i=0; i<3; i++){
				write(client_ary[i], str_add, strlen(str_add));
				close(client_ary[i]);
			}
			memset(buf, 0x00, MAXLINE);
			//close(client_fd);
			return 0;
		}
		else if( pid > 0)
			close(client_fd);
		}
	}
	return 0;
}

