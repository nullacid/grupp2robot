#include "lib/usart.h"

/* Initialize USART. */
void init_USART_up(){
	
	unsigned int baud = 7;
	
	/* Set baud rate */
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;

	/* UCSZn0,1,2 as 010 gives 7 bit frame size, UPMn0,1 as 01 gives enabled even parity
	 USBS0 as 0 gives 1 stopbit */
	UCSR0C |= (1<<UPM01)|(0<<UPM00)|(0<<UCSZ00)|(1<<UCSZ01)|(0<<USBS0);

	//RXEN0=1 enables receive and TXEN0=1 enables transmit
	UCSR0B |= (0<<UCSZ02)|(1<<RXEN0)|(1<<TXEN0);	
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