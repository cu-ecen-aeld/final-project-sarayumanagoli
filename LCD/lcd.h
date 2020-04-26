#ifndef _I2C_H
#define _I2C_H

#include <stdio.h>
#include <stdint.h>

#define write_instruct (uint8_t *)0x8000  //RS pin connected to pin 2 of port 3
#define read_instruct (uint8_t *)0xC000
#define read_data (uint8_t *)0xE000  // RW pin connected to pin 3 of port 3
#define write_data (uint8_t *)0xA000
#define e (uint8_t *)0x8000  //E pin connected to pin 4 of port 3

void msdelay(unsigned int time);
void lcdbusywait();
void lcdgotoaddr(unsigned char addr);
void lcdgotoxy(unsigned char row, unsigned char column);
void lcdputch(char cc);
void lcdputstr(char *ss);
void lcdinit();
void lcdclear(void);

#endif
