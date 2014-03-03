#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#define LENGTH 1024

//Function prototypes
int recieveMenu(int sockfd);
void recievePDF(int sockfd);

//Main funstion
int main(int argc, char* argv[]){
	int sockfd = 0, n = 0;
	char buff[1025];
	memset(buff, '0', sizeof(buff));
	struct sockaddr_in serv_addr;
	
	if(argc != 2){
		printf("\nUSAGE: %s <ip of server>\n", argv[0]);
		return 1;
	}
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
		printf("\nERROR: Could not create socket.\n");
		return 1;
	}
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr	= inet_addr(argv[1]);
	serv_addr.sin_port = htons(4000);

	if((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0){
		printf("\n Error : Connect Failed \n");
		return 1;
	}
	
	int choice;
	choice = recieveMenu(sockfd);
	sprintf(buff, "%d", choice);
	send(sockfd, buff, strlen(buff) + 1, 0);
	
	recievePDF(sockfd);
	
	printf("File transfer complete.\n");
	
	//open with okular
	int pid = fork();
	char *arg[3];
	arg[0] = "okular";
	arg[1] = "receive.pdf";
	arg[2] = NULL;
	if(pid == 0){
		execvp(arg[0], arg);
	}
	
	/*
	 * In infinite loop
	 * Collect ocular information
	 * use xdotool to set them
	 */
	
	close(sockfd);
	return 0;
}

int recieveMenu(int sockfd){
	char buff[LENGTH];
	memset(buff, '0', sizeof(buff));
	int choice;
	int size = recv(sockfd, buff, LENGTH, 0);
	buff[size] = '\0';
	printf("%s", buff);
	memset(buff, '0', sizeof(buff));
	printf("Select a session to join:");
	scanf("%d", &choice);
	return choice;
}

void recievePDF(int sockfd){
	printf("Ready to recieve file!\n");
	char buff[LENGTH];
	char* f_name = "receive.pdf";
	FILE *fp = fopen(f_name, "w");
	if(fp == NULL){
		printf("File %s cannot be opened.\n", f_name);
		exit(0);
	}
	else{
		bzero(buff, LENGTH);
		int f_block_sz = 0;
		int success = 0;
		while(success == 0){
			while(f_block_sz = recv(sockfd, buff, LENGTH, 0)){
				if(f_block_sz < 0)
				{
					printf("Receive file error.\n");
					exit(0);
				}
				if(f_block_sz == 0)
					break;
				buff[f_block_sz] = '\0';
				if(strcmp(buff, "Sent") == 0 || f_block_sz<LENGTH)
					break;
				
				int write_sz = fwrite(buff, sizeof(char), f_block_sz, fp);
				if(write_sz < f_block_sz)
				{
					printf("File write failed.\n");
					exit(0);
				}
				bzero(buff, LENGTH);
			}
			success = 1;
			fclose(fp);
			printf("File Recieve Complete!\n");
		}
		strcpy(buff, "Recieved!!!");
		send(sockfd, buff, sizeof(buff), 0);
	}
	return;
}
