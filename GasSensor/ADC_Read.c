#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 

#define MAX_BUF 200 

#define SYSFS_ADC_DIR "/sys/bus/iio/devices/iio:device0/in_voltage4_raw" 

int adc_read(unsigned int pin) 
{ 
  int fd; 
  char buf[MAX_BUF]; 
  char val[4]; 

  //len = snprintf(buf, sizeof(buf), "/sys/bus/iio/devices/iio\\:device0/in_voltage%d_raw", pin); 
  snprintf(buf, sizeof(buf), SYSFS_ADC_DIR); 
  printf("opening file: %s\n" ,  buf); 

  fd = open(buf, O_RDONLY); 
  if (fd < 0) 
  { 
     perror("adc/get-value"); 
  } 

  read(fd, &val, 4); 
  close(fd); 

  return atoi(val); 
} 


int main() 
{ 

   printf("adc: %d\n" , adc_read(4)); 
   return 0;
} 
