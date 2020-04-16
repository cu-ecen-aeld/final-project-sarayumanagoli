#include <stdio.h>
#include <stdint.h>
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

/* Function to check if a given file exists */
bool file_exists(char *filename)
{
	struct stat buffer;
	return (stat(filename,&buffer) == 0);
}

int main()
{
	char *file_location = "/var/tmp/temperature";	
	char *data = malloc(30 * sizeof(char));
	char check_val[1] = {TEMP_REGISTER};
	char read_val[2] = {0};
	int temp_file, data_file;
	int16_t digitalTemp;
	float tempC, tempF;
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

	while(1)
	{
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
		tempF = (tempC * (9/5)) + 32;
		sprintf(data, "\nThe temperature in C is %fC and in F is %fF.", tempC, tempF);
		if(write(data_file, data, strlen(data)) == -1)
		{
			perror("Write to data file failed!");
			exit(EXIT_FAILURE);
		}
		printf("\nData written to file!");
/*		char *read_buffer = malloc(strlen(data) * sizeof(char));
//		lseek(data_file,0,SEEK_SET);
		fileread = pread(data_file, read_buffer, strlen(data), len);
		len += fileread;
		read_buffer[fileread] = '\0';
		printf("\nData read from file is %s",read_buffer);
		free(read_buffer); */
		sleep(1);
	}
	close(temp_file);
	close(data_file);
	printf("\nFile closed!\n");
	return 0;
}
