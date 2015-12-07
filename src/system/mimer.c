/*
 * Created: i den spirande vårens tid anno 1734 i väntan på vår herre jesus kristus återkomst
 * Author: Mikha'el and Eirikur aka the buzzmeister
 * "Det är lätt med facit i hand,
 *  ändå var det folk som failade på logiktentan" - Peter
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
uint8_t IRF = 0x05;

uint8_t IRLFT = 0x06;
uint8_t IRLBT = 0x07;
uint8_t IRRFT = 0x08;
uint8_t IRRBT = 0x09;
uint8_t IRFT = 0x0a;

int8_t parallelL = 0x0b;
int8_t parallelR = 0x0c;

/*reflex-sensor*/
uint8_t reflex_previous = 0x00;
uint8_t reflex_current = 0x00;
uint8_t segments_turned = 0x00;
uint8_t MR_Reflex = 0x00;

/*GYRO*/
uint32_t rotation_constant;
int gyromode = 0;
uint8_t gyro_zero = 0x00;
int rotation_direction = 0;
uint8_t gyro_token = 0x00;

uint8_t usart_data;

uint8_t spi_tranceiver(uint8_t data);
uint8_t single_measure();
void usart_gogo();

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

/* slave select - for SPI */
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
	// enable adc conversion
	set_ss(0);
	spi_tranceiver(0x94);
	spi_tranceiver(0x00);
	spi_tranceiver(0x00);
	set_ss(1);
}

/*to transfer/recieve data via SPI */
uint8_t spi_tranceiver(uint8_t data)
{
	// Load data into the buffer
	SPDR = data;
	
	//Wait until transmission complete
	while(!(SPSR & (1<<SPIF) ));
	
	// Return received data
	return(SPDR);
}

/* read adc value */
int8_t adc_read(int16_t ch)
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
	
	// return the 8 msb bits
	return (ADCH);
}

