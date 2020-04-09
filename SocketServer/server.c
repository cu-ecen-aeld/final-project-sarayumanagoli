// Course : Advanced Embedded Software Development
// Assignment Five : Socket Server
// Author : Sarayu Managoli (SAMA2321)
// Code Reference : https://www.thegeekstuff.com/2012/02/c-daemon-process/
//    https://www.geeksforgeeks.org/socket-programming-cc/
//    https://beej.us/guide/bgnet/html/#a-simple-stream-server
//    https://stackoverflow.com/questions/31763672/recv-on-socket-by-dynamically-allocating-space
//    https://riptutorial.com/posix/example/16306/posix-timer-with-sigev-thread-notification
//    Lecture slides

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "syslog.h"
#include "unistd.h"
#include "fcntl.h"
#include "signal.h"
#include "string.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/queue.h>
#include <time.h>

#define MAXSIZE 1

int fp, newsockfd, sockfd;
char *client_IP = NULL;
bool flag = false;
int server_len, port, nw, nr;
char *buffer_new;


pthread_mutex_t listlock;

struct node_data
{
	pthread_t thread_ID;
	int thread_complete_flag;
};

// linked list node
struct node
{
	struct node_data thread_data;
	SLIST_ENTRY(node) node_struct;
};

// create data type for head of queue for node_struct of type 'struct node'
SLIST_HEAD(head_s, node) head = SLIST_HEAD_INITIALIZER(head);


//Function to handle events related to threads
void* thread_handler(void* thread_arg)
{
	unsigned long bytes_received = 0;
	unsigned long cur_size = 0;
	int status = 0;
	char *buffer = NULL;
	do
	{	//realloc logic
		if (bytes_received >= cur_size)
		{
			char * tmp;
			cur_size += MAXSIZE; //increase the size
			tmp = realloc(buffer, cur_size);
			if (NULL == tmp)
			{
				syslog(LOG_ERR,"Buffer uninitialised");
				break;
			}

			buffer = tmp;
		}
		status = recv(newsockfd, buffer + bytes_received, MAXSIZE, 0);
		if (status == 0)
		{
			printf("Bye\n");
		}
		else if (status > 0)
		{
			bytes_received += status;
			//printf("%d\n", status);          
		}
		syslog(LOG_INFO,"Receiving");

	}while( status > 0 && buffer[bytes_received-1]!='\n');

	//Write the received characters to a file
	pthread_mutex_lock(&listlock);
	nw=write(fp,buffer,bytes_received);
	pthread_mutex_unlock(&listlock);
	if (nw == -1 && flag != true)
	{
		syslog(LOG_ERR,"ERROR writing to file");
		exit(1);
	}

	//Dynamically allocate the send buffer based on the receive buffer size
	buffer_new = (char *)malloc((nw+1)*sizeof(char));
	if(buffer_new == NULL)
	{
		syslog(LOG_ERR,"ERROR during malloc");
		exit(1);
	}

	//To set the offset to the start of the file
	lseek(fp,0,SEEK_SET);

	//Read till the end of the file
	while((nr = read(fp,buffer_new,sizeof(buffer_new)))!=0)
	{
		//Terminate the buffer
		buffer_new[nr] = '\0';

		//Send the file contents to the client
		if (send(newsockfd, buffer_new, strlen(buffer_new),0) == -1 && flag != true)
		{
			syslog(LOG_ERR,"ERROR while writing");
			exit(1);
		}
	}
	syslog(LOG_INFO,"Closed connection from %s",client_IP);

	pthread_t thread_self;
	thread_self = pthread_self();
	struct node * list_node = NULL;
	pthread_mutex_lock(&listlock);
	SLIST_FOREACH(list_node, &head, node_struct) //Traverse through all nodes
	{
		if(list_node->thread_data.thread_ID == thread_self)
		{
			list_node->thread_data.thread_complete_flag = 1;
		}
	}
	pthread_mutex_unlock(&listlock);
	//Free the send buffer
	free(buffer);
	free(buffer_new);
	return 0;
}

