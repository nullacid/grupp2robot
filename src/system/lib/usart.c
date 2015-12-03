#include "usart.h"
#include <util/delay.h>

/* Initialize USART. */
void init_USART_up(unsigned int baud){
	/* Set baud rate */
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;
	
	flushUSART_up();

	/* UCSZn0,1,2 as 110 gives 8 bit frame size, UPMn0,1 as 00 disables parity
	 USBS0 as 0 gives 1 stopbit, UMSELn as 00 asyncronus usart, UCPOLn as 0: triggered on positive flank */
	UCSR0C = (0<<UPM01)|(0<<UPM00)|(1<<UCSZ00)|(1<<UCSZ01)|(0<<USBS0)|(0<<UMSEL01) | (0<<UMSEL00)|(0<<UCPOL0);
	
	UCSR0A = (0<<U2X0);

	//RXEN0=1 enables receive and TXEN0=1 enables transmit
	UCSR0B = (0<<UCSZ02)|(1<<RXEN0)|(1<<TXEN0);	

}

void init_USART_down(unsigned int baud){
	/* Set baud rate */
	UBRR1H = (unsigned char)(baud>>8);
	UBRR1L = (unsigned char)baud;
	
	flushUSART_down();
	
	UCSR1A = (0<<U2X1);

	/* UCSZn0,1,2 as 110 gives 8 bit frame size, UPMn0,1 as 00 disables parity
	 USBS0 as 0 gives 1 stopbit, UMSELn as 00 asyncronus usart, UCPOLn as 0: triggered on positive flank */
	UCSR1C = (0<<UPM11)|(0<<UPM10)|(1<<UCSZ10)|(1<<UCSZ11)|(0<<USBS1) | (0<<UMSEL11) | (0<<UMSEL10)|(0<<UCPOL1);

	//RXEN0=1 enables receive and TXEN0=1 enables transmit
	UCSR1B = (0<<UCSZ12)|(1<<RXEN1)|(1<<TXEN1);
}

/* Returns the value in the receivebuffer from the module above */
unsigned char receiveByte_up()
{
	// Wait for something to be written in the buffer
	while (!(UCSR0A & (1<<RXC0)));
	
	unsigned char data = UDR0;
	return data;
}

/* Returns the value in the receivebuffer from the module below */
unsigned char receiveByte_down()
{
	// Wait for something to be written in the buffer
	while (!(UCSR1A & (1<<RXC1)));
	
	unsigned char data = UDR1;
	return data;
}

/* Transmit data to the module above */
void transmitByte_up(unsigned char data){
    /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1<<UDRE0)) );
    
    /* Put data into buffer, sends the data */
    UDR0 = data;
	
}

/* Transmit data to the module below */
void transmitByte_down(unsigned char data){
    /* Wait for empty transmit buffer */
    while ( !( UCSR1A & (1<<UDRE1)) );
    
    /* Put data into buffer, sends the data */
    UDR1 = data;
}

/* Returns 1 if there is something in the receivebuffer */
int checkUSARTflag_up(){
	return (UCSR0A & (1<<RXC0));
}

/* Returns 1 if there is something in the receivebuffer */
int checkUSARTflag_down(){
	return (UCSR1A & (1<<RXC1));
}

void transmitSendNext_down(){
	transmitByte_down(0x40);
}

void waitForSendNext_up(){
	_delay_ms(30);
}

void flushUSART_down(){
	int dummy;
	while ( UCSR1A & (1<<RXC0) ) dummy = UDR1;
}

void flushUSART_up(){
	int dummy;
	while ( UCSR0A & (1<<RXC0) ) dummy = UDR0;
}

