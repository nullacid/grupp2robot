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
    
    uint_8 mapData[];
    mapSize = 0;

	

    while (1) 
    {
		unsigned char data = receiveByte_up();
		transmitByte_down(data);
		
        // Removes data and shifts it down
		uint8_t datalength = data & 0xC0;
		datalength = (datalength >> 6);
		
		unsigned char returnDataArray[3];
		
		unsigned int i = 0;

        while(datalength != 0){
            returnDataArray[i] = receiveByte_down();
                // if mapdata, store it for debugging
                if (data == 0x58){
                    mapData[size] = returnDataArray[i];
                    size++;
                }
            datalength--;
            i++;
		}
		if(i != 0){
			do{
				transmitByte_up(returnDataArray[2 - i]);
				i--;
			}while(i != 0);
		}
    }
}

void USART_Flush( void )
{
	unsigned char dummy;
	while ( UCSR1A & (1<<RXC1) ) dummy = UDR1;
}


