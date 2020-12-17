#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
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

	int fds1[2], fds2[2]; //부모용, 자식용
	pipe(fds1);
	pipe(fds2);
	char result[30];
	int acp_cnt=0;
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
		acp_cnt++;
		pid = fork();
		if(pid == 0)
		{
			close( listen_fd );
			memset(buf, 0x00, MAXLINE);
			while((readn = read(client_fd, buf, MAXLINE)) > 0)
			{
				printf("%d : Read Data %s(%d) : %s",getpid(),
						inet_ntoa(client_addr.sin_addr),
						client_addr.sin_port,
						 buf);
				write(fds2[1], buf, sizeof(buf));
				read(fds1[0], buf, sizeof(buf));
				printf("%d 읽은거 : %s\n", getpid(), buf);
				write(client_fd, buf, sizeof(buf));
				//memset(buf, 0x00, MAXLINE);
			}
			write(fds1[1], buf, sizeof(buf));
			memset(buf, 0x00, MAXLINE);
			close(client_fd);
			exit(0);
		}
		else if( pid > 0){
			close(client_fd);
			read(fds2[0], buf, sizeof(buf));
			buf[strlen(buf)-1]='\0';
			strcat(result, buf);
			printf("부모가 %d : 합친후 : %s\n", getpid(), result);
			if(acp_cnt>2){
				for(int i=0; i<3; i++){
					write(fds1[1], result, sizeof(result));
				}
			}
			
		}
	}
	return 0;
}

