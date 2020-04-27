/********************************************************************************************************************************************************************************************
*	Author - Gitanjali Suresh
*	Filename - gpio.h
*	About - This is the header file for the application gpio.c which defines the necessary constants and function prototypes
********************************************************************************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

 /****************************************************************
 * Constants
 ****************************************************************/

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64
#define GPIO_DIR_INPUT  (0)
#define GPIO_DIR_OUTPUT (1)

 /****************************************************************
 * Function prototypes
 ****************************************************************/
int gpio_export(unsigned int gpio);
int gpio_unexport(unsigned int gpio);
int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
int gpio_set_value(unsigned int gpio, unsigned int value);
int gpio_get_value_fd(int fd, unsigned int *value);
int gpio_get_value(unsigned int gpio, unsigned int *value);
int gpio_set_edge(unsigned int gpio, const char *edge);
int gpio_fd_open(unsigned int gpio);
int gpio_fd_close(int fd);
