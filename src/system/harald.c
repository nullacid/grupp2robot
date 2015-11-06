/*
 * Kommunikationsmodul.c
 *
 * Created: 03/11/2015 10:42:37
 * Author : Peter Victor
 */ 

#define F_CPU 14.7456E6
#include <avr/io.h>
#include <util/delay.h>

/* Initialize USART. */
void init_USART(){
	
	unsigned int baud = 7;
	
	/* Set baud rate */
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;
			
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
			
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (0<<USBS0)|(3<<UCSZ00);		
}

unsigned char receiveData()
{
	while (!(UCSR0A & (1<<RXC0)));
	
	PORTA |= 1 << PORTA0;
	/* Get and return received data from buffer */
	unsigned char data = UDR0;
	
	return data;
}

void transmitByte(unsigned char data){
    /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1<<UDRE0)) );
    
    /* Put data into buffer, sends the data */
    UDR0 = data;
}


int main(void)
{
	init_USART();
	
	//transmitByte(0x42);
    while (1) 
    {
		unsigned char data = receiveData();
		if(data == 0x42){
			transmitByte(0xFF);
		}
    }
}


