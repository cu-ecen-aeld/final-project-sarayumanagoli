#include "lcd.h"

#define RS (63)
#define RW (62)
#define E (37)
#define D0 (38)
#define D1 (39)
#define D2 (34)
#define D3 (35)
#define D4 (36)
#define D5 (33)
#define D6 (32)
#define D7 (61)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"

int flag = 0;

void pin_Init(void)
{
	int ret = 0;
	//RS
	if((ret = gpio_export(RS)) != 0)
	{
		printf("Export error for RS\n");
		exit(1);
	}

	if((ret = gpio_set_dir(RS, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for RS\n");
		exit(1);
	}
	//RW
	if((ret = gpio_export(RW)) != 0)
	{
		printf("Export error for RW\n");
		exit(1);
	}

	if((ret = gpio_set_dir(RW, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for RW\n");
		exit(1);
	}
	//E
	if((ret = gpio_export(E)) != 0)
	{
		printf("Export error for E\n");
		exit(1);
	}

	if((ret = gpio_set_dir(E, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for E\n");
		exit(1);
	}
	//D0
	if((ret = gpio_export(D0)) != 0)
	{
		printf("Export error for D0\n");
		exit(1);
	}

	if((ret = gpio_set_dir(D0, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for D0\n");
		exit(1);
	}
	//D1
	if((ret = gpio_export(D1)) != 0)
	{
		printf("Export error for D1\n");
		exit(1);
	}

	if((ret = gpio_set_dir(D1, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for D1\n");
		exit(1);
	}
	//D2
	if((ret = gpio_export(D2)) != 0)
	{
		printf("Export error for D2\n");
		exit(1);
	}

	if((ret = gpio_set_dir(D2, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for D2\n");
		exit(1);
	}
	//D3
	if((ret = gpio_export(D3)) != 0)
	{
		printf("Export error for D3\n");
		exit(1);
	}

	if((ret = gpio_set_dir(D3, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for D3\n");
		exit(1);
	}
	//D4
	if((ret = gpio_export(D4)) != 0)
	{
		printf("Export error for D4\n");
		exit(1);
	}

	if((ret = gpio_set_dir(D4, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for D4\n");
		exit(1);
	}
	//D5
	if((ret = gpio_export(D5)) != 0)
	{
		printf("Export error for D5\n");
		exit(1);
	}

	if((ret = gpio_set_dir(D5, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for D5\n");
		exit(1);
	}
	//D6
	if((ret = gpio_export(D6)) != 0)
	{
		printf("Export error for D6\n");
		exit(1);
	}

	if((ret = gpio_set_dir(D6, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for D6\n");
		exit(1);
	}
	//D7
	if((ret = gpio_export(D7)) != 0)
	{
		printf("Export error for D7\n");
		exit(1);
	}

	if((ret = gpio_set_dir(D7, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for D7\n");
		exit(1);
	}
}	

void data_Register(void)
{
	int ret;

	if((ret = gpio_set_value(RS,1)) != 0)
	{
	    printf("Data register error!\n");
	    exit(1);
	}
}

void command_Register(void)
{
	int ret;

	if((ret = gpio_set_value(RS,0)) != 0)
	{
	    printf("Command register error!\n");
	    exit(1);
	}
}

void read_Value(void)
{
	int ret;

	if((ret = gpio_set_value(RW,1)) != 0)
	{
	    printf("Read value error!\n");
	    exit(1);
	}
}

void write_Value(void)
{
	int ret;

	if((ret = gpio_set_value(RW,0)) != 0)
	{
	    printf("Write value error!\n");
	    exit(1);
	}
}	

void enable_High(void)
{
	int ret;

	if((ret = gpio_set_value(E,1)) != 0)
	{
	    printf("Enable High error!\n");
	    exit(1);
	}
}

void enable_Low(void)
{
	int ret;

	if((ret = gpio_set_value(E,0)) != 0)
	{
	    printf("Enable Low error!\n");
	    exit(1);
	}
}

void data_Write(void)
{
	enable_High();
	usleep(1000);
	enable_Low();
}

void data_Read(void)
{
	enable_Low();
	usleep(1000);
	enable_High();
}

void check_Busy(void)
{
	int ret;
	unsigned int value = 1;

	if((ret = gpio_set_dir(D7, GPIO_DIR_INPUT)) != 0)
	{
		printf("Direction set error for D7\n");
		exit(1);
	}	
	printf("\nD7 is set as input and waiting...");
	while(value == 1)
	{	
		data_Read();
		if((ret = gpio_get_value(D7, &value)) != 0)
		{
			printf("Get value error for D7\n");
			exit(1);
		}
	}
	printf("\nLCD is free!");
	if((ret = gpio_set_dir(D7, GPIO_DIR_OUTPUT)) != 0)
	{
		printf("Direction set error for D7\n");
		exit(1);
	}
}
	
void send_Command(int arr[])
{
	int ret;
	
	command_Register();
	if(flag == 1)
	{
		read_Value();	
		printf("\nCheck if the LCD is not busy");
		check_Busy();
	}
	write_Value();

	if((ret = gpio_set_value(D0,arr[0])) != 0)
	{
	    printf("D0 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D1,arr[1])) != 0)
	{
	    printf("D1 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D2,arr[2])) != 0)
	{
	    printf("D2 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D3,arr[3])) != 0)
	{
	    printf("D3 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D4,arr[4])) != 0)
	{
	    printf("D4 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D5,arr[5])) != 0)
	{
	    printf("D5 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D6,arr[6])) != 0)
	{
	    printf("D6 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D7,arr[7])) != 0)
	{
	    printf("D7 set error!\n");
	    exit(1);
	}
	data_Write();
}

void send_Data(int arr[])
{
	int ret;

	if(flag == 1)
	{
		command_Register();
		read_Value();
		check_Busy();
	}
	data_Register();
	write_Value();

	if((ret = gpio_set_value(D0,arr[0])) != 0)
	{
	    printf("D0 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D1,arr[1])) != 0)
	{
	    printf("D1 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D2,arr[2])) != 0)
	{
	    printf("D2 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D3,arr[3])) != 0)
	{
	    printf("D3 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D4,arr[4])) != 0)
	{
	    printf("D4 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D5,arr[5])) != 0)
	{
	    printf("D5 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D6,arr[6])) != 0)
	{
	    printf("D6 set error!\n");
	    exit(1);
	}
	if((ret = gpio_set_value(D7,arr[7])) != 0)
	{
	    printf("D7 set error!\n");
	    exit(1);
	}
	data_Write();
}

void send_String(char *str)
{
	int temp, bin_temp[8];
	while(*str)
	{
		temp = (int)(*str);
		dec_Binary(temp, bin_temp);
		send_Data(bin_temp);
		*str++;
	}
}

void dec_Binary(int n, int arr[])
{
    // array to store binary number 
    int binaryNum[8]; 
    int k = 0;
  
    // counter for binary array 
    int i = 0; 
    while (n > 0) 
    { 
  
        // storing remainder in binary array 
        binaryNum[i] = n % 2; 
        n = n / 2; 
        i++; 
    } 
  
    // printing binary array in reverse order 
    for (int j = i - 1; j >= 0; j--, k++) 
        arr[k] = binaryNum[j]; 	
}

void lcd_Init(void)
{
	int command[] = {0,0,0,0, 1,1,0,0};
	printf("\nSend command 1");
	send_Command(command);
	usleep(5000);
	printf("\nSend command 2");
	send_Command(command);
	usleep(5000);
	printf("\nSend command 3");
	send_Command(command);
	usleep(5000);
	
//	command = {0,0,0,1, 1,1,0,0};
	command[0] = 0;
	command[1] = 0;
	command[2] = 0;
	command[3] = 1;
	command[4] = 1;
	command[5] = 1;
	command[6] = 0;
	command[7] = 0;
	printf("\nSend command 4");
	send_Command(command);
	usleep(10000);

//	command = {1,1,1,1, 0,0,0,0};
	command[0] = 1;
	command[1] = 1;
	command[2] = 1;
	command[3] = 1;
	command[4] = 0;
	command[5] = 0;
	command[6] = 0;
	command[7] = 0;
	printf("\nSend command 5");
	send_Command(command);
	usleep(10000);

//	command = {1,0,0,0, 0,0,0,0};
	command[0] = 1;
	command[1] = 0;
	command[2] = 0;
	command[3] = 0;
	command[4] = 0;
	command[5] = 0;
	command[6] = 0;
	command[7] = 0;
	printf("\nSend command 6");
	send_Command(command);
}

int main()
{
	printf("\nPin initialization!");
	pin_Init();
	printf("\nPins initializaed successfully!");
	printf("\nTrying to initialize LCD...");
	lcd_Init();
	printf("\nTrying to send Hi!");
	flag = 1;
	send_String("Hi");
	return 0;
}

#pragma GCC diagnostic pop
