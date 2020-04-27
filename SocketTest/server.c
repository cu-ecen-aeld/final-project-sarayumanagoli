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
#include <syslog.h>
#include <sys/stat.h> //umask

#define MAX 100
#define PORT 8080 
#define SA struct sockaddr 

bool signal_flag = false;
bool daemon_flag = false;
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
		syslog(LOG_INFO,"Caught SIGINT!");
		printf("\nCaught SIGINT!\n");
		signal_flag = true;
		// After chatting close the socket 
		close(sockfd); 
	}
	else if(signo == SIGTERM)
	{
		syslog(LOG_INFO,"Caught SIGTERM!");
		printf("\nCaught SIGTERM!\n");
		signal_flag = true;
		// After chatting close the socket 
		close(sockfd); 
	}
	else
	{
		syslog(LOG_INFO,"Unexpected signal!");
		fprintf(stderr, "\nUnexpected signal!\n");
		exit(EXIT_FAILURE);
	}
}


// Function designed for chat between client and server. 
void func(int sockfd) 
{  
	//char buff[MAX];
	int n = 1; 
	// infinite loop for chat 
	while(signal_flag != true) 
	{ 
		read(sockfd, message.mesg_text, sizeof(message.mesg_text)); 
		if(message.mesg_text[0] == 0)
		{
			if(daemon_flag == true)
				syslog(LOG_INFO,"No more data received from the client!");
			else
				printf("\nNo more data received from the client!\n");
			break;
		}
		message.mesg_type = n; 
		//for(i = 0;message.mesg_text[i] != '\n';i++);
		//message.mesg_text[i] = '\0';
		msgsnd(msgid, &message, sizeof(message), 0); 
		// print buffer which contains the client contents 
		if(daemon_flag == true)
			syslog(LOG_INFO,"From client: %s", message.mesg_text);
		else
			printf("From client: %s\n ", message.mesg_text); 
		n++;
		memset(message.mesg_text, 0x0, (100*sizeof(char)));
	} 
} 

// Driver function 
int main(int argc, char *argv[2]) 
{ 
	int connfd, len; 
	struct sockaddr_in servaddr, cli;
	key_t key;  
	struct sigaction sa;
	const char *daemon = NULL;
	pid_t pid, sid;

	openlog("Parent_Server",LOG_NDELAY,LOG_USER);			// Opens a connection to the syslogs

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

			openlog("Daemon_Server",LOG_NDELAY,LOG_USER);			// Opens a connection to the syslogs [2],[3]

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

	// ftok to generate unique key 
	key = ftok("progfile", 65); 

	// msgget creates a message queue 
	// and returns identifier 
	msgid = msgget(key, 0666 | IPC_CREAT);  

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) 
	{ 
		syslog(LOG_DEBUG,"Socket creation failed!");
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
	{
		syslog(LOG_DEBUG,"Socket successfully created!");
		printf("Socket successfully created..\n"); 
	}
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) 
	{ 
		syslog(LOG_DEBUG,"socket bind failed!");
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
	{
		syslog(LOG_DEBUG,"Socket successfully binded");
		printf("Socket successfully binded..\n"); 
	}

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) 
	{ 
		syslog(LOG_DEBUG,"Listen failed!");
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
	{
		syslog(LOG_DEBUG,"Server listening..");
		printf("Server listening..\n"); 
	}
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, (socklen_t*)&len); 
	if (connfd < 0) 
	{ 
		syslog(LOG_DEBUG,"Server accept failed!");
		printf("server acccept failed...\n"); 
		exit(0); 
	} 
	else
	{
		syslog(LOG_DEBUG,"Server accept the client...");
		printf("server acccept the client...\n"); 
	}

	// Function for chatting between client and server 
	func(connfd); 

	// After chatting close the socket 
	close(sockfd); 
} 

