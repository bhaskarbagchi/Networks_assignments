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
	char buff[LENGTH];
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
	
	printf("Connected!!\n");

	int choice;
	choice = recieveMenu(sockfd);
	sprintf(buff, "%d", choice);
	send(sockfd, buff, strlen(buff), 0);
	
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
	char command[100];
	while(1){
	 	recv(sockfd, buff, LENGTH, 0);
	 	int width = atoi(buff);
	 	strcpy(buff, "Recieved!!");
	 	send(sockfd, buff, sizeof(buff), 0);
	 	recv(sockfd, buff, LENGTH, 0);
	 	int height = atoi(buff);
	 	strcpy(buff, "Recieved!!");
	 	send(sockfd, buff, sizeof(buff), 0);
	 	recv(sockfd, buff, LENGTH, 0);
	 	int zoom = atoi(buff);
	 	strcpy(buff, "Recieved!!");
	 	send(sockfd, buff, sizeof(buff), 0);
	 	recv(sockfd, buff, LENGTH, 0);
	 	int page = atoi(buff);
	 	strcpy(buff, "Recieved!!");
	 	send(sockfd, buff, sizeof(buff), 0);
		 	char zoom_str[100];
	 	memset(zoom_str, '\0', sizeof(zoom_str));
	 	char page_str[100];
	 	memset(page_str, '\0', sizeof(page_str));
	 	char temp[100];
	 	memset(temp ,'\0', sizeof(temp));
	 	while(zoom>0){
	 		int a = zoom%10;
	 		sprintf(temp,"%d %s", a, zoom_str);
	 		strcpy(zoom_str, temp);
	 		zoom/=10;
	 		memset(temp ,'\0', sizeof(temp));
	 	}
	 	memset(temp ,'\0', sizeof(temp));
	 	while(page>0){
	 		int a = page%10;
	 		sprintf(temp,"%d %s", a, page_str);
	 		strcpy(page_str, temp);
	 		page/=10;
	 		memset(temp ,'\0', sizeof(temp));
	 	}
		
	 	sprintf(command, "xdotool search --onlyvisible --name okular windowactivate");
	 	system(command);
	 	sprintf(command, "xdotool getactivewindow windowsize %d %d", height, width);
	 	system(command);
	 	sprintf(command, "xdotool key --delay 250 ctrl+f Escape ctrl+f Tab Tab Tab Tab Tab %s Return Escape", zoom_str);
	 	system(command);
	 	sprintf(command, "xdotool key --delay 250 ctrl+g %s Return", page_str);
	 	system(command);
	}
	close(sockfd);
	return 0;
}

int recieveMenu(int sockfd){
	char buff[LENGTH];
	memset(buff, '\0', sizeof(buff));
	int choice;
	int size = recv(sockfd, buff, LENGTH, 0);
	printf("%s", buff);
	memset(buff, '\0', sizeof(buff));
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
		recv(sockfd, buff, LENGTH, 0);
		printf("%s\n", buff);		
		bzero(buff, LENGTH);
		int f_block_sz = 0;
		int success = 0;
		while(success == 0){
			while(f_block_sz = recv(sockfd, buff, LENGTH, 0)){
				//printf("%s\n", buff);
				if(strcmp(buff, "Sent") == 0)// || f_block_sz<LENGTH)
					break;
								
				if(f_block_sz < 0)
				{
					printf("Receive file error.\n");
					exit(0);
				}
				if(f_block_sz == 0)
					break;
				buff[f_block_sz] = '\0';
				if(strcmp(buff, "Sent") == 0)// || f_block_sz<LENGTH)
					break;
				
				int write_sz = fwrite(buff, sizeof(char), f_block_sz, fp);
				if(write_sz < f_block_sz)
				{
					printf("File write failed.\n");
					exit(0);
				}
				memset(buff, '\0', sizeof(buff));
				strcpy(buff, "more");
				send(sockfd, buff, strlen(buff), 0);
				bzero(buff, LENGTH);
			}
			success = 1;
			fclose(fp);
			printf("File Recieve Complete!\n");
		}
		strcpy(buff, "Recieved!!!");
		send(sockfd, buff, strlen(buff), 0);
	}
	return;
}
