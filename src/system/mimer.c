/*
 * adc.c
 *
 * Created: i den spirande vårens tid anno 1734 i väntan på vår herre jesus kristus återkomst
 * Author: Mikha'el and Eirikur
 * Det är lätt med facit i hand 
 * ändå var det folk som failade på logiktentan
 */
#ifndef F_CPU
#define F_CPU 20000000UL
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

int8_t parallelL = 0xa0;
int8_t parallelR = 0xb0;

int8_t MR_Reflex = 0xBF;

/*LIDAR*/
uint8_t LIDAR_H = 0xa0;
uint8_t LIDAR_L = 0xa1;
uint8_t lidarT = 0x45;
int current = 0;

/*GYRO*/
uint8_t gyro1 = 0xb0;
uint8_t gyro2 = 0xb1;
//uint8_t gyro3 = 0xb2;
//uint8_t gyro4 = 0xb3;

uint16_t rotation_constant_90_degrees = 380;
int gyromode = 0;
int gyro_direction = 0;
uint16_t gyro_zero = 0;

int gyro_token = 0;

uint8_t usart_data;

int gyrotime = 0;
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
	if (mode == 0){
		PORTB &= (mode<<PORTB4);
	}
	else{
		PORTB |= (mode<<PORTB4);
	}
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
	//transmitByte_up(IRRF);
	transmitByte_up(MR_Reflex);
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
	transmitByte_up(gyro_token);
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
	
	else if(data == 0x1c){
		gyromode = 1;
		gyro_direction = 1; // 1 = clockwise
	}
	else if(data == 0x1f){
		gyromode = 1;
		gyro_direction = 0; // 0 = counterclockwise
	}
	else if(data == 0x1e){
		gyromode = 0;
		gyro_token =0;
	}
	/*
	else if(data == 0xGYRO_reset){
		gyromode = 0;
		gyro_token = 0;
	}
	*/
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
void calcParallel_old(){
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
        if(quotient < 0){
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

void calcParallel(){
	if (IRRF == 0 || IRRB == 0){
		parallelR = 127;
		parallelL = IRLF - IRLB;
	}
	else{
		parallelR = IRRF - IRRB;
		parallelL = IRLF - IRLB;	
	}
	
}


uint16_t read_lidar(){
	//uint16_t cm;
	int prev = 0;
	//int timeOfMyLife;
	int positive_edge_triggered = 0;
	int j =0;
	double timeOfMyLife;
	int low,high;
	int cm;
	int cm_sum=0;
	while(1){
		if (j == 7){
			j= 0;
			return (cm_sum/8);
		}
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
					
					if (cm > 125){// faster at high distances
						return cm;
					}
					
						
					prev = 0;					 
					positive_edge_triggered = 0;
					cm_sum += cm;
					j++;
				}
			}
		}
	}
}

/*calculate middle point of gyro, called zero. uses 100 values*/
void calibrate_gyro(){
		uint8_t res_adc1; 
		uint8_t res_adc2;
		int i = 0;
		uint8_t single_value_gyro = 0;
		uint16_t gyro_zero_sum = 0;
		// enable adc conversion
		set_ss(0);
		spi_tranceiver(0x94);
		res_adc1 = spi_tranceiver(0x00);
		spi_tranceiver(0x00);
		set_ss(1);
		
		_delay_us(115);
		
		while(i < 16){
			
			set_ss(0);
			spi_tranceiver(0x94);
			res_adc1 = spi_tranceiver(0x00);
			spi_tranceiver(0x00);
			set_ss(1);
		
			_delay_us(115);

			set_ss(0);
			spi_tranceiver(0x80);
			res_adc1 = spi_tranceiver(0x00);
			res_adc2 = spi_tranceiver(0x00);
			set_ss(1);
			
			// stores the 8MSB bits in single_value_gyro		
			single_value_gyro = (res_adc1 << 8) | res_adc2; // angular_rate = res_adc1 & res_adc2
			single_value_gyro >>= 4; // shift four bits right
			single_value_gyro &= 0x00ff; // stores the MSB 8 bits	
			
			gyro_zero += single_value_gyro;
	
			i++;
			
		}
		
		gyro_zero = gyro_zero/16; // calculate mean
		
}
uint8_t single_measure(){
	
	uint8_t res_adc1;
	uint8_t res_adc2;
	uint8_t angular_rate = 0x00;
	int EOC = 0;
	
	// start command
	set_ss(0);
	spi_tranceiver(0x94);
	res_adc1 = spi_tranceiver(0x00);
	spi_tranceiver(0x00);
	set_ss(1);
	
	//_delay_us(115);
	
	while(!EOC){
		set_ss(0);
		spi_tranceiver(0x80);
		res_adc1 = spi_tranceiver(0x00);
		res_adc2 = spi_tranceiver(0x00);
		set_ss(1);
		if((res_adc1 & 0x20) == 0x20){ 
			EOC = 1;
		}
	}
	
	angular_rate = (res_adc1 << 8) | res_adc2; // angular_rate = res_adc1 & res_adc2
	angular_rate >>= 4; // shift four bits right
	angular_rate &= 0x00ff; // stores the MSB 8 bits	

	return angular_rate; 
}
void startMeasure(){
// 	unsigned char data;            //Received data stored here
// 		uint8_t res_adc1;
// 		uint8_t res_adc2;
// 		uint8_t angular_rate;
// 		
// 		// step1 put adc to the active mode if it wasn't
// 		set_ss(0);
// 		spi_tranceiver(0x94);
// 		res_adc1 = spi_tranceiver(0x00);
// 		spi_tranceiver(0x00);
// 	
// 		set_ss(1);
// 		_delay_us(115);
// 		
// 		// step 2 start conversion
// 		set_ss(0);
// 		spi_tranceiver(0x94);
// 		res_adc1 = spi_tranceiver(0x00);
// 		spi_tranceiver(0x00);
// 	
// 		set_ss(1);
// 		
// 		_delay_us(115);
// 	
// 		set_ss(0);
// 		
// 		// step 3 result obtaining
// 		spi_tranceiver(0x80);
// 		res_adc1 = spi_tranceiver(0x00);
// 		res_adc2 = spi_tranceiver(0x00);
// 	
// 		set_ss(1);
// 		gyro_zero = ((res_adc1 & 0x0f) << 4) | ((res_adc2 & 0xf0) >> 4); // b5 = 0
// 		
// 		while (1){
// 			
// 			// step 2 start conversion
// 			set_ss(0);
// 			spi_tranceiver(0x94);
// 			res_adc1 = spi_tranceiver(0x00);
// 			spi_tranceiver(0x00);
// 			
// 			set_ss(1);
// 			
// 			_delay_us(115);
// 			set_ss(0);
// 			
// 			// step 3 result obtaining
// 			spi_tranceiver(0x80);
// 			res_adc1 = spi_tranceiver(0x00);
// 			res_adc2 = spi_tranceiver(0x00);
// 			set_ss(1);
// 			
// 			angular_rate = ((res_adc1 & 0x0f) << 4) | ((res_adc2 & 0xf0) >> 4); // b5 = 0
// 			angular_sum = angular_sum + angular_rate-gyro_zero;
// 			
// 			_delay_ms(100);
// 			
// 			//gyro4 = (angular_sum & 0xf000) >> 24;
// 			//gyro3 = (angular_sum & 0x0f00) >> 16;
// 			gyro2 = (angular_sum & 0x00f0) >> 8;
// 			gyro1 = (angular_sum & 0x000f);
// 		}
}

