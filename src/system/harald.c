/*
 * Created: 03/11/2015 10:42:37
 * Author : Peter Victor
 * "Ah, sweet alcohol. Like a true friend, you replace the anger with better, louder anger." -Erik
 */ 
#define F_CPU 14.7456E6
#define BAUD 7
#define DATASLOTS 19
#include <avr/io.h>
#include <util/delay.h>
#include "lib\usart.h"


int main(void)
{
	/*Initializes usart communication and set variables*/
	init_USART_up(BAUD);
	init_USART_down(BAUD);
    
    uint8_t mapData[1156];
    uint8_t mapSize = 0;

	

    while (1) 
		{
			unsigned char data = receiveByte_up();
			//Is it a sync command?
			if(data == 0x26){
				transmitByte_up(0x26);
			}
			//Is it a transmit all command?
			else if(data == 0x1D){
				transmitByte_down(data);
				unsigned char returnDataArray[DATASLOTS];
				//Get all data from bjarne
				for(int i = 0; i < DATASLOTS; ++i){
					returnDataArray[i] = receiveByte_down_to();
				}
				//Transmit all data to CRAY
				for(int j = 0; j < DATASLOTS; ++j){
					transmitByte_up(returnDataArray[j]);
				}
			}
			//For all other commands
			else{
				transmitByte_down(data);
		
				// Removes data and shifts it down
				uint8_t datalength = data & 0xC0;
				datalength = (datalength >> 6);
		
				unsigned char returnDataArray[3];
		
				unsigned int i = 0;

				while(datalength != 0){
					returnDataArray[i] = receiveByte_down_to();
					datalength--;
					i++;
				}
				while(datalength != i){
					transmitByte_up(returnDataArray[datalength]);
					datalength++;
				}
			}
		}
	
}


