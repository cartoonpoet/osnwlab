#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

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

	mkfifo("./myfifo", S_IRUSR|S_IWUSR);
	
	int fd, pipe_readn;

	if((fd=open("./myfifo", O_RDWR))==-1){
		perror("rfd error");
		return 0;
	}
	
	int acp_cnt=0;
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
		acp_cnt++;
		pid = fork();
		if(pid == 0)
		{
			close( listen_fd );
			memset(buf, 0x00, MAXLINE);
			char pipe_data[MAXLINE];
			memset(pipe_data, 0x00, MAXLINE);
			while((readn = read(client_fd, buf, MAXLINE)) > 0)
			{
				printf("Read Data %s(%d) : %s",
						inet_ntoa(client_addr.sin_addr),
						client_addr.sin_port,
						 buf);

				//printf("pid (%d) : %d\n", getpid(), acp_cnt);
				if(acp_cnt!=1){
					pipe_readn=read(fd, pipe_data, MAXLINE);
					buf[strlen(buf)-1]='\0';
					strcat(pipe_data, buf);
					write(fd, pipe_data, strlen(pipe_data));
					printf("%d : %s\n", getpid(), pipe_data);
				}
				else{
					buf[strlen(buf)-1]='\0';
					write(fd, buf, strlen(buf));
					printf("%d : %s\n", getpid(), buf);
				}
				
				while(1){
					pipe_readn=read(fd, pipe_data, MAXLINE);
					if(pipe_readn>12){	
						write(fd, pipe_data, strlen(pipe_data));
						write(client_fd, pipe_data, strlen(pipe_data));							break;
					}
					else{
						write(fd, pipe_data, strlen(pipe_data));	
					}						
				}

				if(pipe_readn>12){
					break;
				}
				memset(buf, 0x00, MAXLINE);
			}
			close(client_fd);
			return 0;
		}
		else if( pid > 0){
			close(client_fd);
		}
	}
	close(fd);
	return 0;
}

