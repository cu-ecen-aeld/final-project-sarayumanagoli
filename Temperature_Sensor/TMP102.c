#include <stdio.h>
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
	char check_val[1] = {0x35};
	char read_val[2] = {0};
	int temp_file;
	printf("\nThis is a test for the TMP102 sensor");
	if((temp_file = open("/dev/i2c-2",O_RDWR)) < 0)
	{
		perror("\nFailed to open the I2C-2 bus!");
		exit(EXIT_FAILURE);
	}
	printf("\nSuccessfully opened the I2C-2 bus");
	if((ioctl(temp_file, I2C_SLAVE, DEV_ADDRESS)) < 0)
	{
		perror("\nFailed to connect to the sensor!");
		exit(EXIT_FAILURE);
	}
	printf("\nSuccessfully connected to the sensor!");
	printf("\nAttempting to write %x for POST",check_val[0]);
/*	lseek(temp_file, 1, SEEK_SET);
	if((write(temp_file, check_val, 1)) != 1)
	{
		perror("\nFailed to write to the check value to the configuration register");
		exit(EXIT_FAILURE);
	} 
	printf("\nWrite successful!"); */
//	lseek(temp_file, 1, SEEK_SET);
	if((read(temp_file, read_val, 2)) != 2)
	{
		perror("\nFailed to read the check value from the configuration register");
		exit(EXIT_FAILURE);
	}	
	printf("\nThe read value at 0 is 0x%x",read_val[0]);
	printf("\nThe read value at 1 is 0x%x",read_val[1]);
	printf("\nThe read value at 2 is 0x%x",read_val[2]);
	read_val[0] = 0;
	read_val[1] = 0;
	read_val[2] = 0;
	printf("\nRead val before read [0] = 0x%x, [1] = 0x%x, [2] = 0x%x",read_val[0], read_val[1], read_val[2]);
	printf("\nChecking positional read");
	printf("\nReading temperature...");
	if((pread(temp_file, read_val, 2, 0)) != 2)
	{
		perror("\nFailed to read the check value from the configuration register");
		exit(EXIT_FAILURE);
	}
	printf("\nThe read value at 0 is 0x%x",read_val[0]);
	printf("\nThe read value at 1 is 0x%x",read_val[1]);
	printf("\nThe read value at 2 is 0x%x",read_val[2]);
	close(temp_file);
	printf("\nFile closed!\n");
	return 0;
}
