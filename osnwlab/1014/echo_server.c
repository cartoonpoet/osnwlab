#include<sys/socket.h>
#include<sys/stat.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<string.h>
#include<pthread.h>

#define MAXBUF 1024
int main(int argc, char **argv){
	int server_sockfd, client_sockfd, client_len, n;
	char buf[MAXBUF];
	struct sockaddr_in clientaddr, serveraddr;
	client_len=sizeof(clientaddr);
	if((server_sockfd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))==-1){
		perror("socket error:");
		exit(0);
	}
	memset(&serveraddr, 0x00, sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY); //모든 아이피 허용
	serveraddr.sin_port=htons(atoi(argv[1])); 

	bind(server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(server_sockfd, 5);
	
	int cnt=0;//몇번째 들어왔는지 카운트
	char str1[30];//문자열 모을 변수
	int index=0; //배열 인덱스
	int client_ary[2]; //2개의 클라이언트 정보 담을 배열
	while(1){
		
		client_sockfd=accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);
		printf("New Client Connect: %s\n", inet_ntoa(clientaddr.sin_addr));
		memset(buf, 0x00, MAXBUF);
		if((n=read(client_sockfd, buf, MAXBUF))<=0){
			close(client_sockfd);
			continue;
		}
		cnt++;
		printf("receive : %s", buf);
		buf[strlen(buf)-1]='\0';
		strcat(str1, buf);
		if(cnt<3){
			//printf("들어옴\n");
			client_ary[index++]=client_sockfd;
			continue;
		}
		printf("합친거 : %s\n", str1);
		strcpy(buf, str1);
		
		for(int i=0; i<2; i++){
			write(client_ary[i], buf, MAXBUF);
			close(client_ary[i]);
		}

		if(write(client_sockfd, buf, MAXBUF)<=0){
			perror("write error:");
			close(client_sockfd);
		}
		close(client_sockfd);
	}
	close(server_sockfd);
	return 0;
}
