/**************************************************************************************
* Client TCP Program
* @file - client.c
* Reference: https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
*
***************************************************************************************/
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h> 	//for stat 
#include <stdbool.h>
#include <fcntl.h>

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 
/*void func(int sockfd) 
{ 
	char buff[MAX]; 
	int n; 
	for (;;) { 
		bzero(buff, sizeof(buff)); 
		printf("Enter the string : "); 
		n = 0; 
		while ((buff[n++] = getchar()) != '\n') 
			; 
		write(sockfd, buff, sizeof(buff)); 
		bzero(buff, sizeof(buff)); 
		read(sockfd, buff, sizeof(buff)); 
		printf("From Server : %s", buff); 
		if ((strncmp(buff, "exit", 4)) == 0) { 
			printf("Client Exit...\n"); 
			break; 
		} 
	} 
} */

/* Function to check if a given file exists */
bool file_exists(char *filename)
{
	struct stat buffer;
	return (stat(filename,&buffer) == 0);
}

void read_from_file(int sockfd)
{
	char *file_location = "/var/tmp/temperature";
	int data_file;
	char *read_data = NULL;
	int fileread, ret_val;
	
	// Check if the file exists
	if(file_exists(file_location) == false)
	{
		perror("File doesn't exist!");
		exit(EXIT_FAILURE);
	}	
	else
	{
		if((data_file = open(file_location, O_RDWR|O_APPEND)) < 0)
		{
			perror("File couldn't be opened");
			exit(EXIT_FAILURE);
		}
		printf("\nFile opened successfully!");
		read_data = (char *)malloc(500 *sizeof(char));
		lseek(data_file,0,SEEK_SET);
		//len = 0;
		while((fileread = read(data_file, read_data, 500)) != 0)	// Reading the contents of the file until new line is reached
		{
			read_data[fileread] = '\0';
			printf("\nFILE -- %s",read_data);
			ret_val = send(sockfd, read_data, (strlen(read_data)), 0); 	// Sending the read packets to the client socket
			if(ret_val < 0)
			{
				perror("Send failed!");
				exit(EXIT_FAILURE);
			}
			//len += fileread;
		}	
	}
}	



int main() 
{ 
	int sockfd; 
	struct sockaddr_in servaddr; 

	// socket create and varification 
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
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	servaddr.sin_port = htons(PORT); 

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else
		printf("connected to the server..\n"); 

	// function for chat 
	//func(sockfd); 

	read_from_file(sockfd);

	// close the socket 
	close(sockfd); 
} 

