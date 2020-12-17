#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>

#define MAXLINE 1024
#define PORTNUM 3600
#define SOCK_SETSIZE 1021

struct Node{
	int client_num;
	char data[MAXLINE];
	struct Node *next;
};

void print_node(struct Node *head){
	head=head->next;
	int st=1;
	while(head!=NULL){
		printf("%d번째 노드 %d : %s\n", st++, head->client_num, head->data);
		head=head->next;
	}
}

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

	int accept_cnt=1;//실제로 들어온 데이터 입력 카운트

	//고정 배열로하면 공간 낭비가 심하므로 연결리스트로 대체
	//각클라이언트로부터  데이터가 들어오는 순서대로 연결리스트에 추가한다.
	//각 클라이언트당 노드는 1개(데이터를 1개라도 보낸 클라이언트만 존재)를 보유한다.
	//각 클라이언트의 최신  데이터를 보관한다.
	struct Node *head=malloc(sizeof(struct Node));//머리노드
	struct Node *curr;
	struct Node *prev;
	
	while(1)
	{
		allfds = readfds;
	
		printf("Select Wait %d\n", maxfd);
		fd_num = select(maxfd + 1 , &allfds, (fd_set *)0, (fd_set *)0, NULL);
		if (FD_ISSET(listen_fd, &allfds)) //듣기 소켓에 이벤트 발생시
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
			if (FD_ISSET(sockfd, &allfds))
			{
				//printf("들어온 값%d\n", i);
				memset(buf, 0x00, MAXLINE);
				if (read(sockfd, buf, sizeof(buf)) <= 0)
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
						buf[strlen(buf)-1]='\0';
						printf("Read : %s\n", buf);
						
						curr=head->next;
						prev=head;
						while(1){
							if(curr==NULL){//없으면 처음 데이터가 들어온 것이므로 추가
								struct Node *add_node=malloc(sizeof(struct Node));
								add_node->next=NULL;
								add_node->client_num=sockfd;
								strcpy(add_node->data, buf);
								//printf("추가 후 : %s\n", add_node->data);
								prev->next=add_node;
								break;
							}
							else{
								if(sockfd==curr->client_num){//데이터가 이미 있는데 새로 들어온 경우 업데이트
									//printf("변경전 : %s\n", curr->data);
									strcpy(curr->data, buf);
									//printf("변경 후 : %s\n", curr->data);
									break;
								}
							}
							curr=curr->next;
							prev=prev->next;
						}
						
						//print_node(head);
							
						if(accept_cnt%3==0){ //실제로 데이터가 들어온 경우 3개 주기로 전송
							curr=head->next;
							char send_data[MAXLINE]; //전송할 데이터
							memset(send_data, 0x00, MAXLINE);
							int result=0;
							
							int cnt=1;
							char copy_data[MAXLINE];
							memset(copy_data, 0x00, MAXLINE);	
							while(curr!=NULL){
								strcpy(copy_data, curr->data);
								char *temp=strtok(copy_data, " ");//띄어쓰기 기준으로 자르기
								while(temp!=NULL){
									if(cnt%2==1){//문자
										strcat(send_data, temp);
										
									}
									else{//숫자
										
										result+=atoi(temp);	
									}
									cnt++;
									temp=strtok(NULL, " ");
								}
								curr=curr->next;
							}	
							//printf("조립 후 %s : %d\n", send_data, result);
							strcat(send_data, " ");
							char s1[100]; //변환한 문자열을 저장할 배열
							sprintf(s1, "%d", result);//정수를 문자열로 변환
							strcat(send_data, s1); //붙이기
							//printf("전송 전 :%s\n", send_data);
							for(int j=4; j<=maxfd; j++){
								write(j, send_data, strlen(send_data));
							}
					
						}
						
						//printf("fd %d\n", FD_ISSET(sockfd, &readfds));	
						//write(sockfd, buf, strlen(buf));
						accept_cnt++;
						
					}
				}
				if (--fd_num <= 0)
					break;
			}
		}
		
	}
}