void timer_handler(union sigval sv)
{
	uint8_t file_p;
	//Calculation of time to store in the file
	time_t rawtime;
	struct tm *timeinfo;

	char timer[100];
	rawtime = time(NULL);
	timeinfo=localtime(&rawtime); //Calculate time

	strftime(timer,sizeof(timer),"timestamp:%a, %d %b %Y %T %z\n",timeinfo); //In required format of time
	if((file_p = open("/var/tmp/aesdsocketdata",O_RDWR|O_APPEND,0755))<0)
	{
		syslog(LOG_ERR,"Unable to open file");
		exit(1);
	}
	pthread_mutex_lock(&listlock);
	if(write(file_p,timer,strlen(timer))<0)
	{
		syslog(LOG_ERR,"Unable to write to file");
		exit(1);
	}
	pthread_mutex_unlock(&listlock);
	fsync(file_p);
	close(file_p);
}


//Handler for signals
void signal_handler(int signo)
{
	if(signo == SIGINT || signo == SIGTERM) //Caused by CTRL+C
	{
		flag = true;
		syslog(LOG_INFO,"Caught signal, exiting");
		if(shutdown(sockfd,SHUT_RDWR)) //To gracefully shutdown after accept is interrupted
		{
			syslog(LOG_ERR,"Could not close socket");
		}
	}
	else
	{
		syslog(LOG_ERR,"Unexpected signal!");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	uint8_t result = 0;
	pid_t sid = 0;
	struct sockaddr_in server_addr;
	struct sigaction sig_act;
	pid_t process_id = 0;
	timer_t timerid;

	
	if (pthread_mutex_init(&listlock, NULL) != 0) 
	{ 
		printf("\nMutex initialization failed\n"); 
		exit(EXIT_FAILURE); 
	}

	// initialize head before use
	SLIST_INIT(&head);
	struct node * list_node = NULL;

	//timer_t timerid;
	struct sigevent sev;
	struct itimerspec trigger;

	/* Set all `sev` and `trigger` memory to 0 */
	memset(&sev, 0, sizeof(struct sigevent));
	memset(&trigger, 0, sizeof(struct itimerspec));

	//Upon timer expiration, `sigev_notify_function` (thread_handler()),
	//will be invoked as if it were the start function of a new thread_ID.

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = &timer_handler;
	sigemptyset(sev.sigev_value.sival_ptr);

	//CLOCK_REALTIME is used
	timer_create(CLOCK_REALTIME, &sev, &timerid);

	trigger.it_value.tv_sec = 10;
	trigger.it_interval.tv_sec = 10;

	timer_settime(timerid, 0, &trigger, NULL);

	port = 9000; //Setting HOST port
	if(argc == 2)
		result = strcmp(argv[1], "-d"); //Checking if the first argument is -d to daemonize a process

	//Open syslog
	openlog("aesdsocket",LOG_PID|LOG_CONS,LOG_USER);

	//File creation
	fp=creat("/var/tmp/aesdsocketdata",0755);

	//Open file with appropriate permissions
	fp=open("/var/tmp/aesdsocketdata",O_RDWR|O_APPEND);
	syslog(LOG_INFO,"Opening File");

	//Creating IPv4 protocal with sequenced data
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		syslog(LOG_ERR,"ERROR opening socket");
		exit(EXIT_FAILURE);
	}

	//bzero(buffer,MAXSIZE); //Initializing buffer with the value 0
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	//Binding IPv4
	if (bind(sockfd, (struct sockaddr *) &server_addr,sizeof(server_addr)) < 0)
	{
		syslog(LOG_ERR,"ERROR during binding");
		exit(EXIT_FAILURE);
	}

	if(argc == 2 && result == 0) //Checking if there is only 1 argument and if it is '-d'
	{
		// Create child process
		process_id = fork();
		// Indication of fork() failure
		if (process_id < 0)
		{
			// Return failure in exit status
			exit(1);
		}
		// PARENT PROCESS. Need to kill it.
		if (process_id > 0)
		{
			exit(0);
		}
		//unmask the file mode
		umask(0);

		//Open syslog

		openlog("child",LOG_PID|LOG_CONS,LOG_USER);
		syslog(LOG_INFO,"IN CHILD");


		//set new session
		sid = setsid();
		if(sid < 0)
		{
			// Return failure
			exit(1);
		}
		// Change the current working directory to root.
		chdir("/");

		/* Set all `sev` and `trigger` memory to 0 */
		memset(&sev, 0, sizeof(struct sigevent));
		memset(&trigger, 0, sizeof(struct itimerspec));

		sev.sigev_notify = SIGEV_THREAD;
		sev.sigev_notify_function = &timer_handler;

		timer_create(CLOCK_REALTIME, &sev, &timerid);

		trigger.it_value.tv_sec = 10;
		trigger.it_interval.tv_sec = 10;

		timer_settime(timerid, 0, &trigger, NULL);

		// Close stdin. stdout and stderr
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}

	//Listen and accept
	listen(sockfd,3);
	if (sockfd < 0)
	{
		syslog(LOG_ERR,"ERROR on listening");
		exit(EXIT_FAILURE);
	}
	syslog(LOG_INFO,"Listening");
	server_len = sizeof(server_addr);

	//Initializing signal handler
	sig_act.sa_handler = signal_handler;
	sig_act.sa_flags = SA_RESTART;
	sigemptyset(&sig_act.sa_mask);


	//Checking for each signal
	if(sigaction(SIGINT, &sig_act, NULL) == -1)
	{
		syslog(LOG_ERR,"SIGINT Error");
		exit(EXIT_FAILURE);
	}
	if(sigaction(SIGTERM, &sig_act, NULL) == -1)
	{
		syslog(LOG_ERR,"SIGTERM Error");
		exit(EXIT_FAILURE);
	}

	while(flag != true)
	{
		//To display the IP address of client
		client_IP = inet_ntoa(server_addr.sin_addr);

		//Accept a new connection
		newsockfd = accept(sockfd, (struct sockaddr *) &server_addr, (socklen_t *)&server_len);
		syslog(LOG_INFO,"Accepting");

		if (newsockfd < 0 && flag != true)
		{
			syslog(LOG_ERR,"ERROR on accepting connection");
			exit(EXIT_FAILURE);
		}
		else
			syslog(LOG_INFO,"Accepted connection from %s",client_IP);

		if(list_node != NULL)
		{
			if(list_node->thread_data.thread_complete_flag == 1)
			{
				pthread_join(list_node->thread_data.thread_ID, NULL);
				syslog(LOG_DEBUG, "thread_ID joined tid = %ld", list_node->thread_data.thread_ID);
				SLIST_REMOVE(&head, list_node, node, node_struct);
				free(list_node);
			}				
		}

		// create thread_ID to handle client connection
		list_node = (struct node*)malloc(sizeof(struct node));
		if(list_node == NULL)
		{
			exit(EXIT_FAILURE);
		}
		list_node->thread_data.thread_complete_flag = 0; //reset complete flag before creation

		SLIST_INSERT_HEAD(&head, list_node, node_struct);
		
		//Creation of threads
		if((flag!=true) && pthread_create(&list_node->thread_data.thread_ID, NULL, thread_handler,(void *)&newsockfd) < 0)
		{
			syslog(LOG_ERR,"Unable to create thread_ID");
			exit(EXIT_FAILURE);
		}

		pthread_mutex_lock(&listlock);

		pthread_mutex_unlock(&listlock);
		if(flag == true)
		{
			if(list_node != NULL)
			{
				free(list_node);
			}
		}
			
	}
	timer_delete(timerid); //destroy timer
	if(newsockfd!=-1)
		close(newsockfd); //Graceful exit of the process
	close(sockfd);
	close(fp); //Closing of files
	pthread_mutex_destroy(&listlock); //destrol mutex
	system("rm /var/tmp/aesdsocketdata"); //Delete the created file
	exit(0);
}

