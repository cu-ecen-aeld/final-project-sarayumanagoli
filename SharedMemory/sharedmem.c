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

int producer1() 
{   
	char check_val[1] = {TEMP_REGISTER};
	char read_val[2] = {0};
	int16_t digitalTemp;
	float tempC;
	int temp_file;
	
	printf("\nThis is a test for the TMP102 sensor");
	if((temp_file = open("/dev/i2c-2",O_RDWR)) < 0)
	{
		perror("\nFailed to open the I2C-2 bus!");
		exit(EXIT_FAILURE);
	}
	printf("\nSuccessfully opened the I2C-2 bus!");
	if((ioctl(temp_file, I2C_SLAVE, DEV_ADDRESS)) < 0)
	{
		perror("\nFailed to connect to the sensor!");
		exit(EXIT_FAILURE);
	}
	printf("\nSuccessfully connected to the sensor!");
	printf("\nAttempting to reset the read address...");
	if((write(temp_file, check_val, 1)) != 1)
	{
		perror("\nFailed to reset the read address");
		exit(EXIT_FAILURE);
	} 
	printf("\nReset successful!");
	if((read(temp_file, read_val, 2)) != 2)
	{
		perror("\nFailed to read the check value from the configuration register");
		exit(EXIT_FAILURE);
	}
	digitalTemp = (((read_val[0]) << 4) | ((read_val[1]) >> 4));
	if(digitalTemp > 0x7FF)
	{
		digitalTemp |= 0xF000;
	}
	tempC = digitalTemp * 0.0625;

	close(temp_file);

	syslog(LOG_INFO,"Message from PRODUCER 1");
	sem_t* producer1_sem;
	number prod1 = {1,tempC};

	number *prod1_ptr = &prod1;

	/* shared memory file descriptor */
	int file_share; 

	/* pointer to shared memory obect */
	number *ptr = NULL; 

	/* create the shared memory object */
	file_share = shm_open("Trial_Share", O_RDWR, 0666);

	/* memory map the shared memory object */
	ptr = (number *)mmap(NULL, sizeof(number), PROT_WRITE, MAP_SHARED, file_share, 0); 

	producer1_sem = sem_open(prod1_semaphore,0,0666,0);
	printf("Before wait producer 1\n");
	//sem_wait(producer1_sem);
	printf("After wait producer 1\n");
	memcpy((void *)(&ptr[0]),(void*)prod1_ptr,sizeof(number));
	//sem_post(producer1_sem);
	sem_close(producer1_sem);
	close(file_share);

	return 0; 
} 

int producer2()
{
	uint8_t fd; 
	char buffer[MAX_BUF]; 
	char val[4]; 
	uint16_t value_read = 0;
	float float_value = 0.0;
	snprintf(buffer, sizeof(buffer), SYSFS_ADC_DIR); 
	printf("Opening file: %s\n",buffer); 

	fd = open(buffer, O_RDONLY); 
	if (fd < 0) 
	{ 
		perror("Unable to get ADC Value\n"); 
	} 

	read(fd, &val, 4); 
	close(fd); 
	
	value_read = atoi(val);
	float_value = (float)value_read;
	if(value_read > 4000)
	{
		printf("Value is %d\tGas detected!\n",value_read);
	}
	else
	{
		printf("Value is %d\tGas not detected!\n",value_read);
	}

	syslog(LOG_INFO,"Message from PRODUCER 2");
	sem_t* producer2_sem;
	/* strings written to shared memory */

	number prod2 = {2,float_value};

	number *prod2_ptr = &prod2;

	/* shared memory file descriptor */
	int file_share;

	/* pointer to shared memory obect */
	number *ptr = NULL;

	/* create the shared memory object */
	file_share = shm_open("Trial_Share", O_RDWR, 0666);

	/* memory map the shared memory object */
	ptr = (number *)mmap(NULL, sizeof(number), PROT_WRITE, MAP_SHARED, file_share, 0);

	producer2_sem = sem_open(prod2_semaphore,0,0666,0);
	printf("Before wait producer 2\n");
	//sem_wait(producer2_sem);
	printf("After wait producer 2\n");
	memcpy((void *)(&ptr[1]),(void*)prod2_ptr,sizeof(number));
	//sem_post(producer2_sem);	
	sem_close(producer2_sem);
	close(file_share);
	return 0;
}