/*look up corresponding value for ir-sensor*/
uint8_t adc_to_cm(uint8_t value){
	// for short distances
	if (value >= 200)
		return 1;
	return cm_values[value];
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
/* Transmits data from the front IR sensor. 1 byte of pure love. */
void transmitIRF(){
	transmitByte_up(IRF);
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
void transmitIRFT(){
	transmitByte_up(IRFT);
}
void transmitReflexT(){
	transmitByte_up(segments_turned);
}

/* Transmits all sensor data. */
/* Transmitted data order will be IRRF -> IRRB -> IRLF -> IRLB -> IRF -> Parallel Right -> Parallel Left -> Gyro token -> IRRF token -> IRRB token -> IRLF token -> IRLB token -> IRF token -> reflex token. */
void transmitALL(){
	transmitIRRF();
	transmitIRRB();
	transmitIRLF();
	transmitIRLB();
	transmitIRF();

	transmitParallelR();
	transmitParallelL();
	transmitGyroT();
	
	transmitIRRFT();
	transmitIRRBT();
	transmitIRLFT();
	transmitIRLBT();
	transmitIRFT();
	
	transmitReflexT();
}

/* Translates the command code into the corresponding function. */
void processCommand(unsigned char data){
	if(data == 0x08){
		transmitIRF();
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
	else if(data == 0x0F){
		transmitIRFT();
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
		rotation_direction = 0; // clockwise
	}
	else if(data == 0x1f){
		gyromode = 1;
		rotation_direction = 1; // counterclockwise
	}
	else if(data == 0x20){
		gyromode = 1;
		rotation_direction = 2; // 180°
	}
	else if(data == 0x1e){ // reset gyro
		gyromode = 0;
		gyro_token = 0x00;
	}
	else if(data == 0x21){ // reset reflex counter
		segments_turned = 0;
	}
}

/* Calculates the token value given pointers to the stored values. */
void calcTokenIR(uint8_t *IRxx, uint8_t *IRxxT){
	if(1 < *IRxx && *IRxx <= 32){
		*IRxxT = 1;
	}
	else if(32 < *IRxx && *IRxx < 60){
		*IRxxT = 2;
	}
	else{
		*IRxxT = 0;
	}
}

/* Calculate the token value for the front IR sensor */
void calcIRFToken(){
	if(IRF <= 15){
		IRFT = 0;
	}
	else if(IRF <= 47){
		IRFT = 1;
	}
	else{
		IRFT = 2;
	}
}

/*	Calculates the token values for the IR sensors */
void calcTokensIR(){
	calcTokenIR(&IRLB, &IRLBT);
	calcTokenIR(&IRLF, &IRLFT);
	calcTokenIR(&IRRB, &IRRBT);
	calcTokenIR(&IRRF, &IRRFT);
}

/* parallelism for the IR sensors */
void calcParallel(){
	if (IRRF == 0 || IRRB == 0){
		parallelR = 127;
	}
	else{
		parallelR = IRRF - IRRB;
	}
	parallelL = IRLF - IRLB;
}

/*calculate middle point of gyro, called gyro_zero. uses 16 values. is called in the beginning of the program.*/
void calibrate_gyro(){
	int i = 0;
	uint8_t single_value_gyro = 0x00;
	uint16_t gyro_zero_sum = 0;
	
	while(i < 16){ //measure 16 times
		single_value_gyro = single_measure();
		gyro_zero_sum += single_value_gyro;
		i++;
	}
	
	gyro_zero_sum = gyro_zero_sum/16; // divides with 'magic' constant (4!-3!-(2!)^2+1!+0!)
	gyro_zero = gyro_zero_sum;
}

/* Single measure from the GYRO */
uint8_t single_measure(){ 
	
	uint8_t res_adc1;
	uint8_t res_adc2;
	uint16_t angular_rate_temp = 0x0000;
	uint8_t angular_rate = 0x00;
	int EOC = 0;
	
	// start command
	set_ss(0);
	spi_tranceiver(0x94);
	spi_tranceiver(0x00);
	spi_tranceiver(0x00);
	set_ss(1);
	
	// wait until EOC is set ( end of conversion )
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
	
	angular_rate_temp = (res_adc1 << 8) | res_adc2;			// angular_rate = res_adc1 & res_adc2
	angular_rate_temp >>= 4;								// shift four bits right
	angular_rate_temp &= 0x00ff;							// stores the MSB 8 bits	
	angular_rate = angular_rate_temp;
	
	return angular_rate; 
}

/* to rotate either 90° clockwise/counterclockwise or 180°  */
void gyro_gogo(){
	uint8_t angular_rate = 0x00;
	uint32_t angular_sum = 0;
	
	if (rotation_direction == 2){ // 180 degrees
		rotation_constant = 124736;
	}
	else if(rotation_direction == 1){ // counter-clockwise
		rotation_constant = 61002;
	}
	else if(rotation_direction == 0){  // clockwise
		rotation_constant = 58000;
	}
	
	while(gyromode == 1){	// until rotation complete, spin
		angular_rate = single_measure();
			
		if (angular_rate > (gyro_zero+10)){
			angular_rate -= gyro_zero;
		}
		else if(angular_rate < (gyro_zero-10)){
			angular_rate = gyro_zero - angular_rate;
		}
		else{ 
			angular_rate = 0;
		}
		angular_sum += angular_rate;
		
		if (angular_sum > rotation_constant){
			gyromode = 0;
			gyro_token = 0x44;
		}
		
		usart_gogo();
	}
}

/* Counts # of segments turned */
void reflex_sensor(){
	if(MR_Reflex > 160){
		reflex_current = 0x00;
	}
	else if(MR_Reflex < 60){
		reflex_current = 0x01;
	}
	if (reflex_current != reflex_previous){
		segments_turned++;		
	}
	reflex_previous = reflex_current;
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
	adc_init();
	init_USART_up(10); 	//Baud rate is 10
	
	SPI_init();
	_delay_ms(500);
	calibrate_gyro();
	
	// waiting for something good to happen
	while(1){				
		// usart
		usart_gogo();
		
		// ad conversion from IR-sensors
		IRLB = adc_to_cm(adc_read(0));
		IRRB = adc_to_cm(adc_read(1));
		IRLF = adc_to_cm(adc_read(2));
		IRRF = adc_to_cm(adc_read(3));
		IRF = adc_to_cm(adc_read(4));
		
		usart_gogo();
		
		MR_Reflex = adc_read(5);
		reflex_sensor();
		
 		calcTokensIR();
		calcIRFToken();	
 		calcParallel();
		 
		usart_gogo();

		if(gyromode == 1){
			gyro_gogo();
		}
	}
}