void gyro_gogo(){
	
	uint8_t angular_rate = 0x00;
	uint16_t angular_sum = 0;
	int angular_rate_n = 0;
	
	//calibrate_gyro();
	
	
	transmitByte_up(0x44);

	while(1){	
		usart_gogo();
		_delay_ms(10);
		//if(gyro_direction == 1){ // clockwise
			
			angular_rate = single_measure();
			if ((angular_rate > (gyro_zero + 0x05)) || (angular_rate < (gyro_zero - 0x05))){
				
			
			
			
				if (angular_rate > gyro_zero){
				angular_rate -= gyro_zero;
				
				}
				else{
					angular_rate = gyro_zero - angular_rate;
				}
			//angular_rate_n = angular_rate_n - gyro_zero ; //actual difference
			
				angular_sum += angular_rate;
			}
		//}
// 		else if(gyro_direction == 0){ // counterclockwise
// 			
// 			angular_rate = single_measure();
// 			if (a)
// 			angular_rate_n = angular_rate;
// 			angular_rate_n += gyro_zero; //actual difference
// 			angular_sum += angular_rate_n;
// 		}
		if (angular_sum > rotation_constant_90_degrees){
			break;
		}
	}
	
	gyromode = 0;
	gyro_token = 1; // how to reset?
}


void reflex_sensor(){
//	current = adc_read(5);
//	if (current != previous){
//		segments_turned++;
//	}
//	previous = current;
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
	uint16_t lidar_cm;
	int gyro_sum;
	int clk = 0;

	timer1_init();
	adc_init();
	init_USART_up(10); 	//Baud rate is 10
	
	SPI_init();
	_delay_ms(500);
	calibrate_gyro();
	
	// waiting for something good to happen
	while(1){
		single_measure();
	}
	/*
	while(1){				
		// usart
		usart_gogo();
			
		// lidar
		lidar_cm = read_lidar();
		if(lidar_cm < 10){
			lidar_cm -= 1;
		}
		else if(lidar_cm < 15){
			lidar_cm -= 3;
		}
		else if(lidar_cm < 20){
			lidar_cm -= 1;
		}
		else if(lidar_cm < 25){
			lidar_cm -= 2;
		}
		LIDAR_L = lidar_cm & 0xff;
		LIDAR_H = lidar_cm >> 8;	
		calcLidarT();	
		
		usart_gogo();
		
		// adc	
		IRLB = adc_to_cm(adc_read(0));
		IRRB = adc_to_cm(adc_read(1));
		IRLF = adc_to_cm(adc_read(2));
		IRRF = adc_to_cm(adc_read(3));
		
		MR_Reflex = adc_read(5);
		
 		calcTokensIR();
 		calcParallel();
		usart_gogo();
		
		//gyrotime++;
		//if (gyrotime == 100){
			//int gyro_sum = single_measure();
			gyro1 = gyro_zero; //(gyro_sum & 0x0f);
			gyro2 = 0; //(gyro_sum & 0xf0)  >> 8;
			//gyrotime = 0;
		
		//}
		
		// not needed 
		if(gyromode == 1){
			gyro_gogo();
			// reset gyroToken when processed
		}
		*/
		
	
	//}
}