int consumer()
{

	char *data = malloc(30 * sizeof(char));
	syslog(LOG_INFO,"Message from CONSUMER");
	sem_t* consumer_sem;
	int fp = 0;
	fp=creat("/var/tmp/temperature",0755);
	if(fp < 0)
	{
		perror("Data File creation unsuccessful!");
		exit(EXIT_FAILURE);
	}
	number cons;
	number *cons_ptr = &cons;
	/* shared memory file descriptor */
	int file_share;

	/* pointer to shared memory obect */
	number *ptr = NULL;

	//Open file with appropriate permissions
        fp=open("/var/tmp/temperature",O_RDWR|O_APPEND);
	if(fp < 0)
	{
		perror("Data File open unsuccessful!");
		exit(EXIT_FAILURE);
	}
	
	/* create the shared memory object */
	file_share = shm_open("Trial_Share", O_RDWR, 0666);
	/* memory map the shared memory object */
	ptr = (number *)mmap(0, sizeof(number), PROT_READ, MAP_SHARED, file_share, 0);
	
	consumer_sem = sem_open(cons_semaphore,0,0666,0);
	sem_post(consumer_sem);
	printf("Before wait consumer\n");
	sem_wait(consumer_sem);
	printf("After wait consumer\n");
	memcpy((void*)cons_ptr,(void*)(&ptr[0]),sizeof(number));
	//memcpy((void*)cons_ptr,(void*)(&ptr[1]),sizeof(number));
	sem_post(consumer_sem);
	/* read from the shared memory object */ 
	sprintf(data,"\nID is %d and data acquired is %f",ptr[0].ID,ptr[0].data);
	if(write(fp, data, strlen(data)) == -1)
			{
				perror("Write of ptr0 to data file failed!");
				exit(EXIT_FAILURE);
			}
	sprintf(data,"\nID is %d and data acquired is %f",ptr[1].ID,ptr[1].data);
	if(write(fp, data, strlen(data)) == -1)
			{
				perror("Write of ptr1 to data file failed!");
				exit(EXIT_FAILURE);
			}
	sem_close(consumer_sem);
	/* remove the shared memory object */
	shm_unlink("Trial_Share");
	munmap(ptr,sizeof(number));
	printf("Exiting consumer\n");
	close(fp);
	return 0;
}

int main(void)
{
	sem_t *main_sem;
//	pid_t process_id = 0;
//	pid_t sid = 0;
//	pid_t cid = 0;
//	int status = 0;

	openlog("SharedMemory",LOG_PID|LOG_CONS,LOG_USER);
	syslog(LOG_INFO,"Message from MAIN");

	//int ret;
	main_sem = sem_open(prod1_semaphore, O_CREAT, 0600, 0);
	sem_close(main_sem);	
	main_sem = sem_open(prod2_semaphore, O_CREAT, 0600, 0);
	sem_close(main_sem);	
	main_sem = sem_open(cons_semaphore, O_CREAT, 0600, 0);
	sem_close(main_sem);
	int file_share = shm_open("Trial_Share",O_CREAT | O_RDWR, 0666);
	if(file_share < 0)
	{ 
		printf("SHM OPEN Error\n"); 
	}

	ftruncate(file_share, 4096);
	
	if (close(file_share) < 0) 
	{ 
		printf("FILE CLOSE ERROR\n"); 
	}


	printf("Creating child process\n");
	// Create child process
//	process_id = fork();
//	printf("Child process is  = %d\n",process_id);
//	cid = wait(&status);	
//	printf("After wait for PID %d\n",cid);

	// Indication of fork() failure
//	if (process_id < 0)
//	{
		// Return failure in exit status
//		exit(1);
//	}
	// PARENT PROCESS. Need to kill it.
//	if (process_id > 0)
//	{
//		exit(0);
//	}
	//unmask the file mode
//	umask(0);
//	printf("IN CHILD\n");
	

	//set new session
//	sid = setsid();
//	if(sid < 0)
//	{
//		printf("SID is less than 0\n");
		// Return failure
//		exit(1);
//	}
	// Change the current working directory to root.
//	chdir("/");
	
	while(1)
	{
		//Calling Producer 1
		printf("Calling producer 1\n");
		producer1();
		//Calling Producer 2
		printf("Calling producer 2\n");
	    	producer2();
		//Calling Consumer
		printf("Calling consumer\n");		
		consumer();
		usleep(500000);
	}
	
	sem_unlink(prod1_semaphore);
	sem_unlink(prod2_semaphore);
	sem_unlink(cons_semaphore);
	printf("Semaphore unlinked!\n");

	// Close stdin. stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
        return 0;
}
