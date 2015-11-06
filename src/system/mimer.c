// Program to SPI (serial peripheral interface) using AVR microcontroller (ATmega16)
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
 
void SPI_init();
unsigned char spi_tranceiver (uint8_t data);
int main()
{
 
	 SPI_init();                  //Initialize SPI Master
	 DDRD |= 0x01;                       //PD0 as Output for testing
	 
	 unsigned char data;                 //Received data stored here
	 uint8_t x = 0;
	while (1){
		data = spi_tranceiver(++x);
	}
}
 
void SPI_init() //SPI initialization
{
	DDRB=(1<<5)|(1<<3); // set MOSI SCK as output pin, rest as input
	SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPIE);; // Enable SPI
	sei();
}
 
unsigned char spi_tranceiver (uint8_t data)
{
	// Load data into the buffer
	SPDR = data;
	
	//Wait until transmission complete
	while(!(SPSR & (1<<SPIF) ));
	
	// Return received data
	return(SPDR);
}
 