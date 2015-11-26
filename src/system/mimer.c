/*
 * adc.c
 *
 * Created: i den spirande vårens tid anno 1734 i väntan på vår herre jesus kristus återkomst
 *  Author: Mikha'el and Eirikur
 */
#ifndef F_CPU
#define F_CPU 20000000
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include "lib\usart.h"
#include <avr/interrupt.h>

uint8_t cm_values[200] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 98, 91, 86, 81, 76, 72, 69, 65, 62, 59, 57, 55, 52, 50, 48, 47, 45, 43, 42, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 29, 28, 27, 27, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 21, 21, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

/*for ir-sensor, normal and token values*/	
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

/*lidar, normal and token values*/
uint8_t LIDAR_H = 0xa0;
uint8_t LIDAR_L = 0xa1;

uint8_t lidarT = 0x45;

/*gyro, normal and token values*/
uint8_t gyro1 = 0xb0;
uint8_t gyro2 = 0xb1;
//uint8_t gyro3 = 0xb2;
//uint8_t gyro4 = 0xb3;

uint8_t angular_rate = 0x00;
int angular_sum = 0;

uint8_t zero = 0;

// current PWM-value
int current = 0;

uint8_t usart_data;

/* initializes timer for pwm reading */
void timer1_init(){
	// reset registers and set prescaler to 1024
	TCCR1A = 0;
	TCCR1B = 0;
	
	//TCCR1B |= (1<<CS10)|(0<<CS11)|(1<<CS12);
	
	// prescaler 8 i
	TCCR1B |= (0<<CS10)|(1<<CS11)|(0<<CS12);
	
	// reset timer
	TCNT1 = 0;
}
/* initializes ad converter */
void adc_init()
{
	// AREF = AVcc
	// ADLAR = 1 gives left adjusted data (8 MSB values in ADCH)
	ADMUX = (1<<REFS0)|(1<<ADLAR);
	
	// ADC Enable and prescaler of 128
	// 20 000 000/128 = 156 250 Hz (50 kHz < 156 kHz < 200 kHz)
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

void set_ss(int mode){
	PORTB &= (mode<<PORTB4);
}

/*SPI initialization*/
void SPI_init()
{
	DDRB=(1<<PCINT15)|(1<<PCINT13)|(1<<PORTB4); // set SCK, MOSI and SS as output
	SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR0); // Enable SPI | set as master | SCK freq = mclk/8
	
	set_ss(1);
}

/*to transfer/recieve data via spi*/
unsigned char spi_tranceiver (uint8_t data)
{
	// Load data into the buffer
	SPDR = data;
	
	//Wait until transmission complete
	while(!(SPSR & (1<<SPIF) ));
	
	// Return received data
	return(SPDR);
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

/*look up corresponding value for ir-sensor*/
int16_t adc_to_cm(int16_t value){
	if (value > 200)
		return 1;
	return cm_values[value];
}

/* These functions should get the internally stored values and transmit them, the functions sending two bytes will probably need to do some shifting. */

/* Transmits data from LIDAR. 2 bytes. MOST SIGNIFICANT BYTE NEED TO BE SENT FIRST. */
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
//	transmitByte_up(gyro4);
//	transmitByte_up(gyro3);
	transmitByte_up(gyro2);
	transmitByte_up(gyro1);
}

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

/*counts multiples of 40cm. example: 60cm = 1, 80cm = 2 ... */
void calcLidarT(){
	uint16_t lidarValue = LIDAR_H*256 + LIDAR_L;
	//lidarT = lidarValue / 40;
	uint8_t x = 0;
	while (x < 20){
		if (lidarValue >= 40){
			x++;
			lidarValue -= 40;
		}
		else{
			break;
		}
	}
	lidarT = x;
}

/*	Calculates the token values for the IR sensors. 
	1 represents a value between 1 and 32 (a wall within 1 square), 
	2 represents a value between 32 and 89 ( a wall within 2 squares). */
