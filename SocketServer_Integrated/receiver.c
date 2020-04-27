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
#include <stdlib.h>
#include "gpio.h"

#define LED   (47)

bool signal_flag = false;
int msgid;

// structure for message queue 
struct mesg_buffer { 
	long mesg_type; 
	char mesg_text[100]; 
} message; 


void sig_handler(int signo)
{
	int ret;
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
	if((ret = gpio_unexport(LED)) != 0)
	{
		perror("gpio_unexport");
		exit(EXIT_FAILURE);
	}
}

void parse_Data(char string[])
{
	int sensor_ID, i, equal_sign_count = 1, ret;
	float sensor_Value;
	char sensor_Value_string[10];
	static float current_Temperature;
	float temperature_Threshold = 30.00, gas_Threshold = 1500.00;
	for(i = 0;string[i] != '\0';i++)
	{
		if(string[i] == '=' && signal_flag != true)
		{
			if(equal_sign_count == 1 && signal_flag != true)
			{
				sensor_ID = atoi(&string[i+2]);
				printf("\nSensor ID = %d",sensor_ID);
				equal_sign_count++;
			}
			else if(equal_sign_count == 2 && signal_flag != true)
			{
				strncpy(sensor_Value_string, &string[i+2], 10);
				sensor_Value = strtof(sensor_Value_string, NULL);
				if(sensor_ID == 1 && signal_flag != true)
				{
					if(sensor_Value >= temperature_Threshold)
					{
						printf("\n\r*****************ALERT:High Temperature****************");
						if((ret = gpio_set_value(LED,1)) != 0)
						{
						    perror("Set ON value error!");
						    exit(EXIT_FAILURE);
						}
					}
					else
					{
						if((ret = gpio_set_value(LED,0)) != 0)
						{
						    perror("Set OFF value error!");
						    exit(EXIT_FAILURE);
						}
					}			
					printf("\nTemperature Value = %f\n\r",sensor_Value);
					current_Temperature = sensor_Value;
				}
				if(sensor_ID == 2 && signal_flag != true)
				{
					if(sensor_Value >= gas_Threshold)
					{
						printf("\n\r***************** ALERT:Smoke detected with Temperature = %f ****************",current_Temperature);
						if((ret = gpio_set_value(LED,1)) != 0)
						{
						    perror("Set ON value error!");
						    exit(EXIT_FAILURE);
						}
					}
					else
					{
						if((ret = gpio_set_value(LED,0)) != 0)
						{
						    perror("Set OFF value error!");
						    exit(EXIT_FAILURE);
						}
					}
					printf("\nGas Value = %f\n\r",sensor_Value);
				}
				equal_sign_count = 1;
				return;
			}
		}
	}
	return;
}

int main(int argc, char *argv[2]) 
{ 
	key_t key; 
	int n = 1, ret = 0; 
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

	if((ret = gpio_export(LED)) != 0)
	{
		perror("Export error");
		exit(EXIT_FAILURE);
	}

	if((ret = gpio_set_dir(LED, GPIO_DIR_OUTPUT)) != 0)
	{
		perror("Direction set error");
		exit(EXIT_FAILURE);
	}

	// ftok to generate unique key 
	key = ftok("progfile", 65); 

	// msgget creates a message queue 
	// and returns identifier 
	msgid = msgget(key, 0666 | IPC_CREAT); 

	while(signal_flag != true)
	{
		// msgrcv to receive message 
		msgrcv(msgid, &message, sizeof(message), n, 0); 
		
		if(argc == 2 && signal_flag != true)
		{
			syslog(LOG_INFO,"%s",message.mesg_text); 
		}
		else if(signal_flag != true)
		{
			// display the message 
			printf("Data Received is : %s \n",message.mesg_text); 
			parse_Data(message.mesg_text);
		}
		n++;
	}
	return 0; 
} 

