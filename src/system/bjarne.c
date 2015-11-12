#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 20000000
#include <util/delay.h>
#include "lib/usart.h"

#define FORWARD 1
#define BACK 0
#define false 0
#define true 1
#define bool uint8_t

void setSpeed(uint8_t lspeed, uint8_t rspeed, uint8_t ldir, uint8_t rdir);
void init_motors();
void handle_messages();
uint8_t decide_if_repeated(uint8_t msg);
void update_sensor_data(); 

uint8_t dir_left = 1; //0 or 1 = back or forward
uint8_t dir_right = 1;
uint8_t spd_left = 0; //0 to 100 = procent of full speed
uint8_t spd_right = 0;
uint8_t button_autonom = 0; // 0 om manuellt läge, 1 om autonomt läge

uint8_t activeDirs = 0;
bool new_message = false;



//-----SENSOR DB------------

//	sensor/token_sensor_fram/bak/vänster/höger upper/lower

uint8_t s_LIDAR_u = 0;
uint8_t s_LIDAR_l = 0;
uint8_t t_LIDAR = 0;

uint8_t s_ir_h_f = 0;
uint8_t s_ir_h_b = 0;
uint8_t s_ir_v_f = 0;
uint8_t s_ir_v_b = 0;

//	Token parallell vänster/höger

uint8_t t_p_h = 0;
uint8_t t_p_v = 0;

uint8_t t_vagg_h_f = 0;
uint8_t t_vagg_h_b = 0;
uint8_t t_vagg_v_f = 0;
uint8_t t_vagg_v_b = 0;

uint8_t s_gyro_u = 0;
uint8_t s_gyro_l = 0;
uint8_t t_gyro = 0;

uint8_t s_reflex = 0;
uint8_t t_reflex_u = 0;
uint8_t t_reflex_l = 0;
//----------------------------




//------KARTA DB--------------
uint8_t kartdata_x = 0;
uint8_t kartdata_y = 0;
uint8_t styrdata = 0;
uint8_t posdata_x = 0;
uint8_t posdata_y = 0;
uint8_t posalgoritm = 0;
uint8_t kartdata_temp_x = 0;
uint8_t kartdata_temp_y = 0;

//----------------------------

int main(){
	
	init_USART_up(10);
	init_USART_down(10);
	init_motors();
	
	while(1){
		
		handle_messages();
		//update_sensor_data();

		if (button_autonom == 1){
			
			//tolka sensordata
			//reagera på sensordata
			
		}
		
		
	
	
		//setSpeed(50,100,FORWARD,BACK);
	}
}

uint8_t decide_if_repeated(uint8_t msg){
	
	bool answer = false;
	switch(msg){
		case (0): //pil UPP trycks ner	
			answer = activeDirs	&& 1; // 1 om kommandot är repeatat annars 0
			activeDirs = activeDirs || 1;
		break;
		case(1): //pil UPP släpps		
			answer = false;
			activeDirs = activeDirs && 254;
		break;
		case(2): //pil VÄNSTER trycks ner
			answer = activeDirs	&& 4; // 1 om kommandot är repeatat annars 0
			activeDirs = activeDirs || 4;
		break;
		case(3): //pil VÄNSTER släpps
			answer = false;
			activeDirs = activeDirs && 251;
		break;
		case (4): //pil NER trycks ner	
			answer = activeDirs	&& 2; // 1 om kommandot är repeatat annars 0
			activeDirs = activeDirs || 2;
		break;
		case(5): //pil NER släpps
			answer = false;
			activeDirs = activeDirs && 253;
		break;
		case(6): //pil HÖGER trycks ner
			answer = activeDirs	&& 8; // 1 om kommandot är repeatat annars 0
			activeDirs = activeDirs || 8;
		break;
		case(7): //pil HÖGER släpps
			answer = false;
			activeDirs = activeDirs && 247;
		break;				
	}	

	return answer;
}

