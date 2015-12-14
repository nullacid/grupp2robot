/*
 * Created: November 2015
 * Author : Peter T and Victor T
 * "Ah, sweet alcohol. Like a true friend, you replace the anger with better, louder anger." -Erik
 *
 * A very simple communication module that exists purely as an interface between the system and the bluetooth
 * communication with CRAY™.
 */ 
#define F_CPU 14.7456E6
#define BAUD 7
#include <avr/io.h>
#include <util/delay.h>
#include "lib\usart.h"


int main(void)
{
	/*Initializes usart communication and set variables*/
	init_USART_up(BAUD);
	init_USART_down(BAUD);
    

    while (1) 
		{
			unsigned char data = receiveByte_up();
			//Is it a sync command?
			if(data == 0x26){
				transmitByte_up(0x26);
			}
			
			//For all other commands
			else{
				transmitByte_down(data);
		
				// Removes data and shifts it down
				uint8_t datalength = data & 0xC0;
				datalength = (datalength >> 6);
				
				// Array with all data to be transmitted up to CRAY
				unsigned char returnDataArray[3];
		
				unsigned int i = 0;

				// Receives all data
				while(datalength != 0){
					returnDataArray[i] = receiveByte_down_to();
					datalength--;
					i++;
				}
				// Transmits it to CRAY
				while(datalength != i){
					transmitByte_up(returnDataArray[datalength]);
					datalength++;
				}
			}
		}
	
}


