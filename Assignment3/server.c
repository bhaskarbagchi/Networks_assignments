#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

int doSomething(int sock){
	char* fname = "doc1.pdf";
	char sendBuff[1025];
	FILE* fp = fopen(fname, "r");
	if(fp == NULL){
		printf("Error in fopen.\n");
		exit(0);
	}
	memset(sendBuff, '0', sizeof(sendBuff)); 
	int n = 0;
	while((n= fread(sendBuff, sizeof(char), 1024, fp))>0){
		if(send(sock, sendBuff, n, 0)<0){
			printf("Error in sending file.\n");
			break;
		}
		memset(sendBuff, '0', sizeof(sendBuff)); 
	}
	return 0; 
}

int main(int argc, char *argv[])
{
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr; 
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(5000); 

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
	printf("%d\n", listenfd);
	listen(listenfd, 10); 
	int pid;
	while(1)
	{
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		printf("\n\nConnection recieved %d\n\n\n", connfd);
		pid = fork();
		if(pid<0){
			exit(0);
		}
		else if(pid == 0){
			doSomething(connfd);
			exit(0);
		}
		else{
			close(connfd);
		}
		sleep(1);
	}
}
