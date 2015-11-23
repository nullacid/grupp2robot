/*
 * adc.c
 *
 * Created: 11/9/2015 2:49:39 PM
 *  Author: micso554
 */

#define F_CPU 20000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include "lib\usart.h"

uint8_t cm_values[120] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 90, 85, 79, 75, 71, 67, 64, 61, 58, 55, 53, 51, 49, 47, 45, 43, 42, 40, 39, 38, 37, 35, 34, 33, 32, 31, 31, 30, 29, 28, 28, 27, 26, 26, 25, 24, 24, 23, 23, 22, 22, 21, 21, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 15, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9};
	
uint8_t IRLF = 0x01;
uint8_t IRLB = 0x02;
uint8_t IRRF = 0x03;
uint8_t IRRB = 0x04;

uint8_t IRLFT = 0x0a;
uint8_t IRLBT = 0x0b;
uint8_t IRRFT = 0x0c;
uint8_t IRRBT = 0x0d;

uint8_t parallelL = 0xa0;
uint8_t parallelR = 0xb0;

uint8_t lidarT = 0x45;

uint8_t LIDAR_H = 0xa0;
uint8_t LIDAR_L = 0xa1;

// current PWM-value
int current = 0;

void timer1_init(){
	// reset registers and set prescaler to 1024
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B |= (1<<CS10)|(0<<CS11)|(1<<CS12);
	
	// reset timer
	TCNT1 = 0;
}

void adc_init()
{
	// AREF = AVcc
	// ADLAR = 1 gives left adjusted data (8 MSB values in ADCH)
	ADMUX = (1<<REFS0)|(1<<ADLAR);
	
	// ADC Enable and prescaler of 128
	// 20 000 000/128 = 156 250 Hz (50 kHz < 156 kHz < 200 kHz)
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
		return 1;
	return cm_values[value];
}

/* Calculates the token value given pointers to the stored values. */
void calcTokenIR(uint8_t *IRxx, uint8_t *IRxxT){
	if(1 < *IRxx && *IRxx <= 32){
		*IRxxT = 1;
	}
	else if(32 < *IRxx && *IRxx < 90){
		*IRxxT = 2;
	}
	else{
		*IRxxT = 0;
	}
}

/* Calculates the token values for the IR sensors. A 1 represents a value between 1 and 32 (a wall within 1 square), 2 represents a value between 32 and 89 ( a wall within 2 squares). */
void calcTokensIR(){	
	calcTokenIR(&IRLB, &IRLBT);
	calcTokenIR(&IRLF, &IRLFT);
	calcTokenIR(&IRRB, &IRRBT);
	calcTokenIR(&IRRF, &IRRFT);
}

void calcLidarT(){
	uint16_t lidarValue = LIDAR_H*256 + LIDAR_L;
	lidarT = lidarValue / 40;
}

/* Calculates the token values for parallelity by taking the quotient of the front and back sensors. 0 represents parallelity. 
   1 and 2 represents a positive offset in the front sensor (front sensor is further away from the wall, turn right to fix this).
   3 and 4 represents a negative offset in the front sensor (front sensor is closer to the wall, turn left to fix this).
   The lower value represents a smaller offset. 
   Will return 0xFF if there is not a wall within one square. */
void calcParallel(){
	//Calculates parallelity for the right sensors.
	if(IRRFT == 1 && IRRBT == 1){
		double floatingIRRF = IRRF;
		double floatingIRRB = IRRB;
		double quotient = 1 - (floatingIRRF/floatingIRRB);
		uint8_t offset = 0;
		//Negative quotient means that IRRF > IRRB ((1 - >1) < 0). Sets offset to 2 so that the token values will be set to 3 and 4 instead of 1 and 2.
		if(quotient > 0){
			offset = 2;
			quotient = fabs(quotient); //Takes the absolute value of quotient to make calculations easier.
		}
		
		if(quotient < 0.1){
			parallelR = 0x00;
		}
		else if(quotient < 0.2){
			parallelR = offset + 0x01;
		}
		else if(quotient < 0.4){
			parallelR = offset + 0x02;
		}
	}
	else{
		parallelR = 0xFF;
	}
	
	//Calculates parallelity for the left sensors.
	if(IRLFT == 1 && IRLBT == 1){
		double floatingIRLF = IRLF;
		double floatingIRLB = IRLB;
		double quotient = 1 - (floatingIRLF/floatingIRLB);
		uint8_t offset = 0;
		if(quotient < 0){
			offset = 2;
			quotient = fabs(quotient);
		}
		if(quotient < 0.1){
			parallelL = 0x00;
		}
		else if(quotient < 0.2){
			parallelL = offset + 0x01;
		}
		else if(quotient < 0.4){
			parallelL = offset + 0x02;
		}
	}
	else{
		parallelL = 0xFF;
	}
}




