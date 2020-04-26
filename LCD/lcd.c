#include <stdio.h>
#include <stdint.h>
#include "lcd.h"

void msdelay(unsigned int time)  // Function for creating delay in milliseconds.
{
    unsigned i,j ;
    for(i=0;i<time;i++)
    for(j=0;j<1275;j++);
}

void lcdbusywait()
{
    while(*read_instruct & 0x80) //D7 bit of the LCD is polled for BF
    {
        ;
    }
}

void lcdgotoaddr(unsigned char addr)
{
    *write_instruct = (addr-0x80);  //moves the cursor to the required 'addr'
    lcdbusywait(); //poll for BF
}


void lcdgotoxy(unsigned char row, unsigned char column)
{
	if (row == 1)
		lcdgotoaddr(0x00+column); //to go to a particular column in row 1
	else if (row == 2)
		lcdgotoaddr(0x40+column); //to go to a particular column in row 2
	else if (row == 3)
		lcdgotoaddr(0x10+column); //to go to a particular column in row 3
	else
		lcdgotoaddr(0x50+column); //to go to a particular column in row 4
}

void lcdputch(char cc)
{
    *write_data = cc;
    uint8_t temp = *read_instruct;
    lcdbusywait();  //poll for BF
    if(temp == 0x8F) //to wrap to row 2
    {
        lcdgotoaddr(0x40);
    }
    if(temp == 0xCF)   //to wrap to row 3
    {
        lcdgotoaddr(0x10);
    }
    if(temp == 0x9F)    //to wrap to row 4
    {
        lcdgotoaddr(0x50);
    }
    if(temp == 0xDF)    //to wrap to row 1
    {
        lcdgotoaddr(0x00);
    }

}

void lcdputstr(char *ss)
{
    for(uint8_t i = 0;ss[i]!='\0';i++)
    {
        lcdputch(ss[i]);
    }
}

 void lcdinit()    //Function to prepare the LCD  and get it ready
{
    msdelay(15);
    *write_instruct = 0x30;  // for using 2 lines and 5X7 matrix of LCD
    msdelay(5);
    *write_instruct = 0x30;  // for using 2 lines and 5X7 matrix of LCD
    msdelay(100);
    *write_instruct = 0x30;  // for using 2 lines and 5X7 matrix of LCD
    lcdbusywait();  //poll for BF
    *write_instruct = 0x38;  // for using 2 lines and 5X7 matrix of LCD
    lcdbusywait();  //poll for BF
    *write_instruct = 0x08;  // turn display ON, cursor blinking
    lcdbusywait();  //poll for BF
    *write_instruct = 0x0C;  // turn display ON, cursor blinking
    lcdbusywait();  //poll for BF
    *write_instruct = 0x06;
    lcdbusywait();  //poll for BF
    *write_instruct = 0x01;  // clear screen
    lcdbusywait();  //poll for BF
}

void lcdclear(void)
{
    *write_instruct = 0x80;  // bring cursor to position 1 of line 1
    lcdbusywait();  //poll for BF
    *write_instruct = 0x01;  // clear screen
    lcdbusywait();  //poll for BF
}

 void lcd_init()    //Function to prepare the LCD  and get it ready
{
    msdelay(100);
    *write_instruct = 0x38;  // for using 2 lines and 5X7 matrix of LCD
    msdelay(100);
    *write_instruct = 0x38;  // for using 2 lines and 5X7 matrix of LCD
    msdelay(100);
    *write_instruct = 0x38;  // for using 2 lines and 5X7 matrix of LCD
    msdelay(100);
    *write_instruct = 0x0E;  // turn display ON, cursor blinking
    msdelay(100);
    *write_instruct = 0x06;  // bring cursor to position 1 of line 1
    msdelay(100);
    *write_instruct = 0x01;  // clear screen
    msdelay(100);
}

int main()
{
    unsigned char a[10] = "HELLO!";
    lcd_init();
    for(uint8_t i = 0;i<10;i++)
    {
        *write_data = a[i];
        msdelay(100);
    }
	return 0;
}
