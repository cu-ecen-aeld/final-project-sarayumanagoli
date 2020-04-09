//Program to test the basic functionality of Buildroot Setup
//Prints "Hello World!"

#include<stdio.h>
#include "syslog.h"

int main(void)
{
	printf("Hello World!\n");
	//Open syslog
	openlog("HELLOWORLD",LOG_PID|LOG_CONS,LOG_USER);
	syslog(LOG_INFO,"Message from HelloWorld!");
	return 0;
}
