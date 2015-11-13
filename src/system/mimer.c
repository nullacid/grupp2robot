/*
 * adc.c
 *
 * Created: 11/9/2015 2:49:39 PM
 *  Author: micso554
 */

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include "lib\usart.h"

uint8_t cm_values[120] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 90, 85, 79, 75, 71, 67, 64, 61, 58, 55, 53, 51, 49, 47, 45, 43, 42, 40, 39, 38, 37, 35, 34, 33, 32, 31, 31, 30, 29, 28, 28, 27, 26, 26, 25, 24, 24, 23, 23, 22, 22, 21, 21, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 15, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9};

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
int16_t adc_read(int16_t ch)
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

int16_t adc_to_cm(int16_t value){
	if (value > 120)
		return 0;
	return cm_values[value];
}


/* These functions should get the internally stored values and transmit them, the functions sending two bytes will probably need to do some shifting. */

/* Transmits data from LIDAR. 2 bytes. */
void transmitLidar(){
	transmitByte_up(0x05);
	transmitByte_up(0x08);
}
/* Transmits data from the right front IR sendor. 1 byte. */
void transmitIRRF(){
	transmitByte_up(0x5d);
}
/* Transmits data from the right back IR sensor. 1 byte. */
void transmitIRRB(){
	transmitByte_up(0x73);
}
/* Transmits data from the left front sensor. 1 byte. */
void transmitIRLF(){
	transmitByte_up(0x34);
}
/* Transmits data from the left back IR sensor. 1 byte. */
void transmitIRLB(){
	transmitByte_up(0x23);
}
/* Transmits data from the Gyro. 2 bytes. */
void transmitGyro(){
	transmitByte_up(0x21);
	transmitByte_up(0x22);
}

void transmitLidarT(){
	transmitByte_up(0x99);
}

void transmitParallelR(){
	transmitByte_up(0x5A);
}

void transmitParallelL(){
	transmitByte_up(0x5B);
}

void transmitGyroT(){
	transmitByte_up(0xAB);
}

void transmitIRRFT(){
	transmitByte_up(0x0A);
}

void transmitIRRBT(){
	transmitByte_up(0x0B);
}

void transmitIRLFT(){
	transmitByte_up(0x0C);
}

void transmitIRLBT(){
	transmitByte_up(0x0D);
}

/* Transmits all sensor data. */
/* Transmitted data order will be LIDAR1 -> LIDAR2 -> IRRF -> IRRB -> IRLF -> GYRO1 -> GYRO2 -> LIDAR token -> Parallel Right -> Parallel Left -> Gyro token -> IRRF token -> IRRB token ->
	-> IRLF token -> IRLB token. */
void transmitALL(){
	transmitLidar();
	transmitIRRF();
	transmitIRRB();
	transmitIRLF();
	transmitIRLB();
	transmitGyro();
	transmitLidarT();
	transmitParallelR();
	transmitParallelL();
	transmitGyroT();
	transmitIRRFT();
	transmitIRRBT();
	transmitIRLFT();
	transmitIRLBT();
}

/* Translates the command code into the corresponding function. */
void processCommand(unsigned char data){
	if(data == 0x08){
		transmitLidar();
	}
	else if(data == 0x09){
		transmitIRRF();
	}
	else if(data == 0x0A){
		transmitIRRB();
	}
	else if(data == 0x0B){
		transmitIRLF();
	}
	else if(data == 0x0C){
		transmitIRLB();
	}
	else if(data == 0x0D){
		transmitGyro();
	}
	else if(data == 0x0F){
		transmitLidarT();
	}
	else if(data == 0x10){
		transmitParallelR();
	}
	else if(data == 0x11){
		transmitParallelL();
	}
	else if(data == 0x12){
		transmitGyroT();
	}
	else if(data == 0x13){
		transmitIRRFT();
	}
	else if(data == 0x14){
		transmitIRRBT();
	}
	else if(data == 0x15){
		transmitIRLFT();
	}
	else if(data == 0x16){
		transmitIRLBT();
	}
	else if(data == 0x1D){
		transmitALL();
	}
}

int main()
{
	uint8_t i = 0;
	
	int16_t adc_res[4] = {0,0,0,0};
	int16_t adc_median = 0;
	
	// initialize adc
	adc_init();
	
	//Baud rate is 10
	init_USART_up(10);
	
	_delay_ms(50);
	
	while(1)
	{
		// display adc value as cm on port D	
		//PORTD = adc_to_cm(adc_read(0));
		/*
		adc_res[i] = adc_to_cm(adc_read(0));
		
		if (i == 3){
			i = 0;
			adc_median = (adc_res[0] + adc_res[1] + adc_res[2] + adc_res[3])/(4) ;
		
			PORTD = adc_median;
		}
		i++;
		*/
		
		
		if(checkUSARTflag_up()){
			data = receiveByte_up();
			data &= 0x3F	//Maska ut kommandot
			processCommand(data);
		}
		
		
		PORTD = adc_to_cm(adc_read(0));
	
		_delay_ms(50);
	}
}