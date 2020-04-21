// C Program for Message Queue (Sender Process) 
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
	int msgid, i = 0, n = 1; 

	// ftok to generate unique key 
	key = ftok("progfile", 65); 

	// msgget creates a message queue 
	// and returns identifier 
	msgid = msgget(key, 0666 | IPC_CREAT); 
	while(1)
	{
		message.mesg_type = n; 
		message.mesg_text = (char *)malloc(100 * sizeof(char));
		printf("Write Data : "); 
		while((message.mesg_text[i++] = getchar()) != '\n');

		// msgsnd to send message 
		msgsnd(msgid, &message, sizeof(message), 0); 

		// display the message 
		printf("Data send is : %s \n", message.mesg_text); 
		n++;
		i = 0;
		free(message.mesg_text);
	}

	return 0; 
} 

