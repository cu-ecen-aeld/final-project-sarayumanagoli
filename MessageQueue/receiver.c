// C Program for Message Queue (Receiver Process) 
// Reference - https://www.geeksforgeeks.org/ipc-using-message-queues/
#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <syslog.h>
#include <string.h>
#include <sys/stat.h> //umask


bool signal_flag = false;
int msgid;

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
		// to destroy the message queue 
		msgctl(msgid, IPC_RMID, NULL); 
	}
	else if(signo == SIGTERM)
	{
		printf("\nCaught SIGTERM!\n");
		signal_flag = true;
		// to destroy the message queue 
		msgctl(msgid, IPC_RMID, NULL); 
	}
	else
	{
		fprintf(stderr, "\nUnexpected signal!\n");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[2]) 
{ 
	key_t key; 
	int n = 1; 
	struct sigaction sa;
	const char *daemon = NULL;
	pid_t pid, sid;

	openlog("Parent_MQ",LOG_NDELAY,LOG_USER);			// Opens a connection to the syslogs

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

			openlog("Daemon_MQ",LOG_NDELAY,LOG_USER);			// Opens a connection to the syslogs [2],[3]

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

	while(signal_flag != true)
	{
		//message.mesg_text = (char *)malloc(100 * sizeof(char));
		// msgrcv to receive message 
		msgrcv(msgid, &message, sizeof(message), n, 0); 
		
		if(argc == 2)
		{
			syslog(LOG_INFO,"%s",message.mesg_text);
		}
		else
		{
			// display the message 
			printf("Data Received is : %s \n",message.mesg_text); 
		}
		n++;
		//free(message.mesg_text);
	}
	return 0; 
} 

