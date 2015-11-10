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
		transmitByte_down(data);

    }
}


