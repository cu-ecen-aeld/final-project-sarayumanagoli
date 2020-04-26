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

#define DEV_ADDRESS (0x48)
#define TEMP_REGISTER (0x00)

#define MAX_BUF 200 
#define SYSFS_ADC_DIR "/sys/bus/iio/devices/iio:device0/in_voltage4_raw" 

typedef struct {
	uint8_t ID;
	float data;
} number;


char *prod1_semaphore = "producer1_sem_main";
char *prod2_semaphore = "producer2_sem_main";
char *cons_semaphore = "consumer_sem_main";

int temp_file, data_file;

void temperature_init(void)
{
	char check_val[1] = {TEMP_REGISTER};

	syslog(LOG_INFO,"\nThis is a test for the TMP102 sensor");
	if((temp_file = open("/dev/i2c-2",O_RDWR)) < 0)
	{
		syslog(LOG_INFO,"\nFailed to open the I2C-2 bus!");
		exit(EXIT_FAILURE);
	}
	syslog(LOG_INFO,"\nSuccessfully opened the I2C-2 bus!");
	if((ioctl(temp_file, I2C_SLAVE, DEV_ADDRESS)) < 0)
	{
		syslog(LOG_ERR,"\nFailed to connect to the sensor!");
		exit(EXIT_FAILURE);
	}
	syslog(LOG_INFO,"\nSuccessfully connected to the sensor!");
	syslog(LOG_INFO,"\nAttempting to reset the read address...");
	if((write(temp_file, check_val, 1)) != 1)
	{
		syslog(LOG_ERR,"\nFailed to reset the read address");
		exit(EXIT_FAILURE);
	} 
	syslog(LOG_INFO,"\nReset successful!");
}

/* Function to check if a given file exists */
bool file_exists(char *filename)
{
	struct stat buffer;
	return (stat(filename,&buffer) == 0);
}


void producer1() 
{   
	syslog(LOG_INFO,"In PRODUCER 1\n");
	char read_val[2] = {0};
	int16_t digitalTemp;
	float tempC;
	
	if((read(temp_file, read_val, 2)) != 2)
	{
		syslog(LOG_ERR,"\nFailed to read the check value from the configuration register");
		exit(EXIT_FAILURE);
	}
	digitalTemp = (((read_val[0]) << 4) | ((read_val[1]) >> 4));
	if(digitalTemp > 0x7FF)
	{
		digitalTemp |= 0xF000;
	}
	tempC = digitalTemp * 0.0625;

	syslog(LOG_INFO,"Temperature = %f\n",tempC);

	number prod1 = {1,tempC};

	number *prod1_ptr = &prod1;

	/* shared memory file descriptor */
	int file_share; 

	/* pointer to shared memory obect */
	number *ptr = NULL; 

	/* create the shared memory object */
	file_share = shm_open("Trial_Share", O_RDWR, 0666);
	syslog(LOG_INFO,"Producer 1 SHM Opened\n");
	/* memory map the shared memory object */
	ptr = (number *)mmap(NULL, sizeof(number), PROT_WRITE, MAP_SHARED, file_share, 0); 
	syslog(LOG_INFO,"Producer 1 MMAP\n");

	close(file_share);
	syslog(LOG_INFO,"Closed Trial_Share\n");

	memcpy((void *)(&ptr[0]),(void*)prod1_ptr,sizeof(number));
	syslog(LOG_INFO,"Producer 1 MEMCPY\n");
	munmap(ptr,sizeof(number));
	syslog(LOG_INFO,"Producer 1 MUNMAP\n");
} 

void producer2()
{

	uint8_t fd; 
	char buffer[MAX_BUF]; 
	char val[4]; 
	uint16_t value_read = 0;
	float float_value = 0.0;

	syslog(LOG_INFO,"Message from PRODUCER 2\n");

	snprintf(buffer, sizeof(buffer), SYSFS_ADC_DIR); 
	syslog(LOG_INFO,"Opening file: %s\n",buffer); 

	fd = open(buffer, O_RDONLY); 
	if (fd < 0) 
	{ 
		syslog(LOG_ERR,"Unable to get ADC Value\n"); 
	} 

	read(fd, &val, 4); 
	close(fd); 
	
	value_read = atoi(val);
	float_value = (float)value_read;
	if(value_read > 4000)
	{
		syslog(LOG_INFO,"Value is %d\tGas detected!\n",value_read);
	}
	else
	{
		syslog(LOG_INFO,"Value is %d\tGas not detected!\n",value_read);
	}

	/* strings written to shared memory */
	number prod2 = {2,float_value};

	number *prod2_ptr = &prod2;

	/* shared memory file descriptor */
	int file_share;

	/* pointer to shared memory obect */
	number *ptr = NULL;

	/* create the shared memory object */
	file_share = shm_open("Trial_Share", O_RDWR, 0666);
	syslog(LOG_INFO,"Producer 2 SHM Opened\n");
	/* memory map the shared memory object */
	ptr = (number *)mmap(NULL, sizeof(number), PROT_WRITE, MAP_SHARED, file_share, 0);
	syslog(LOG_INFO,"Producer 2 MMAP\n");

	close(file_share);
	syslog(LOG_INFO,"Closed Trial_Share\n");

	memcpy((void *)(&ptr[1]),(void*)prod2_ptr,sizeof(number));
	syslog(LOG_INFO,"Producer 2 MEMCPY\n");
	munmap(ptr,sizeof(number));
	syslog(LOG_INFO,"Producer 2 MUNMAP\n");
}

void sharedmem(void)
{
//	sem_t *main_sem;

	syslog(LOG_INFO,"In sharedmem function\n");
	int file_share = shm_open("Trial_Share",O_CREAT | O_RDWR, 0666);
	if(file_share < 0)
	{ 
		syslog(LOG_INFO,"SHM OPEN Error\n"); 
	}

	ftruncate(file_share, 10000);
	
	if (close(file_share) < 0) 
	{ 
		syslog(LOG_INFO,"FILE CLOSE ERROR\n"); 
	}


	producer1();
    	producer2();

}

int main(int argc,char *argv[])
{
	pid_t process_id = 0;
	pid_t sid = 0;
	pid_t cid = 0;
	int status = 0;
	int ret = 0;
	uint8_t result = 0;

	//Open syslog
	openlog("producer",LOG_PID|LOG_CONS,LOG_USER);

	if(argc == 2)
		result = strcmp(argv[1], "-d"); //Checking if the first argument is -d to daemonize a process
	syslog(LOG_INFO,"Result = %d\n",result);

	temperature_init();

	syslog(LOG_INFO,"Creating child process\n");
	if(argc == 2 && result == 0) //Checking if there is only 1 argument and if it is '-d'
	{
		// Create child process
		process_id = fork();
		syslog(LOG_INFO,"Child process is  = %d\n",process_id);
		
		//cid = waitpid(process_id,&status,0);
		cid = wait(&status);	
		syslog(LOG_INFO,"After wait for PID %d\n",cid);

		
		ret = kill(process_id,0);
		syslog(LOG_INFO,"Kill returned %d\n", ret);

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
		syslog(LOG_INFO,"IN CHILD\n");
		

		//set new session
		sid = setsid();
		if(sid < 0)
		{
			syslog(LOG_INFO,"SID is less than 0\n");
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
	}

	return 0;
}

