// C Program for Message Queue (Receiver Process) 
// Reference - https://www.geeksforgeeks.org/ipc-using-message-queues/
#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <stdlib.h>

// structure for message queue 
struct mesg_buffer { 
	long mesg_type; 
	char *mesg_text; 
} message; 

int main() 
{ 
	key_t key; 
	int msgid, n = 1; 

	// ftok to generate unique key 
	key = ftok("progfile", 65); 

	// msgget creates a message queue 
	// and returns identifier 
	msgid = msgget(key, 0666 | IPC_CREAT); 

	while(1)
	{
		message.mesg_text = (char *)malloc(100 * sizeof(char));
		// msgrcv to receive message 
		msgrcv(msgid, &message, sizeof(message), n, 0); 

		// display the message 
		printf("Data Received is : %s \n", 
						message.mesg_text); 
		n++;
		free(message.mesg_text);
	}

	// to destroy the message queue 
	msgctl(msgid, IPC_RMID, NULL); 

	return 0; 
} 

