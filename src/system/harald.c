/*
 * Kommunikationsmodul.c
 *
 * Created: 03/11/2015 10:42:37
 * Author : Peter Victor
 */ 

#define F_CPU 14.7456E6
#include <avr/io.h>
#include <util/delay.h>
#include "lib\usart.h"


int main(void)
{
	init_USART_up();
	init_USART_down();

    while (1) 
    {
		unsigned char data = receiveByte_up();
		
		if(data == 0x01){
			transmitByte_up(0x43);
		}
		else if(data == 0x09){
			transmitByte_up(0x09);
		}
		else if(data == 0x89){
			transmitByte_up(0x89);
		}
		else if(data == 0x03){
			transmitByte_up(0x54);
		}
		else if(data == 0x7E){
			transmitByte_up(0x7E);
		}
		else{
			//transmitByte_up(0x01);
		}
    }
}