/* These functions should get the internally stored values and transmit them, the functions sending two bytes will probably need to do some shifting. */

/* Transmits data from LIDAR. 2 bytes. */
void transmitLidar(){
	transmitByte_up(LIDAR_H);
	transmitByte_up(LIDAR_L);
}
/* Transmits data from the right front IR sendor. 1 byte. */
void transmitIRRF(){
	transmitByte_up(IRRF);
}
/* Transmits data from the right back IR sensor. 1 byte. */
void transmitIRRB(){
	transmitByte_up(IRRB);
}
/* Transmits data from the left front sensor. 1 byte. */
void transmitIRLF(){
	transmitByte_up(IRLF);	
}
/* Transmits data from the left back IR sensor. 1 byte. */
void transmitIRLB(){
	transmitByte_up(IRLB);
}
/* Transmits data from the Gyro. 2 bytes. MOST SIGNIFICANT BYTE NEED TO BE SENT FIRST. */
void transmitGyro(){
	transmitByte_up(0x21);
	transmitByte_up(0x22);
}
/* Lidar Token is the result of integer division between the Lidar value and 40. */
void transmitLidarT(){
	transmitByte_up(lidarT);
}

void transmitParallelR(){
	transmitByte_up(parallelR);
}

void transmitParallelL(){
	transmitByte_up(parallelL);
}

void transmitGyroT(){
	transmitByte_up(0xAB);
}

void transmitIRRFT(){
	transmitByte_up(IRRFT);
}

void transmitIRRBT(){
	transmitByte_up(IRRBT);
}

void transmitIRLFT(){
	transmitByte_up(IRLFT);
}

void transmitIRLBT(){
	transmitByte_up(IRLBT);
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
uint16_t read_lidar(){
	uint16_t cm;
	int prev = 0;
	uint16_t timeOfMyLife;
	int positive_edge_triggered = 0;
	
	while(1){
		
		//lidar PWM PC1 (SDA PIN 22)
		prev = current;
		current = PINC && 0x02;
		
		if (prev == 0){
			if (current == 1){
				//reset timer
				TCNT1 = 0;
				prev = 1;
				positive_edge_triggered = 1;
			}
		}
			// negative edge trigger
		else if (positive_edge_triggered == 1){
			if (prev == 1){
				if (current == 0){
					// read timer value
					uint8_t low  = TCNT1;
					uint8_t high = TCNT1H;
			
					// timeOfMyLife = high + low;
					timeOfMyLife = (high << 8) |low;
			
					// conversion to cm with number magic
					cm = timeOfMyLife *200/39;	
					prev = 0;					 
					positive_edge_triggered = 0;
					
					return cm;
				}
			}
		}
	}
}
int main()
{
	uint8_t data;
	uint16_t cm;

	timer1_init();
	adc_init();
	init_USART_up(10); 	//Baud rate is 10
	
	/*
	// adc code.
	uint8_t i = 0;
	int16_t adc_res[4] = {0,0,0,0};
	int16_t adc_median = 0;
	*/
	
	// waiting for something good to happen
	_delay_ms(500);
	
	while(1)
	{		
		/*
		//a try to calculate median, didn't work
		adc_res[i] = adc_to_cm(adc_read(0));
		
		if (i == 3){
			i = 0;
			adc_median = (adc_res[0] + adc_res[1] + adc_res[2] + adc_res[3])/(4) ;
		}
		i++;
		*/
				
		// usart
		if(checkUSARTflag_up()){
			data = receiveByte_up();
			data &= 0x3F;	//Maska ut kommandot
			processCommand(data);
		}
		
		// lidar
		cm = read_lidar();
		LIDAR_L = cm & 0xff;
		LIDAR_H = cm >> 8;		
		
		// adc
		IRLF = adc_to_cm(adc_read(0));
		IRLB = adc_to_cm(adc_read(1));
		IRRF = adc_to_cm(adc_read(2));
		IRRB = adc_to_cm(adc_read(3));
		
		calcTokensIR();
		calcParallel();
		calcLidarT();
		
		PORTD = IRRB;
	}
}