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
	init_USART_up(7);
	init_USART_down(7);

    while (1) 
    {
		unsigned char data = receiveByte_up();
		//testcase1();
		//data &= 0x7F;
		transmitByte_down(data);
    }
}

void USART_Flush( void )
{
	unsigned char dummy;
	while ( UCSR1A & (1<<RXC1) ) dummy = UDR1;
}

void testcase1(){
	unsigned char data = receiveByte_up();
	data &= 0x7F;
	if (data == 0x06){
		// right down
		transmitByte_down(0x06); // exakt en byte
		//transmitByte_down(data); // oklart om den innehåller parity och typ stoppbitar
	}
}


