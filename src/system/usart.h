
#ifndef USART_H
#define USART_H

#include <avr/io.h>

/* Initializes USART connection to the module upwards. */
void init_USART_up(unsigned int baud);

/* Initializes USART connection to the module downwards. */
void init_USART_down(unsigned int baud);

/* Listens and receives a byte from the upward module. */
unsigned char receiveByte_up(void);

/* Listens and receives a byte from the downward module. */
unsigned char receiveByte_down(void);

/* Transmits a byte to the upward module. */
void transmitByte_up(unsigned char data);

/* Transmits a byte to the downward module. */
void transmitByte_down(unsigned char data);

/* Returns 1 if there is data in receivebuffer from usart0 */
int checkUSARTflag_up(void);

/* Returns 1 if there is data in receivebuffer from usart1 */
int checkUSARTflag_down(void);

#endif