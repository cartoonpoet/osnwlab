#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAXLINE 1024
#define PORTNUM 3600
#define SOCK_SETSIZE 1021
struct data{
	char str[MAXLINE];
	int num;
};
int main(int argc, char **argv)
{
	int listen_fd, client_fd;
	socklen_t addrlen;
	int fd_num;
	int maxfd = 0;
	int sockfd;
	int i= 0;
	char buf[MAXLINE];
	fd_set readfds, allfds;

	struct sockaddr_in server_addr, client_addr;

	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket error");
		return 1;
	}   
	memset((void *)&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNUM);
	
	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("bind error");
		return 1;
	}   
	if(listen(listen_fd, 5) == -1)
	{
		perror("listen error");
		return 1;
	}   
	
	FD_ZERO(&readfds); //fd_set 초기화 (0으로 채운다)
	FD_SET(listen_fd, &readfds); //듣기소켓 추가

	maxfd = listen_fd; //검사할 비트 테이블 크기 설정
	int accept_cnt=0; //접속 클라이언트 수
	int sockfd_ary[3];	//3개 클라이언트 번호 담기
	struct data combine_data;
	memset(&combine_data, 0x00, sizeof(combine_data));
	while(1)
	{
		allfds = readfds;
		printf("Select Wait %d\n", maxfd);
		fd_num = select(maxfd + 1 , &allfds, (fd_set *)0,
					  (fd_set *)0, NULL);
		
		if (FD_ISSET(listen_fd, &allfds)) //듣기 소켓으로부터 데이터가 있다면
		{
			addrlen = sizeof(client_addr);
			client_fd = accept(listen_fd,
					(struct sockaddr *)&client_addr, &addrlen);

			FD_SET(client_fd,&readfds);

			if (client_fd > maxfd)
				maxfd = client_fd;
			printf("Accept OK\n");
			
			continue;
		}
		
		for (i = 0; i <= maxfd; i++)
		{
			sockfd = i;
			//printf("이벤트 확인%d : %d\n", i, FD_ISSET(sockfd, &allfds));
			if (FD_ISSET(sockfd, &allfds))
			{
				memset(buf, 0x00, MAXLINE);
				if (read(sockfd, buf, MAXLINE) <= 0)
				{
					close(sockfd);
					FD_CLR(sockfd, &readfds);
				}
				else
				{
					if (strncmp(buf, "quit\n", 5) ==0)
					{
						close(sockfd);
						FD_CLR(sockfd, &readfds);
					}
					else
					{
						accept_cnt++;
						sockfd_ary[accept_cnt%3]=sockfd;
						printf("Read : %s", buf);
						char temp[MAXLINE];
						strcpy(temp, buf);
						char *ptr=strtok(temp, " ");
						int n=0;
						while(ptr!=NULL){
							if(n==0){
								strcat(combine_data.str, ptr);
							}
							else{
								combine_data.num+=atoi(ptr);
							}
							ptr=strtok(NULL, " ");
							n++;
						}
						if(accept_cnt%3==0){
							buf[0]='\0';//초기화
							strcat(buf, combine_data.str);
							strcat(buf, " ");
							char trans[MAXLINE];
							sprintf(trans, "%d", combine_data.num);
							strcat(buf, trans);
							printf("보내기 전 : %s\n", buf);
							//write(sockfd, buf, strlen(buf));
							
							for(int a=0; a<3; a++){
								write(sockfd_ary[a], buf, strlen(buf));
							}

							combine_data.num=0;
							combine_data.str[0]='\0';
						}
					}
				}
				if (--fd_num <= 0){
				//	printf("%d : 종료\n", i);
					break;
				}
			}
		}
	}
}


