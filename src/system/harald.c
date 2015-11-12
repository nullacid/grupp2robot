/*
 * Kommunikationsmodul.c
 *
 * Created: 03/11/2015 10:42:37
 * Author : Peter Victor
 */ 

#define F_CPU 14.7456E6
#define BAUD 7
#include <avr/io.h>
#include <util/delay.h>
#include "lib\usart.h"


int main(void)
{
	init_USART_up(BAUD);
	init_USART_down(BAUD);

	unsigned char returnDataArray[3];

    while (1) 
    {
		unsigned char data = receiveByte_up();
		unsigned int datalength = data & 0x60; //removes data and parity
		datalength = (datalength >> 5);
		unsigned int i = 0;
		transmitByte_down(data);

		while(datalength != 0){
			returnDataArray[i] = receiveByte_down();
			datalength--;
			i++;
		}
		
		do{
			transmitByte_up(returnDataArray[2 - i]);
			i--;
		}while(i != 0)
		
    }
}

void USART_Flush( void )
{
	unsigned char dummy;
	while ( UCSR1A & (1<<RXC1) ) dummy = UDR1;
}


