#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>	//for lseeks
#include <unistd.h>
#include <stdlib.h>	//for exit status

#define DEV_ADDRESS (0x48)
#define CONFIG_REGISTER (0x01)
#define TEMP_REGISTER (0x00)

int main()
{
	char check_val[1] = {TEMP_REGISTER};
	char read_val[2] = {0};
	int temp_file;
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
//	lseek(temp_file, 0, SEEK_SET);
	while(1)
	{
		if((read(temp_file, read_val, 2)) != 2)
		{
			perror("\nFailed to read the check value from the configuration register");
			exit(EXIT_FAILURE);
		}	
		//printf("\nThe read value at 0 is 0x%02x",read_val[0]);
		//printf("\nThe read value at 1 is 0x%02x",read_val[1]);
		digitalTemp = (((read_val[0]) << 4) | ((read_val[1]) >> 4));
		if(digitalTemp > 0x7FF)
		{
			digitalTemp |= 0xF000;
		}
		tempC = digitalTemp * 0.0625;
		tempF = (tempC * (9/5)) + 32;
		printf("\nThe temperature in C is %fC and in F is %fF", tempC, tempF);
		sleep(1);
	}
	close(temp_file);
	printf("\nFile closed!\n");
	return 0;
}
