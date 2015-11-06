#include "lib/usart.h"

/* Initialize USART. */
void init_USART_up(){
	
	unsigned int baud = 7;
	
	/* Set baud rate */
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;
			
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
			
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (0<<USBS0)|(3<<UCSZ00);		
}

unsigned char receiveByte_up()
{
	while (!(UCSR0A & (1<<RXC0)));
	
	PORTA |= 1 << PORTA0;
	/* Get and return received data from buffer */
	unsigned char data = UDR0;
	
	return data;
}

void transmitByte_up(unsigned char data){
    /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1<<UDRE0)) );
    
    /* Put data into buffer, sends the data */
    UDR0 = data;
}