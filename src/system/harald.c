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
<<<<<<< HEAD
		unsigned char data = receiveByte_up();
		//PORTA = data;
		transmitByte_down(data);
		
=======
		//unsigned char data = receiveByte_up();
		//transmitByte_down(data);
		testcase1();
>>>>>>> d2fe86ce1a657419012b5d108ac31e0d308453df

    }
}

void testcase1(){
	unsigned char data = receiveByte_up;
	data &= 7F;
	if (data == 06){
		// right down
		transmitByte_down(06); // exakt en byte
		//transmitByte_down(data); // oklart om den innehåller parity och typ stoppbitar
	}
}


