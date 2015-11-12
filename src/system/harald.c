/*
 * Kommunikationsmodul.c
 *
 * Created: 03/11/2015 10:42:37
 * Author : Peter Victor
 */ 

#define F_CPU 14.7456E6
#define BAUD 7
#include <avr/io.h>
#include "lib\usart.h"


int main(void)
{
	init_USART_up(BAUD);
	init_USART_down(BAUD);

	unsigned char returnDataArray[3];

    while (1) 
    {
		unsigned char data = receiveByte_up();
		unsigned int datalength = data & 0xC0; //removes data and parity
		datalength = (datalength >> 5);
		
		transmitByte_down(data);

		unsigned int i = 0;

		while(datalength != 0){
		returnDataArray[i] = receiveByte_down();
		datalength--;
		i++;
		}

		do{
		transmitByte_up(returnDataArray[2 - i]);
		i--;
		}while(i != 0);

		//i = waitForResponse(&returnDataArray);
		
		//transmitBytes_up(returnDataArray, i);

    }
}

void USART_Flush( void )
{
	unsigned char dummy;
	while ( UCSR1A & (1<<RXC1) ) dummy = UDR1;
}

unsigned int waitForResponse(unsigned char* returnDataArray, unsigned int datalength){
	unsigned int i = 0;
	while(datalength != 0){
		returnDataArray[i] = receiveByte_down();
		datalength--;
		i++;
	}
	return i;
}

void transmitBytes_up(unsigned char returnDataArray, unsigned int i){
	do{
		transmitByte_up(returnDataArray[2 - i]);
		i--;
	}while(i != 0);
}