void calcTokensIR(){
	calcTokenIR(&IRLB, &IRLBT);
	calcTokenIR(&IRLF, &IRLFT);
	calcTokenIR(&IRRB, &IRRBT);
	calcTokenIR(&IRRF, &IRRFT);
}


void calcGyroT(){
	
	
	//gyro4 = (angular_sum & 0xf000) >> 24;
	//gyro3 = (angular_sum & 0x0f00) >> 16;
	//gyro2 = (angular_sum & 0x00f0) >> 8;
	//gyro1 = (angular_sum & 0x000f);
}

/*  Calculates the token values for parallelism by taking the quotient of the front and back sensors. 
	0 represents parallelism. 
    1 and 2 represents a positive offset in the front sensor (front sensor is further away from the wall, turn right to fix this).
    3 and 4 represents a negative offset in the front sensor (front sensor is closer to the wall, turn left to fix this).
    The lower value represents a smaller offset. 
    Will return 0xFF if there is not a wall within one square. */
void calcParallel(){
	double floatingIRF;
	double floatingIRB;
	double quotient;
	uint8_t offset;
    
	//Calculates parallelism for the right sensors.
	if(IRRFT == 1 && IRRBT == 1){
		floatingIRF = IRRF;
        floatingIRB = IRRB;
        quotient = 1 - (floatingIRF/floatingIRB);
        offset = 0;
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
        
	//Calculates parallelism for the left sensors.
	if(IRLFT == 1 && IRLBT == 1){
		floatingIRF = IRLF;
		floatingIRB = IRLB;
		quotient = 1 - (floatingIRF/floatingIRB);
		offset = 0;
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


uint16_t read_lidar(){
	//uint16_t cm;
	int prev = 0;
	//int timeOfMyLife;
	int positive_edge_triggered = 0;
	
	double timeOfMyLife;
	int low,high;
	int cm;
	
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
					low  = TCNT1;
					high = TCNT1H;
			
					// timeOfMyLife = high + low;
					timeOfMyLife = (high << 8) |low;
			
					// conversion to cm with number magic
					//cm = timeOfMyLife*200/39;	
					cm = (int)timeOfMyLife/25;	
					prev = 0;					 
					positive_edge_triggered = 0;
					
					return cm;
				}
			}
		}
	}
}

