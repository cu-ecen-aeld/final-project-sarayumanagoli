/***********************************************************************************************
 * Aesdsocket Application - The server side application for a socket program communicating on 
 * port 9000.
 *
 * Author - Gitanjali Suresh (gisu9983)
 *
 * The application can be compiled using either of the two commands:
 * Native Platform - gcc -g aesdsocket.c -o aesdsocket
 * Cross-Platform - arm-unknown-linux-gnueabi-gcc -static -g aesdsocket.c -o aesdsocket
 * Here 'aesdsocket' is the executable created
 * 
 ***********************************************************************************************/
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h>
#include <sys/stat.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <syslog.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>

#define PORT 9000 
#define MAX_SIZE 512

/* Function to check if a given file exists */
bool file_exists(char *filename)
{
	struct stat buffer;
	return (stat(filename,&buffer) == 0);
}

int server_fd, file_fd;				//Socket and file descriptors
char *file = "/var/tmp/aesdsocketdata";		//Path of the file to be created

/* Signal handler for SIGINT and SIGTERM */
void sig_handler(int signo)
{
	if(signo == SIGINT)
	{
		printf("\nCaught SIGINT!\n");
		syslog(LOG_INFO,"Caught signal SIGINT, exiting");
	}
	else if(signo == SIGTERM)
	{
		printf("\nCaught SIGTERM!\n");
		syslog(LOG_INFO,"Caught signal SIGTERM, exiting");
	}
	else
	{
		fprintf(stderr, "\nUnexpected signal!\n");
		exit(EXIT_FAILURE);
	}
	closelog();
	close(server_fd);
	close(file_fd);
	remove(file);
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[2]) 
{ 
	int new_socket, valread = 0, fileread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	ssize_t ret_val;
	char buffer[MAX_SIZE] = {0};					//Buffer to store the byte stream received from client (sockettest.sh)				
	char *file_buffer = NULL;
	char *client_ip;
	struct sigaction sa;
	pid_t pid, sid;
	const char *daemon;
	if(argc == 2)
	{
		daemon = argv[1];
	}

	openlog("--SOCKETS--",LOG_NDELAY,LOG_USER);			// Opens a connection to the syslogs	
	
	// Creating socket file descriptor 
	syslog(LOG_DEBUG, "Creating socket file descriptor...");
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	syslog(LOG_DEBUG, "Socket file descriptor created successfully!");
	
	// Forcefully attaching socket to the port 9000 
	syslog(LOG_DEBUG, "Attaching socket to the port 9000...");
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	syslog(LOG_DEBUG, "Socket attached to port 9000.");
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons(PORT); 

	// Binding socket to the port 9000 
	syslog(LOG_DEBUG, "Binding socket...");
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		close(server_fd);
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	syslog(LOG_DEBUG, "Bind successfull!");

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

			openlog("SOCKETS_CHILD",LOG_NDELAY,LOG_USER);			// Opens a connection to the syslogs [2],[3]

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
		}
	}
	syslog(LOG_DEBUG, "Listening...");
	if (listen(server_fd, 3) < 0) 
	{ 
		close(server_fd);
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	
	if(file_exists(file) == true)
	{
		remove(file);
	}
	
	// Check if the file exists else create a new file
	if(file_exists(file) == false)
	{
		file_fd = creat(file,0755);			// Creating a new file with all permissions granted to the user [4]
	}
	file_fd = open(file, O_RDWR|O_APPEND);		// The file is opened in APPEND mode to allow for consecutive writes without overwriting the previous data
	
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


	while(1)
	{	
		// Accepting the socket connection
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
		{ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 

		client_ip = inet_ntoa(address.sin_addr);		// To print the IP address in a readable form
		syslog(LOG_INFO, "Accepted connection from %s", client_ip);
		printf("\nAccepted connection from %s", client_ip);
		
		valread = recv(new_socket, buffer, MAX_SIZE, 0);	// Receiving the socket stream from the client
		if(valread < 0)
		{
			perror("Receive Failed!");
			exit(EXIT_FAILURE);
		}
		printf("\nReceived packet: %s",buffer);

		ret_val = write(file_fd, buffer, valread);		//Writing the received socket packet into a file
		if(ret_val == -1)
		{
			perror("Write to file failed!");
			exit(EXIT_FAILURE);
		}

		file_buffer = (char *)malloc((ret_val+1)*sizeof(char));	// Creation of file buffer for sending characters
		if(file_buffer == NULL)
		{
			perror("Memory allocation for 'file_buffer' unsuccessful!");
			exit(EXIT_FAILURE);
		}

		lseek(file_fd,0,SEEK_SET);
		while((fileread = read(file_fd, file_buffer, sizeof(file_buffer))) != 0)	// Reading the contents of the file until new line is reached
		{
			file_buffer[fileread] = '\0';
			ret_val = send(new_socket, file_buffer, strlen(file_buffer), 0); 	// Sending the read packets to the client socket
			if(ret_val < 0)
			{
				perror("Send failed!");
				exit(EXIT_FAILURE);
			}
		}

		free(file_buffer);
		syslog(LOG_INFO, "Closed connection from %s", client_ip);
	}
	return 0; 
} 
/*************************************** References *******************************************
 * [1] https://beej.us/guide/bgnet/html/
 * [2] https://www.thegeekstuff.com/2011/12/c-socket-programming/
 * [3] https://blog.abhi.host/blog/2010/03/09/writing-daemon-in-c-or-daemonize/
 * [4] https://codeforwin.org/2018/03/c-program-check-file-or-directory-exists-not.html
 **********************************************************************************************/