void handle_messages(){
	if(checkUSARTflag()){
	
		uint8_t message = receiveByte_up();	
		uint8_t message_cpy = message >> 1;
		PORTA = message_cpy;
		//plocka ut OP-koden
		message_cpy &= 31;
		PORTA = activeDirs;
		//uint8_t repeated = decide_if_repeated(message_cpy);
		uint8_t repeated = false;
		
		if (button_autonom == 0){ //Manuellt läge
			if (!repeated){
				switch(message_cpy){
					case (0): //pil UPP trycks ner

					dir_left = FORWARD;
					dir_right = FORWARD;
					spd_left = spd_left + 50;
					spd_right = spd_right + 50;
					break;
					case(1): //pil UPP släpps
					spd_left = spd_left - 50;
					spd_right = spd_right - 50;
					break;
					case(2): //pil VÄNSTER trycks ner
					spd_right = spd_right + 50;
					break;
					case(3): //pil VÄNSTER släpps
					spd_right = spd_right - 50;
					break;
					case (4): //pil NER trycks ner
					dir_left = BACK;
					dir_right = BACK;
					spd_left = spd_left + 50;
					spd_right = spd_right + 50;
					break;
					case(5): //pil NER släpps
					spd_left = spd_left - 50;
					spd_right = spd_right - 50;
					dir_left = FORWARD;
					dir_right = FORWARD;

					break;
					case(6): //pil HÖGER trycks ner
					spd_left = spd_left + 50;
					break;
					case(7): //pil HÖGER släpps
					spd_left = spd_left - 50;
					break;			
				}
			}
			setSpeed(spd_left,spd_right,dir_left,dir_right);
		}

		/*switch(message_cpy){
			
			case (0x08):
				//lägg lidardata i send-buffern
				transmitByte_up(s_LIDAR_u);
				transmitByte_up(s_LIDAR_l);

			break;
			
			case (0x09):
			//lägg sensordata IR höger fram-data i send-buffern
				transmitByte_up(s_ir_h_f);

			break;
			
			case (0x0A):
			//lägg sensordata IR höger bak-data i send-buffern
				transmitByte_up(s_ir_h_b);
			break;
			
			case (0x0B):
			//lägg sensordata IR vänster fram-data i send-buffern
				transmitByte_up(s_ir_v_f);
			break;
			
			case (0x0C):
			//lägg sensordata IR vänster bak-data i send-buffern
				transmitByte_up(s_ir_v_b);
			break;
			
			case (0x0D):
			//lägg gyro-data i send-buffern
				transmitByte_up(s_gyro_u);
				transmitByte_up(s_gyro_l);
			break;
			
			case (0x0E):
			//lägg reflexsensor-data i send-buffern
				transmitByte_up(s_reflex);
			break;
			
			case (0x0F):
			//lägg lidar-token i send-buffern
				transmitByte_up(t_LIDAR);
			break;
			
			case (0x10):
			//lägg parallell höger-token i send-buffern
				transmitByte_up(t_p_h);
			break;
			
			case (0x11):
			//lägg parallell vänster-token i send-buffern
				transmitByte_up(t_p_v);
			break;
			
			case (0x12):
			//lägg gyro-token i send-buffern
				transmitByte_up(t_gyro);
			break;
			
			case (0x13):
			//lägg vägg höger fram-token i send-buffern
				transmitByte_up(t_vagg_h_f);
			break;
			
			case (0x14):
			//lägg vägg höger bak-token i send-buffern
				transmitByte_up(t_vagg_h_b);
			break;
			
			case (0x15):
			//lägg vägg vänster fram-token i send-buffern
				transmitByte_up(t_vagg_v_f);
			break;
			
			case (0x16):
			//lägg vägg vänster bak-token i send-buffern
				transmitByte_up(t_vagg_v_b);
			break;
			
			case (0x17):
			//lägg reflex-token i send-buffern
				transmitByte_up(t_reflex_u);
				transmitByte_up(t_reflex_l);
			break;
			
			case (0x18):
			//lägg kartdata i send-buffern
				transmitByte_up(kartdata_x);
				transmitByte_up(kartdata_y);
			break;
			
			case (0x19):
			//senaste styrbeslut i send-buffern
				transmitByte_up(styrdata);
			break;

			case (0x1A):
			//pos karta X och Y i send-buffern
				transmitByte_up(posdata_x);
				transmitByte_up(posdata_y);
			break;
			
			case (0x1B):
			//pos i algoritm i send-buffern
				transmitByte_up(posalgoritm);
			break;
			
			case (0x1C):
			//lägg tempKartdata i send-buffern
				transmitByte_up(kartdata_temp_x);
				transmitByte_up(kartdata_temp_y);
			break;

		}*/
	}		
}

void update_sensor_data(){
	//från 08 ---> 1 7 rader
	transmitByte_down(0x1D); //fråg efter all data

	s_LIDAR_u = receiveByte_down();
	s_LIDAR_l = receiveByte_down();
	s_ir_h_f = receiveByte_down();
	s_ir_h_b = receiveByte_down();
	s_ir_v_f = receiveByte_down();
	s_ir_v_b = receiveByte_down();
	s_gyro_u = receiveByte_down();
	s_gyro_l = receiveByte_down();
	s_reflex = receiveByte_down();


	t_LIDAR = receiveByte_down();
	t_p_h = receiveByte_down();;
	t_p_v = receiveByte_down();
	t_gyro = receiveByte_down();
	t_vagg_h_f = receiveByte_down();
	t_vagg_h_b = receiveByte_down();
	t_vagg_v_f = receiveByte_down();
	t_vagg_v_b = receiveByte_down();
	t_reflex_u = receiveByte_down();
	t_reflex_l = receiveByte_down();


	return;
}

void init_motors(){
	
	PORTB=0x00; //Reset any output
	PORTA=0x00; //Port A sets dir of motors (PA6, PA7)
	DDRA |=(1 << DDA6) | (1 << DDA7);
	DDRB|=(1 << DDB6 );//set OC3A as output.
	DDRD|=(1 << DDD5 );//set OC1A as output.
	
	//Setup timer1
	TCCR1A |= (1 << WGM10) | (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);
		
	//Setup timer3
	TCCR3A |= (1 << WGM30) | (1 << WGM31) | (1 << COM3A1);
	TCCR3B |= (1 << WGM32) | (1 << CS31) | (1 << CS30);
}

void setSpeed(uint8_t lspeed, uint8_t rspeed, uint8_t ldir , uint8_t rdir){
	
	PORTA |= (dir_left << DDA7) | (dir_right << DDA6); //DDA7 är vänster, DDA6 är höger 	
	OCR1A = 10*rspeed;//set the duty cycle(out of 1023) Höger	
	OCR3A = 10*lspeed;//set the duty cycle(out of 1023) Vänster
	
}


