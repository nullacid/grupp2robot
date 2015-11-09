#include "usart.h"

/* Initialize USART. */
void init_USART_up(){
	
	unsigned int baud = 7;
	
	/* Set baud rate */
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;

	/* UCSZn0,1,2 as 010 gives 7 bit frame size, UPMn0,1 as 01 gives enabled even parity
	 USBS0 as 0 gives 1 stopbit */
	UCSR0C = (1<<UPM01)|(1<<UPM00)|(0<<UCSZ00)|(1<<UCSZ01)|(0<<UCSZ02)|(0<<USBS0);

	//RXEN0=1 enables receive and TXEN0=1 enables transmit
	UCSR0B = (0<<UCSZ02)|(1<<RXEN0)|(1<<TXEN0);	
}

void init_USART_down(){
	unsigned int baud = 7;
	
	/* Set baud rate */
	UBRR1H = (unsigned char)(baud>>8);
	UBRR1L = (unsigned char)baud;

	/* UCSZn0,1,2 as 010 gives 7 bit frame size, UPMn0,1 as 01 gives enabled even parity
	 USBS0 as 0 gives 1 stopbit */
	UCSR1C = (1<<UPM11)|(0<<UPM10)|(0<<UCSZ10)|(1<<UCSZ11)|(0<<UCSZ12)|(0<<USBS1);

	//RXEN0=1 enables receive and TXEN0=1 enables transmit
	UCSR1B = (0<<UCSZ12)|(1<<RXEN1)|(1<<TXEN1);	
}

/* Returns the value in the receivebuffer if the paritybit is correct, 0xFF otherwise */
unsigned char receiveByte_up()
{
	//PORTA |= (1<<PORTA2) | (1<<PORTA3);
	while (!(UCSR0A & (1<<RXC0)));
	
	/* If upe0=1 parity check failed */
	if (!(UCSR0A & (1<<UPE0))){
		/* Get and return received data from buffer */
		unsigned char data = UDR0;
		transmitOK_up();
		return data;
	}
	else{
		transmitERROR_up();
	}
	return 0xFF;
}

unsigned char receiveByte_down()
{
	while (!(UCSR1A & (1<<RXC1)));
	
	/* If upe0=1 parity check failed */
	if (!(UCSR1A & (1<<UPE1))){
			/* Get and return received data from buffer */
		//unsigned char data = UDR0;
		//return data;
		unsigned char data = UDR1;
		transmitOK_down();
		return data;
	}
	return 0xFF;
	transmitERROR_down();
}

/* Transmit data to the module above */
void transmitByte_up(unsigned char data){
    /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1<<UDRE0)) );
    
    /* Put data into buffer, sends the data */
    UDR0 = data;
	
	if(responseError_up()){
		transmitByte_up(data);
	}
}

/* Transmit data to the module below */
void transmitByte_down(unsigned char data){
    /* Wait for empty transmit buffer */
    while ( !( UCSR1A & (1<<UDRE1)) );
    
    /* Put data into buffer, sends the data */
    UDR1 = data;
	
	if(responseError_down()){
		transmitByte_down(data);
	}
}

/* Transmits an ok to the module upstairs. */
void transmitOK_up(void){
    /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1<<UDRE0)) );
    
    /* Put data into buffer, sends the data */
    UDR0 = 0x7e;
}

void transmitERROR_up(void){
    /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1<<UDRE0)) );
    
    /* Put data into buffer, sends the data */
    UDR0 = 0x7f;
}

void transmitOK_down(void){
    /* Wait for empty transmit buffer */
    while ( !( UCSR1A & (1<<UDRE1)) );
    
    /* Put data into buffer, sends the data */
    UDR1 = 0x7e;
}

void transmitERROR_down(void){
    /* Wait for empty transmit buffer */
    while ( !( UCSR1A & (1<<UDRE1)) );
    
    /* Put data into buffer, sends the data */
    UDR1 = 0x7f;
}

int responseError_up(){
	//PORTA |= (1<<PORTA2) | (1<<PORTA3);
	while (!(UCSR0A & (1<<RXC0)));
	/* If upe0=0 parity check failed */
	if (!(UCSR0A & (1<<UPE0))){
		/* Get and return received data from buffer */
		//unsigned char data = UDR0;
		//return data;
		unsigned char data = UDR0;
		
		PORTA = (1 << PORTA0);
		
		if(data == 0x7e){
			return 0;
		}
		return 1;
	}
	return 0;
}

int responseError_down(){
	unsigned char data = receiveByte_down();
	if(data == 0x7e){
		return 0;
	}
	return 1;
}