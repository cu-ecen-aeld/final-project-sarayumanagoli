// Course : Advanced Embedded Software Development
// Final Project : SHARED MEMORY IMPLEMENTATION
// Author : Sarayu Managoli (SAMA2321)
// Code Reference : http://www.cse.psu.edu/~deh25/cmpsc473/notes/OSC/Processes/shm-posix-producer-orig.c
//    		    http://www.cse.psu.edu/~deh25/cmpsc473/notes/OSC/Processes/shm-posix-consumer.c
//		    https://www.geeksforgeeks.org/posix-shared-memory-api/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/wait.h> 
#include "syslog.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>	//for lseeks
#include <unistd.h>
#include <stdlib.h>	//for exit status and for malloc
#include <string.h>	//for strlen
#include <stdbool.h>	//for boolean values
#include <sys/stat.h> 	//for stat 


typedef struct {
	uint8_t ID;
	float data;
} number;


char *prod1_semaphore = "producer1_sem_main";
char *prod2_semaphore = "producer2_sem_main";
char *cons_semaphore = "consumer_sem_main";

int data_file;

/* Function to check if a given file exists */
bool file_exists(char *filename)
{
	struct stat buffer;
	return (stat(filename,&buffer) == 0);
}

void consumer()
{	
	char *data = malloc(100 * sizeof(char));
	number cons;
	number *cons_ptr = &cons;

	/* shared memory file descriptor */
	int file_share;

	/* pointer to shared memory obect */
	number *ptr = NULL;


	/* create the shared memory object */
	file_share = shm_open("Trial_Share", O_RDWR, 0666);
	/* memory map the shared memory object */
	ptr = (number *)mmap(0, sizeof(number), PROT_READ, MAP_SHARED, file_share, 0);
	printf("Consumer MMAP\n");

	close(file_share);
	printf("Closed Trial_Share\n");

	memcpy((void*)cons_ptr,(void*)(&ptr[0]),sizeof(number));
	memcpy((void*)cons_ptr,(void*)(&ptr[1]),sizeof(number));
	printf("Consumer MEMCPY\n");

	/* read from the shared memory object */ 
	printf("%d\n", ptr[0].ID);   
	printf("%f\n", ptr[0].data);
	printf("%d\n", ptr[1].ID);
	printf("%f\n", ptr[1].data);

	sprintf(data, "\nSensor ID = %d\tTemperature sensor data= %f", ptr[0].ID, ptr[0].data);
	if(write(data_file, data, strlen(data)) == -1)
	{
		perror("Write 1 to data file failed!");
		exit(EXIT_FAILURE);
	}
	printf("\nData 1 written to file!");

	sprintf(data, "\nSensor ID = %d\tGas sensor data = %f", ptr[1].ID, ptr[1].data);
	if(write(data_file, data, strlen(data)) == -1)
	{
		perror("Write 2 to data file failed!");
		exit(EXIT_FAILURE);
	}
	printf("\nData 2 written to file!");

	printf("Write done!\n");
	//shm_unlink("Trial_Share");
	//printf("SHM unlinked!\n");
	munmap(ptr,sizeof(number));
	printf("Consumer MUNMAP\n");
}

void sharedmem(void)
{
	syslog(LOG_INFO,"In sharedmem function\n");
	int file_share = shm_open("Trial_Share", O_RDONLY, 0666);
	if(file_share < 0)
	{ 
		syslog(LOG_INFO,"SHM OPEN Error\n"); 
	}
	
	if (close(file_share) < 0) 
	{ 
		syslog(LOG_INFO,"FILE CLOSE ERROR\n"); 
	}


	consumer();

}

int main(int argc,char *argv[])
{
	pid_t process_id = 0;
	pid_t sid = 0;

	uint8_t result = 0;

	//Open syslog
	openlog("consumer",LOG_PID|LOG_CONS,LOG_USER);

	char *file_location = "/var/tmp/temperature";

	if(file_exists(file_location) == true)
	{
		remove(file_location);
	}
	
	// Check if the file exists else create a new file
	if(file_exists(file_location) == false)
	{
		data_file = creat(file_location,0755);			// Creating a new file with all permissions granted to the user [4]
		if(data_file < 0)
		{
			perror("Data File creation unsuccessful!");
			exit(EXIT_FAILURE);
		}
	}
	data_file = open(file_location, O_RDWR|O_APPEND);		
	// The file is opened in APPEND mode to allow for consecutive writes without overwriting the previous data
	if(data_file < 0)
	{
		perror("Data File open unsuccessful!");
		exit(EXIT_FAILURE);
	}

	if(argc == 2)
		result = strcmp(argv[1], "-d"); //Checking if the first argument is -d to daemonize a process
	syslog(LOG_INFO,"Result = %d\n",result);

	syslog(LOG_INFO,"Creating child process\n");
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

		// Close stdin. stdout and stderr
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}

	while(1)
	{
		sharedmem();
		usleep(100000);
	}

	close(data_file);
	return 0;
}


