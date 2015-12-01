#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lib/usart.h"
#include "lib/mem.h"
#include "lib/auto.h"
#include "lib/brain.h"

#define FORWARD 1
#define BACK 0
#define false 0
#define true 1
#define bool uint8_t

void init_motors();
void handle_messages();
uint8_t decide_if_repeated(uint8_t msg);

uint8_t dir_left = 1; //0 or 1 = back or forward
uint8_t dir_right = 1;
uint8_t spd_left = 0; //0 to 100 = procent of full speed
uint8_t spd_right = 0;
uint8_t button_autonom = 0; // 0 om manuellt läge, 1 om autonomt läge

uint8_t b1 = 2;
uint8_t b2 = 3;
uint8_t b3 = 4;
uint8_t b4 = 5;

bool new_message = false;
mapchange temp = {.x = 0, .y = 0, .t = 0};

uint8_t clk = 0;



int main(){
	

	
	init_USART_up(10);
	init_USART_down(10);
	init_mem();
	init_motors();
	init_auto();


	_delay_ms(1000);

	while(1){

		if(clk){

			clk = 0;
			PORTA &= 0xF7;
		}
		else{

			clk = 1;
			PORTA |= (1 << PORTA3);

		}		

		if(button_autonom != (PINA & 1)){
			spd_right = 0;
			spd_left = 0;
			setSpeed(0,0,FORWARD,FORWARD);
		}
		button_autonom = (PINA & 1);

		if(!spinning){
			update_sensor_data();
			handle_messages();	
		}	

		if (button_autonom == 1){
	
			//think();	
			autonom();
			
		}			
	}
}


void handle_messages(){


	if(checkUSARTflag_up()){	

		uint8_t message = receiveByte_up();	
		uint8_t message_cpy = message;

		//plocka ut OP-koden
		message_cpy &= 31;
		

		if (button_autonom == 0){ //Manuellt läge
			
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
					dir_left = FORWARD;
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
					dir_right = FORWARD;
					spd_left = spd_left + 50;
					break;
					case(7): //pil HÖGER släpps
					spd_left = spd_left - 50;
					break;			
				}
			
			setSpeed(spd_left,spd_right,dir_left,dir_right);
		}

		switch(message_cpy){
			
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
			//lägg debug
				transmitByte_up(debug);
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
				//transmitByte_up(t_vagg_v_b);
				transmitByte_up(t_vagg_v_b);
			break;
			
			case (0x17):
			//lägg reflex-token i send-buffern
				//transmitByte_up(t_reflex_u);
				//transmitByte_up(t_reflex_l);
			break;
			
			case (0x18):
			//lägg kartdata i send-buffern
				temp = gstack();
				transmitByte_up(temp.x);
				transmitByte_up(temp.y | (temp.t<<6));
			break;
			
			case (0x19):
			//senaste styrbeslut i send-buffern
				transmitByte_up(motor_l);
				transmitByte_up(motor_r);
			break;

			case (0x1A):
			//pos karta X och Y i send-buffern
				transmitByte_up(robot_pos_x);
				transmitByte_up(robot_pos_y);
			break;
			
			case (0x1B):
			//pos i algoritm i send-buffern
			//	transmitByte_up(posalgoritm);
			break;
			
			case (0x1C):
			//lägg tempKartdata i send-buffern


			break;

		}
	}		
}

void init_motors(){
	

	PORTB=0x00; //Reset any output
	PORTA=0x00; //Port A sets dir of motors (PA6, PA7)
	DDRA |=(1 << DDA6) | (1 << DDA7) | (0<<DDA0) | (1 << DDA1 | (1 << DDA3));
	DDRB|=(1 << DDB6 );//set OC3A as output.
	DDRD|=(1 << DDD5 );//set OC1A as output.
	
	//Setup timer1
	TCCR1A |= (1 << WGM10) | (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);
		
	//Setup timer3
	TCCR3A |= (1 << WGM30) | (1 << WGM31) | (1 << COM3A1);
	TCCR3B |= (1 << WGM32) | (1 << CS31) | (1 << CS30);
}




