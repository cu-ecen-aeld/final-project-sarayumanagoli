#include "gpio.h"

// Function prototypes
void pin_Init(void);
void data_Register(void);
void command_Register(void);
void read_Value(void);
void write_Value(void);
void enable_High(void);
void enable_Low(void);
void data_Write(void);
void data_Read(void);
void check_Busy(void);
void send_Command(int arr[]);
void send_Data(int arr[]);
void send_String(char *str);
void dec_Binary(int n, int arr[]);
void lcd_Init(void);

