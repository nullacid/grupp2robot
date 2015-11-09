#include "usart.h"

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

void init_USART_down(){
	unsigned int baud = 7;
	
	/* Set baud rate */
	UBRR1H = (unsigned char)(baud>>8);
	UBRR1L = (unsigned char)baud;

	/* UCSZn0,1,2 as 010 gives 7 bit frame size, UPMn0,1 as 01 gives enabled even parity
	 USBS0 as 0 gives 1 stopbit */
	UCSR1C |= (1<<UPM11)|(0<<UPM10)|(0<<UCSZ10)|(1<<UCSZ11)|(0<<USBS1);

	//RXEN0=1 enables receive and TXEN0=1 enables transmit
	UCSR1B |= (0<<UCSZ12)|(1<<RXEN1)|(1<<TXEN1);	
}

/* Returns the value in the receivebuffer if the paritybit is correct, 0xFF otherwise */
unsigned char receiveByte_up()
{
	while (!(UCSR0A & (1<<RXC0)));
	
	/* If upe0=1 parity check failed */
	if (!(UCSR0A & (1<<UPE0))){
			/* Get and return received data from buffer */
		//unsigned char data = UDR0;
		//return data;
		return UDR0;
	}

	return 0xFF;
}

unsigned char receiveByte_in()
{
	while (!(UCSR1A & (1<<RXC1)));
	
	/* If upe0=1 parity check failed */
	if (!(UCSR1A & (1<<UPE1))){
			/* Get and return received data from buffer */
		//unsigned char data = UDR0;
		//return data;
		return UDR1;
	}

	return 0xFF;
}

/* Transmit data to the module above */
void transmitByte_up(unsigned char data){
    /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1<<UDRE0)) );
    
    /* Put data into buffer, sends the data */
    UDR0 = data;
}

/* Transmit data to the module below */
void transmitByte_in(unsigned char data){
    /* Wait for empty transmit buffer */
    while ( !( UCSR1A & (1<<UDRE1)) );
    
    /* Put data into buffer, sends the data */
    UDR1 = data;
}