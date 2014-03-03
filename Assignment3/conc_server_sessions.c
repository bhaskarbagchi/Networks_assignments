#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 1024

//Datatype to store linked list of all clients
typedef struct _client_list_node{
	int client_fd;
	struct _client_list_node* next;
} client_list_node;

//Datatype to store the list, by storing the head and tail of list
typedef struct _client_list{
	client_list_node* head;
	client_list_node* tail;
}client_list;

//Client list for each session
client_list list[10];

//Mutex for each individual session client list
pthread_mutex_t list_mutex[10];


//Function prototypes
void *handover(void* args);
void *handleSession(void* args);

//Main function begins
int main(int arcg, char* argv[]){
	int count = 0, rc;

	//Create sessions: create threads
	for(count = 0; count<10; count++){
		pthread_mutex_init(&list_mutex[count], NULL);
			list[count].head = NULL;
			list[count].tail = NULL;
	}
	
	pthread_t thread[10];
	pthread_t *create;
	/*for(count = 0; count<10; count++){
		rc = pthread_create(&thread1[count], NULL, handleSession, (void *)count);
		if(rc){
			printf("Error in thread creation.\n");
			exit(0);
		}
	}*/
	
	//Create server
	int sock_fd, new_sock_fd;
	int clilen;
	struct sockaddr_in client_addr, server_addr;
	int i, pid;
	char buff[LENGTH];
	if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Cannot create socket.\n");
		exit(0);
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(4000);
	
	if(bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		printf("Unable to bind to local address.\n");
		exit(0);
	}
	listen(sock_fd, 10);
	
	while(1){
		clilen = sizeof(client_addr);
		new_sock_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &clilen);
		if(new_sock_fd < 0){
			printf("Accept error.\n");
			exit(0);
		}
		create = (pthread_t *)malloc(sizeof(pthread_t));
		i = pthread_create(create, NULL, handover, (void *)new_sock_fd);
		if(i){
			printf("Error in thread creation.\n");
			exit(0);
		}
	}
	return 0;
}

//Function definitions

//Function to handover incomming client to respective client list
void *handover(void* args){
	char buff[LENGTH];
	int new_sock_fd = (int)args;
	int session;
	client_list_node* temp;
	temp = (client_list_node *)malloc(sizeof(client_list_node));
	temp->client_fd = new_sock_fd;
	temp->next = NULL;
	
	//Pass session information to the client
	strcpy(buff, "---MENU---\n");
	send(new_sock_fd, buff, strlen(buff), 0);
	/*send all open sessions then send stop marker*/
	memset(buff, '\0', sizeof(buff));
	
	//Recieve request from user
	int check = 0;
	while(check == 0){
		check = recv(new_sock_fd, buff, LENGTH, 0);
	}
	session = atoi(buff) - 1;
	printf("Requested session = %d\n", session+1);
	
	char fname[100];
	memset(fname, '\0', sizeof(fname));
	
	switch(session){
		case 0:{	strcpy(fname, "doc0.pdf");
					break;
		}
		case 1:{	strcpy(fname, "doc1.pdf");
					break;
		}
		case 2:{	strcpy(fname, "doc2.pdf");
					break;
		}
		case 3:{	strcpy(fname, "doc3.pdf");
					break;
		}
		case 4:{	strcpy(fname, "doc4.pdf");
					break;
		}
		case 5:{	strcpy(fname, "doc5.pdf");
					break;
		}
		case 6:{	strcpy(fname, "doc6.pdf");
					break;
		}
		case 7:{	strcpy(fname, "doc7.pdf");
					break;
		}
		case 8:{	strcpy(fname, "doc8.pdf");
					break;
		}
		case 9:{	strcpy(fname, "doc9.pdf");
					break;
		}
		default:{	strcpy(fname, "doc9.pdf");
					break;
		}
	}
	printf("File to be transfered is %s\n", fname);
	
	char sendBuff[1025];
	FILE* fp = fopen(fname, "r");
	if(fp == NULL){
		printf("Error in fopen.\n");
		exit(0);
	}
	memset(sendBuff, '0', sizeof(sendBuff)); 
	int n = 0;
	printf("File transfer start!\n");
	while((n= fread(sendBuff, sizeof(char), LENGTH, fp))>0){
		if(send(new_sock_fd, sendBuff, n, 0)<0){
			printf("Error in sending file.\n");
			exit(0);
		}
		bzero(sendBuff, LENGTH);
	}
	strcpy(sendBuff, "Sent");
	send(new_sock_fd, sendBuff, sizeof(char)*strlen("Sent"), 0);
	fclose(fp);
	printf("File transfer complete!\n");
	
	strcpy(sendBuff, "Sent");
	send(new_sock_fd, sendBuff, sizeof(sendBuff), 0);
	
	memset(buff, '0', sizeof(buff));
	printf("Waiting for confirmation.\n");
	recv(new_sock_fd, buff, LENGTH, 0);
	if(strcmp(buff, "Recieved!!!") == 0)
		printf("%s\nRecieved confirmation\n", buff);
	else{
		printf("Couldn't complete file transfer!!\n");
		exit(0);
	}
	//Insert the incomming connection in the proper list
	pthread_mutex_lock(&list_mutex[session]);
	if(list[session].head == NULL && list[session].tail == NULL){
		list[session].head = list[session].tail = temp;
	}
	else{
		list[session].tail->next = temp;
		list[session].tail = temp;
	}
	pthread_mutex_unlock(&list_mutex[session]);
	
	//Exit from thread
	pthread_exit(NULL);
}

