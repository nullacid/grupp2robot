/*
 * Created: November 2015
 * Author : Peter T and Victor T
 * "Ah, sweet alcohol. Like a true friend, you replace the anger with better, louder anger." -Erik
 * 
 * Header file for USART communication. USART is implemented in the system as asynchrońous communication
 * 
 */ 

#ifndef USART_H
#define USART_H

#include <avr/io.h>

/* Initializes USART connection to the module upwards. */
void init_USART_up(unsigned int baud);

/* Initializes USART connection to the module downwards. */
void init_USART_down(unsigned int baud);

/* Receives byte with timeout. */
unsigned char receiveByte_down_to(void);

/* Receives byte with timeout. */
unsigned char receiveByte_up_to(void);

/* Listens and receives a byte from the upward module. */
unsigned char receiveByte_up(void);

/* Listens and receives a byte from the downward module. */
unsigned char receiveByte_down(void);

/* Transmits a byte to the upward module. */
void transmitByte_up(unsigned char data);

/* Transmits a byte to the downward module. */
void transmitByte_down(unsigned char data);

/* Returns 1 if there is data in receive buffer from usart0 */
int checkUSARTflag_up(void);

/* Returns 1 if there is data in receive buffer from usart1 */
int checkUSARTflag_down(void);

/* Used to synchronize when receiving 2 bytes from different clock.
Otherwise the buffer might be overwritten by the second transmit before the first byte has been read. */
void transmitSendNext_down(void);

/* Used to synchronize when transmitting 2 bytes to different clock frequency.
Otherwise the buffer might be overwritten by the second transmit before the first byte has been read. */
void waitForSendNext_up(void);

#endif