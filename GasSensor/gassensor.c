// Course : Advanced Embedded Software Development
// Final Project : GAS SENSOR INTERFACE
// Author : Sarayu Managoli (SAMA2321)
// Code Reference : http://eliaradeverydayembedded.blogspot.com/2017/08/how-to-configure-adc-for-beaglebone-in.html

#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <stdint.h>

#define MAX_BUF 200 

#define SYSFS_ADC_DIR "/sys/bus/iio/devices/iio:device0/in_voltage4_raw" 

void read_value(unsigned int pin) 
{ 
	uint8_t fd; 
	char buffer[MAX_BUF]; 
	char val[4]; 
	uint16_t value_read = 0;

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
	if(value_read > 600)
	{
		printf("Value is %d\tGas detected!\n",value_read);
	}
	else
	{
		printf("Value is %d\tGas not detected!\n",value_read);
	}
} 


int main() 
{ 
	read_value(4);
	return 0;
} 
