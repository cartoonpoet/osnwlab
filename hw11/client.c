#include <sys/socket.h>  /* 소켓 관련 함수 */
#include <arpa/inet.h>   /* 소켓 지원을 위한 각종 함수 */
#include <sys/stat.h>
#include <stdio.h>      /* 표준 입출력 관련 */
#include <string.h>     /* 문자열 관련 */
#include <unistd.h>     /* 각종 시스템 함수 */
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>

#define MAXLINE    1024

int main(int argc, char **argv)
{
    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
    char buf[MAXLINE];
    int maxfd=0;
    int fd_num;
    fd_set readfds, allfds;
    int sockfd; 

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {
        perror("error :");
        return 1;
    }

    /* 연결요청할 서버의 주소와 포트번호 프로토콜등을 지정한다. */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serveraddr.sin_port = htons(3600);

    client_len = sizeof(serveraddr);

    /* 서버에 연결을 시도한다. */
    if (connect(server_sockfd, (struct sockaddr *)&serveraddr, client_len)  == -1)
    {
        perror("connect error :");
        return 1;
    }

	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds); //버퍼 등록
	FD_SET(server_sockfd, &readfds); //서버 등록
	maxfd=server_sockfd;
while(1) 
{
    memset(buf, 0x00, MAXLINE);
    allfds=readfds;
    fd_num=select(maxfd+1, &allfds, (fd_set *)0, (fd_set *)0, NULL);
    
    if(strncmp(buf, "quit\n",5) == 0)
   	break;
    /*
    if (write(server_sockfd, buf, MAXLINE) <= 0) // 입력 받은 데이터를 서버로 전송한다.
    {
        perror("write error : ");
        return 1;
    }
    */
    /* 서버또는 버퍼로부터  데이터를 읽는다. */
    for(int i=0; i<=maxfd; i++){
	    sockfd=i;
	if(FD_ISSET(sockfd, &allfds)){
    		if(read(sockfd, buf, MAXLINE) <= 0){
       		 	perror("read error : ");
       			 return 1;
    		}
		else{
			if(sockfd==0){//버퍼로부터 들어온거면 서버에 전송
				if(write(server_sockfd, buf, MAXLINE)<=0){ //서버로 데이터 전송
					perror("write error : ");
					return 1;
				}
			}
			else if(sockfd>0){//서버로부터 받은 것
				printf("read : %s\n", buf);
			}
		}
		if(--fd_num<=0){
			break;
		}
	}
    }
    
}
    close(server_sockfd);
    return 0;
}

