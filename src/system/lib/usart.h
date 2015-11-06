
#ifndef USART_H
#define USART_H

#include <avr/io.h>

/* Initializes USART connection to the module upwards. */
void init_USART_up(void);

/* Initializes USART connection to the module downwards. */
void init_USART_down(void);

/* Listens and receives a byte from the upward module. */
unsigned char receiveByte_up(void);

/* Listens and receives a byte from the downward module. */
unsigned char receiveByte_down(void);

/* Transmits a byte to the upward module. */
void transmitByte_up(unsigned char data);

/* Transmits a byte to the downward module. */
void transmitByte_down(unsigned char data);

#endif