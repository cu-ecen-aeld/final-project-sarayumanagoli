// C Program for Message Queue (Receiver Process) 
// Reference - https://www.geeksforgeeks.org/ipc-using-message-queues/
#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

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

int main() 
{ 
	key_t key; 
	int n = 1; 
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

	while(signal_flag != true)
	{
		//message.mesg_text = (char *)malloc(100 * sizeof(char));
		// msgrcv to receive message 
		msgrcv(msgid, &message, sizeof(message), n, 0); 

		// display the message 
		printf("Data Received is : %s \n", 
						message.mesg_text); 
		n++;
		//free(message.mesg_text);
	}
	return 0; 
} 

