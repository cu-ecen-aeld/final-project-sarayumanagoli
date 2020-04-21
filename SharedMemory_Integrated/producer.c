/**
 * Simple program demonstrating shared memory in POSIX systems.
 *
 * This is the producer process that writes to the shared memory region.
 *
 * Figure 3.17
 *
 * @author Silberschatz, Galvin, and Gagne
 * Operating System Concepts  - Ninth Edition
 * Copyright John Wiley & Sons - 2013
 */

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

const char *ID1 = '1';
const float message0;
const char *ID2 = '2';
const float message1;

void temperature_init(void)
{
	char check_val[1] = {TEMP_REGISTER};

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
}

void TMP_102(void)
{
	char read_val[2] = {0};
	int16_t digitalTemp;
	float tempC;
	
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
	message0 = tempC;
}

void MQ_135(void)
{
	
	uint8_t fd; 
	char buffer[MAX_BUF]; 
	char val[4]; 
	uint16_t value_read = 0;
	float float_value = 0.0;

	printf("Message from PRODUCER 2\n");

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
	message1 = float_value
}


int main()
{
	const int SIZE = 10000;
	temperature_init();
	int shm_fd;
	void *ptr;

	const char *name = "AESD";	
	
	/* create the shared memory segment */
	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);

	/* configure the size of the shared memory segment */
	ftruncate(shm_fd,SIZE);

	printf("Creating child process\n");
	// Create child process
	process_id = fork();
	printf("Child process is  = %d\n",process_id);
	
	//cid = waitpid(process_id,&status,0);
	cid = wait(&status);	
	printf("After wait for PID %d\n",cid);

	
	ret = kill(process_id,0);
	printf("Kill returned %d\n", ret);

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
	printf("IN CHILD\n");
	

	//set new session
	sid = setsid();
	if(sid < 0)
	{
		printf("SID is less than 0\n");
		// Return failure
		exit(1);
	}
	// Change the current working directory to root.
	chdir("/");


	/* now map the shared memory segment in the address space of the process */
	ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Map failed\n");
		return -1;
	}

	/**
	 * Now write to the shared memory region.
 	 *
	 * Note we must increment the value of ptr after each write.
	 */
	sprintf(ptr,"%s",ID0);
	ptr += strlen(ID0);
	sprintf(ptr,"%s",message1);
	ptr += strlen(message1);
	sprintf(ptr,"%s",message2);
	ptr += strlen(message2);

	return 0;
}

