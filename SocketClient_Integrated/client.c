/******************************************************************************************
* Client TCP Program
* Filename - client.c
* About - This application is responsible for sending the sensor data to the server socket
* Reference: https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
*
*******************************************************************************************/
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h> 	//for stat and umask
#include <signal.h>
#include <stdbool.h>
#include <fcntl.h>
#include <syslog.h>

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 

bool daemon_flag = false;
bool signal_flag = false;
int sockfd;

/* Function to check if a given file exists */
bool file_exists(char *filename)
{
	struct stat buffer;
	return (stat(filename,&buffer) == 0);
}

/* Handler for the signals */
void sig_handler(int signo)
{
	if(signo == SIGINT)
	{
		syslog(LOG_INFO,"Caught SIGINT!");
		printf("\nCaught SIGINT!\n");
		signal_flag = true;
		// After chatting close the socket 
		close(sockfd); 
		system("rm /var/tmp/temperature");
	}
	else if(signo == SIGTERM)
	{
		syslog(LOG_INFO,"Caught SIGTERM!");
		printf("\nCaught SIGTERM!\n");
		signal_flag = true;
		// After chatting close the socket 
		close(sockfd); 
		system("rm /var/tmp/temperature");
	}
	else
	{
		syslog(LOG_INFO,"Unexpected signal!");
		fprintf(stderr, "\nUnexpected signal!\n");
		exit(EXIT_FAILURE);
	}
}

void read_from_file(int sockfd)
{
	char *file_location = "/var/tmp/temperature";
	FILE *data_file;
	char *read_data = NULL;
	int ret_val;
	
	// Check if the file exists
	if(file_exists(file_location) == false)
	{
		perror("File doesn't exist!");
		exit(EXIT_FAILURE);
	}	
	else
	{
		if((data_file = fopen(file_location, "r")) < 0)
		{
			perror("File couldn't be opened");
			exit(EXIT_FAILURE);
		}
		printf("\nFile opened successfully!");
		read_data = (char *)malloc(100 *sizeof(char));
		while(signal_flag != true)
		{
			while((fgets(read_data, 100, data_file)) == NULL)
			;
			printf("\n To Server: %s",read_data);
			ret_val = write(sockfd, read_data, (strlen(read_data))); 	// Sending the read packets to the client socket
			//printf("\nRet_val = %d",ret_val);
			if(ret_val < 0)
			{
				perror("Send failed!");
				exit(EXIT_FAILURE);
			}	
			//usleep(900000);
			//sleep(1);
		}
		printf("\nNo more data to send!\n");
		free(read_data);				
	}
}	



int main(int argc, char *argv[2]) 
{  
	struct sockaddr_in servaddr; 
	struct sigaction sa;
	const char *daemon = NULL;
	pid_t pid, sid;

	openlog("Parent_Client",LOG_NDELAY,LOG_USER);			// Opens a connection to the syslogs

	if(argc == 2)
	{
		daemon = argv[1];
	}

	if(argc == 2)
	{
		if(strcmp(daemon,"-d") == 0)		// Condition to check if the program is to be run as a daemon
		{
			// Fork off the parent process 
			pid = fork();
			if (pid < 0) 
			{
				exit(EXIT_FAILURE);
			}
			// If we got a good PID, then we can exit the parent process. 
			if (pid > 0) 
			{ // Child can continue to run even after the parent has finished executing
				closelog();
				exit(EXIT_SUCCESS);
			}

			// Change the file mode mask
			umask(0);

			openlog("Daemon_Client",LOG_NDELAY,LOG_USER);			// Opens a connection to the syslogs [2],[3]

			// Create a new SID for the child process 
			sid = setsid();
			if (sid < 0) 
			{
				// Log the failure 
				exit(EXIT_FAILURE);
			}
			syslog(LOG_DEBUG,"SID set!");
			// Change the current working directory 
			if ((chdir("/")) < 0) 
			{
				// Log the failure 
				exit(EXIT_FAILURE);
			}
			syslog(LOG_DEBUG,"Directory changed to root.");
			// Close out the standard file descriptors 
			//Because daemons generally dont interact directly with user so there is no need of keeping these open
			close(STDIN_FILENO);
			syslog(LOG_DEBUG,"STDIN closed.");
			close(STDOUT_FILENO);
			syslog(LOG_DEBUG,"STDOUT closed.");
			close(STDERR_FILENO);
			syslog(LOG_DEBUG,"STDERR closed.");
			daemon_flag = true;
		}
	}

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

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) 
	{ 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("10.0.0.70"); 
	servaddr.sin_port = htons(PORT); 

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0)
	{ 
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
	printf("\nClosed the socket!");
} 

