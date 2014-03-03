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


int main(int argc, char *argv[]){
	int sockfd = 0, n = 0;
	char recvBuff[1024];
	struct sockaddr_in serv_addr; 

	if(argc != 2){
		printf("\n Usage: %s <ip of server> \n",argv[0]);
		return 1;
	}

	memset(recvBuff, '0',sizeof(recvBuff));
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("\n Error : Could not create socket \n");
		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000); 

	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0){
		printf("\n inet_pton error occured\n");
		return 1;
	}

	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		printf("\n Error : Connect Failed \n");
		return 1;
	}
/*
	while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0){
        recvBuff[n] = 0;
        if(fputs(recvBuff, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
        }
    } 

    if(n < 0)
    {
        printf("\n Read error \n");
    } 
*/

	int LENGTH = 1024;
	char* f_name = "receive.pdf";
    FILE *fp = fopen(f_name, "a");
    if(fp == NULL) printf("File %s cannot be opened.\n", f_name);
    else
    {
        bzero(recvBuff, LENGTH);
        int f_block_sz = 0;
        int success = 0;
        while(success == 0)
        {
            while(f_block_sz = recv(sockfd, recvBuff, LENGTH, 0))
            {
                if(f_block_sz < 0)
                {
                    printf("Receive file error.\n");
                    break;
                }
                if(f_block_sz == 0)
					break;
                int write_sz = fwrite(recvBuff, sizeof(char), f_block_sz, fp);
                if(write_sz < f_block_sz)
                {
                    printf("File write failed.\n");
                    break;
                }
                bzero(recvBuff, LENGTH);
            }
            printf("ok!\n");
            success = 1;
            fclose(fp);
        }
    }
    int pid = fork();
    char *arg[4];
    arg[0] = "okular";
    arg[1] = "receive.pdf";
    arg[2] = NULL;
    arg[3] = NULL;
//    system("gnome->try.pdf");
    if(pid == 0){
		//execvp(arg[0],arg);
	}
	sleep(10);
	//system("xdotool getactivewindow windowmove 100 100");
    return 0;
}
/*

main()
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[100];

	/* Opening a socket is exactly similar to the server process */
//	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//		printf("Unable to create socket\n");
//		exit(0);
//	}

	/* Recall that we specified INADDR_ANY when we specified the server
	   address in the server. Since the client can run on a different
	   machine, we must specify the IP address of the server. 

	   TO RUN THIS CLIENT, YOU MUST CHANGE THE IP ADDRESS SPECIFIED
	   BELOW TO THE IP ADDRESS OF THE MACHINE WHERE YOU ARE RUNNING
	   THE SERVER. 

    	*/
//	serv_addr.sin_family		= AF_INET;
//	serv_addr.sin_addr.s_addr	= inet_addr("10.109.66.85");/*144.16.202.221*/
//	serv_addr.sin_port		= htons(5000);

	/* With the information specified in serv_addr, the connect()
	   system call establishes a connection with the server process.
	*/
/*	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		printf("Unable to connect to server\n");
		exit(0);
	}
*/
	/* After connection, the client can send or receive messages.
	   However, please note that recv() will block when the
	   server is not sending and vice versa. Similarly send() will
	   block when the server is not receiving and vice versa. For
	   non-blocking modes, refer to the online man pages.
	*/
/*	while(1){
	for(i=0; i < 100; i++) buf[i] = '\0';
	recv(sockfd, buf, 100, 0);
	printf("%s\n", buf);

	
	strcpy(buf,"Message from client");
	send(sockfd, buf, strlen(buf) + 1, 0);
	}
	while(1)
		sleep(1);
	close(sockfd);
}
*/