/*calculate middle point of gyro, called zero*/
void calibrate_gyro(){
		unsigned char data;                 //Received data stored here
		uint8_t res_adc1;
		uint8_t res_adc2;
		uint8_t angular_rate;

		
		// step1 put adc to the active mode if it wasn't
		set_ss(0);
		spi_tranceiver(0x94);
		res_adc1 = spi_tranceiver(0x00);
		spi_tranceiver(0x00);

		set_ss(1);
		_delay_us(115);
		
		// step 2 start conversion
		set_ss(0);
		spi_tranceiver(0x94);
		res_adc1 = spi_tranceiver(0x00);
		spi_tranceiver(0x00);

		set_ss(1);
		
		_delay_us(115);

		set_ss(0);
		
		// step 3 result obtaining
		spi_tranceiver(0x80);
		res_adc1 = spi_tranceiver(0x00);
		res_adc2 = spi_tranceiver(0x00);

		set_ss(1);
		zero = ((res_adc1 & 0x0f) << 4) | ((res_adc2 & 0xf0) >> 4); // b5 = 0

}
int single_measure(){
	uint8_t res_adc1;
	uint8_t res_adc2;
	set_ss(0);
	spi_tranceiver(0x94);
	res_adc1 = spi_tranceiver(0x00);
	spi_tranceiver(0x00);
	set_ss(1);
	_delay_us(115);
	set_ss(0);
	
	// step 3 result obtaining
	spi_tranceiver(0x80);
	res_adc1 = spi_tranceiver(0x00);
	res_adc2 = spi_tranceiver(0x00);
	set_ss(1);
	
	angular_rate = ((res_adc1 & 0x0f) << 4) | ((res_adc2 & 0xf0) >> 4); // b5 = 0
	angular_sum = angular_sum + angular_rate-zero;
	
	//gyro4 = (angular_sum & 0xf000) >> 24;
	//gyro3 = (angular_sum & 0x0f00) >> 16;
	return angular_sum;
}
void startMeasure(){
	unsigned char data;                 //Received data stored here
	uint8_t res_adc1;
	uint8_t res_adc2;
	uint8_t angular_rate;
	
	
	// step1 put adc to the active mode if it wasn't
	set_ss(0);
	spi_tranceiver(0x94);
	res_adc1 = spi_tranceiver(0x00);
	spi_tranceiver(0x00);

	set_ss(1);
	_delay_us(115);
	
	// step 2 start conversion
	set_ss(0);
	spi_tranceiver(0x94);
	res_adc1 = spi_tranceiver(0x00);
	spi_tranceiver(0x00);

	set_ss(1);
	
	_delay_us(115);

	set_ss(0);
	
	// step 3 result obtaining
	spi_tranceiver(0x80);
	res_adc1 = spi_tranceiver(0x00);
	res_adc2 = spi_tranceiver(0x00);

	set_ss(1);
	zero = ((res_adc1 & 0x0f) << 4) | ((res_adc2 & 0xf0) >> 4); // b5 = 0
	
	while (1){
		
		// step 2 start conversion
		set_ss(0);
		spi_tranceiver(0x94);
		res_adc1 = spi_tranceiver(0x00);
		spi_tranceiver(0x00);
		
		set_ss(1);
		
		_delay_us(115);
		set_ss(0);
		
		// step 3 result obtaining
		spi_tranceiver(0x80);
		res_adc1 = spi_tranceiver(0x00);
		res_adc2 = spi_tranceiver(0x00);
		set_ss(1);
		
		angular_rate = ((res_adc1 & 0x0f) << 4) | ((res_adc2 & 0xf0) >> 4); // b5 = 0
		angular_sum = angular_sum + angular_rate-zero;
		
		_delay_ms(100);
		
		//gyro4 = (angular_sum & 0xf000) >> 24;
		//gyro3 = (angular_sum & 0x0f00) >> 16;
		gyro2 = (angular_sum & 0x00f0) >> 8;
		gyro1 = (angular_sum & 0x000f);
	}
}

/*usart code*/
void usart_gogo(){
	if(checkUSARTflag_up()){
		usart_data = receiveByte_up();
		usart_data &= 0x3F;	//Maska ut kommandot 
		processCommand(usart_data);
	}
}

int main()
{
	uint16_t cm;
	int gyro_sum;
	int clk = 0;

	timer1_init();
	adc_init();
	init_USART_up(10); 	//Baud rate is 10
	
	SPI_init();
	calibrate_gyro();
	
	// Det är lätt med facit i hand//
	// ändå var det folk som failade på logiktentan//
	
	// waiting for something good to happen
	while(1){		
		
	/*	if(clk){
			clk = 0;
			PORTA &= 0xf0;
		}
		else{
			clk = 1;
			PORTA |= (1 << PORTA5);
		}
		
		*/		

		// usart
		usart_gogo();
			
		// lidar
		cm = read_lidar();
		LIDAR_L = cm & 0xff;
		LIDAR_H = cm >> 8;	
		calcLidarT();	
		
		usart_gogo();
		
		// adc	
		IRRF = adc_to_cm(adc_read(0));
		IRLF = adc_to_cm(adc_read(1));
		IRRB = adc_to_cm(adc_read(2));
		IRLB = adc_to_cm(adc_read(3));
		
		usart_gogo();
		
		int gyro_sum = single_measure();
		gyro2 = (gyro_sum & 0x00f0) >> 8;
		gyro1 = (gyro_sum & 0x000f);
		//startMeasure();
		
		calcTokensIR();
		calcParallel();
	
	}
}