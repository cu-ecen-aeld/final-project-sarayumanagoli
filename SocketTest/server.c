/**************************************************************************************
* Server TCP Program
* @file - server.c
* Reference: https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
*
***************************************************************************************/
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 

#define MAX 100
#define PORT 8080 
#define SA struct sockaddr 

bool signal_flag = false;
int msgid, sockfd;

// structure for message queue 
struct mesg_buffer { 
	long mesg_type; 
	char mesg_text[100]; 
} message; 

void sig_handler(int signo)
{
	if(signo == SIGINT)
	{
		printf("\nCaught SIGINT!\n");
		signal_flag = true;
		// After chatting close the socket 
		close(sockfd); 
	}
	else if(signo == SIGTERM)
	{
		printf("\nCaught SIGTERM!\n");
		signal_flag = true;
		// After chatting close the socket 
		close(sockfd); 
	}
	else
	{
		fprintf(stderr, "\nUnexpected signal!\n");
		exit(EXIT_FAILURE);
	}
}


// Function designed for chat between client and server. 
void func(int sockfd) 
{  
	char buff[MAX];
	int n = 1, i; 
	// infinite loop for chat 
	while(signal_flag != true) 
	{ 
		bzero(buff, MAX); 
		// read the message from client and copy it in buffer 
		read(sockfd, buff, sizeof(buff)); 
		if(buff[0] == 0)
		{
			printf("\nNo more data received from the client!\n");
			break;
		}
		message.mesg_type = n; 
		for(i = 0;buff[i] != '\n';i++)
			message.mesg_text[i] = buff[i];
		//message.mesg_text = buff;
		msgsnd(msgid, &message, sizeof(message), 0); 
		// print buffer which contains the client contents 
		printf("From client: %s\n ", buff); 
		n++;
		memset(message.mesg_text, 0x0, (100*sizeof(char)));
		bzero(buff, MAX); 
	} 
} 

// Driver function 
int main() 
{ 
	int connfd, len; 
	struct sockaddr_in servaddr, cli;
	key_t key;  
	struct sigaction sa;

	// Signal implementations
	sa.sa_handler = sig_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1) 
	{
		perror("sigaction: SIGINT");
		exit(EXIT_FAILURE);
        } 
	if (sigaction(SIGTERM, &sa, NULL) == -1) 
	{
		perror("sigaction: SIGTERM");
		exit(EXIT_FAILURE);
        }

	// ftok to generate unique key 
	key = ftok("progfile", 65); 

	// msgget creates a message queue 
	// and returns identifier 
	msgid = msgget(key, 0666 | IPC_CREAT);  

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, (socklen_t*)&len); 
	if (connfd < 0) { 
		printf("server acccept failed...\n"); 
		exit(0); 
	} 
	else
		printf("server acccept the client...\n"); 

	// Function for chatting between client and server 
	func(connfd); 

	// After chatting close the socket 
	close(sockfd); 
} 

