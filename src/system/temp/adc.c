/*
 * adc.c
 *
 * Created: 11/9/2015 2:49:39 PM
 *  Author: micso554
 */

#include <avr/io.h>
#include <util/delay.h>

void adc_init()
{
	// AREF = AVcc
	// ADLAR = 1 gives left adjusted data (8 MSB values in ADCH)
	ADMUX = (1<<REFS0)|(1<<ADLAR);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// read adc value
uint8_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with '7' will always keep the value
	// of 'ch' between 0 and 7
	ch &= 0b00000111;  // AND operation with 7
	ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
	
	// start single conversion
	// write '1' to ADSC
	ADCSRA |= (1<<ADSC);
	
	// wait for conversion to complete
	// ADSC becomes '0' again
	// till then, run loop continuously
	while(ADCSRA & (1<<ADSC));
	
	return (ADCH);
}

int main()
{
	DDRB = 0xff;
	
	// initialize adc
	adc_init();
	_delay_ms(50);
	
	while(1)
	{
		PORTD = adc_read(0);
		_delay_ms(50);
	}
}