//Function to handle each session
void *handleSession(void* args){
	int session_no = (int)args;
	
	char filename[100];
	memset(filename, '\0', sizeof(filename));
	char scriptname[100];
	memset(scriptname, '\0', sizeof(scriptname));
	
	switch(session_no){
		case 0:{	strcpy(filename, "doc0.pdf");
					strcpy(scriptname, "./get_doc0");
					break;
		}
		case 1:{	strcpy(filename, "doc1.pdf");
					strcpy(scriptname, "./get_doc1");
					break;
		}
		case 2:{	strcpy(filename, "doc2.pdf");
					strcpy(scriptname, "./get_doc2");
					break;
		}
		case 3:{	strcpy(filename, "doc3.pdf");
					strcpy(scriptname, "./get_doc3");
					break;
		}
		case 4:{	strcpy(filename, "doc4.pdf");
					strcpy(scriptname, "./get_doc4");
					break;
		}
		case 5:{	strcpy(filename, "doc5.pdf");
					strcpy(scriptname, "./get_doc5");
					break;
		}
		case 6:{	strcpy(filename, "doc6.pdf");
					strcpy(scriptname, "./get_doc6");
					break;
		}
		case 7:{	strcpy(filename, "doc7.pdf");
					strcpy(scriptname, "./get_doc7");
					break;
		}
		case 8:{	strcpy(filename, "doc8.pdf");
					strcpy(scriptname, "./get_doc8");
					break;
		}
		case 9:{	strcpy(filename, "doc9.pdf");
					strcpy(scriptname, "./get_doc9");
					break;
		}
		default:{	strcpy(filename, "doc9.pdf");
					strcpy(scriptname, "./get_doc9");
					break;
		}
	}
	
	char *arg[3];
	arg[0] = "okular";
	strcpy(arg[1], filename);
	arg[2] = NULL;
	int pid = fork();
	if(pid == 0){
		execvp(arg[0],arg);
	}
	client_list clients;
	client_list_node *temp, *temp1;
	
	char sendBuff[1025];
	memset(sendBuff, '0', sizeof(sendBuff)); 
	int n = 0;
	
	while(1){
		//run the shell script containing xdotool commands
		system(scriptname);
		
		pthread_mutex_lock(&list_mutex[session_no]);
		clients = list[session_no];
		pthread_mutex_unlock(&list_mutex[session_no]);
		
		temp = clients.head;
		
		//For each client of this session
		while(temp!=NULL){
			//Do the needful here
			//If send or recieve fails, i.e. the client is closed, in that case delete that node from linked list
			char buff[LENGTH];
			strcpy(buff,"Message from server");
			if(send(temp->client_fd, buff, strlen(buff) + 1, 0) <= 0){
				pthread_mutex_lock(&list_mutex[session_no]);
				temp1 = list[session_no].head;
				if(temp == list[session_no].head && temp == list[session_no].tail){
					list[session_no].head = list[session_no].tail = NULL;
				}
				else if(temp == temp1){
					list[session_no].head = temp->next;
				}
				else{
					while(temp1->next != temp){
						temp1 = temp1->next;
					}
					temp1->next = temp->next;
					if(temp == list[session_no].tail){
						list[session_no].tail = temp1;
					}
				}
				pthread_mutex_unlock(&list_mutex[session_no]);
				temp = temp->next;
				continue;
			}
			memset(buff, '0', sizeof(buff));
			if(recv(temp->client_fd, buff, LENGTH, 0) <= 0){
				pthread_mutex_lock(&list_mutex[session_no]);
				temp1 = list[session_no].head;
				if(temp == list[session_no].head && temp == list[session_no].tail){
					list[session_no].head = list[session_no].tail = NULL;
				}
				else if(temp == temp1){
					list[session_no].head = temp->next;
				}
				else{
					while(temp1->next != temp){
						temp1 = temp1->next;
					}
					temp1->next = temp->next;
					if(temp == list[session_no].tail){
						list[session_no].tail = temp1;
					}
				}
				pthread_mutex_unlock(&list_mutex[session_no]);
				temp = temp->next;
				continue;
			}
			printf("%s\n", buff);
			temp = temp->next;
		}
		//Repeat the above after each 100 microseconds
		usleep(100);
	}
	pthread_exit(NULL);
}
