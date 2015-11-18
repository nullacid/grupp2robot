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
    
    uint8_t mapData[1156];
    uint8_t mapSize = 0;

	

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
                //if (data == 0x58){
                  //  mapData[mapSize] = returnDataArray[i];
                  //  mapSize++;
                //}
            datalength--;
            i++;
		}
        while(datalength != i){
            transmitByte_up(returnDataArray[datalength]);
            datalength++;
		}
    }
}


