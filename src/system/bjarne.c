/*
	Created: November 2015
 *  Author: Mikael Å and Anton R
 * "Jag gillar inte länkade listor" - Bjarne Stroustrup

 * This is the main file for the steering module.
 * It handles the motors and messages from the communication module.
 * The system's decision making is done in other files, but are called from the main function here.

*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lib/usart.h"
#include "lib/mem.h"
#include "lib/auto.h"
#include "lib/brain.h"


#define FORWARD 1 //Values for motor direction
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
uint8_t prev_autonom = 0; // för att stanna om man switchar tillbaka till manuellt


bool new_message = false;
mapchange temp = {.x = 0, .y = 0, .t = 0};

uint8_t clk = 0;



int main(){
	
	//Initialize USART communication with value 10 to get a baud rate of 115200.
	init_USART_up(10);
	init_USART_down(10);
	init_mem();		//Initialize Map Memory
	init_motors();	//Initialize motors
	init_auto();	//Initialize autonomous algorithm

	_delay_ms(1500);

	while(1){
		//Stop system when autonomous/manual switch is turned
		if(button_autonom != (PINA & 1)){
			spd_right = 0;
			spd_left = 0;
			setSpeed(0,0,FORWARD,FORWARD);
		}
		button_autonom = (PINA & 1);


		update_sensor_data();	//get sensor data from sensor module
		handle_messages();		//handle message from communication module


		if(button_autonom == 1){	//in autonomous mode?
			if(map_complete == 0){	
				think();			//get next action
				autonom();			//do next action
			}
			else{
				setSpeed(0,0,1,1);	//stop if map is finished
			}
		}			
		if((prev_autonom == 1) && (button_autonom == 0)){ //Stop any autonomous movement when switch is set to manual.
			setSpeed(0,0,FORWARD,FORWARD);
		}
		prev_autonom = button_autonom;

	}
}

/*
 *	Handles messages from communication module.
 *	Messages are either steering commands or data fetches.
 *	Steering commands are disregarded in autonomous mode.
 */
void handle_messages(){


	if(checkUSARTflag_up()){	 //Is there a command from the communication module?

		uint8_t message = receiveByte_up();		//get the command
		uint8_t message_cpy = message;

		message_cpy &= 0x3F;		//get the op-code.
		

		if (button_autonom == 0){ //Manual mode
			
				switch(message_cpy){
					case (0): //W is pressed (go forward)

						dir_left = FORWARD;
						dir_right = FORWARD;
						spd_left = spd_left + 50;
						spd_right = spd_right + 50;
					break;
					case(1): //W is released (stop forward)
						spd_left = spd_left - 50;
						spd_right = spd_right - 50;
					break;
					case(2): //A is pressed (turn left)
						dir_left = FORWARD;
						spd_right = spd_right + 50;
					break;
					case(3): //A is released (stop left)
						spd_right = spd_right - 50;
					break;
					case (4): //S is pressed (go back)
						dir_left = BACK;
						dir_right = BACK;
						spd_left = spd_left + 50;
						spd_right = spd_right + 50;
					break;
					case(5): //S is released (stop back)
						spd_left = spd_left - 50;
						spd_right = spd_right - 50;
						dir_left = FORWARD;
						dir_right = FORWARD;

					break;
					case(6): //D is pressed (turn right)
						dir_right = FORWARD;
						spd_left = spd_left + 50;
					break;
					case(7): //D is released (stop right)
						spd_left = spd_left - 50;
					break;

					case(0x22): //Q is pressed (spin left)
						dir_right = FORWARD;
						dir_left = BACK;
						spd_left = 100;
						spd_right = 100;

					break;
					case(0x23): //Q is released (stop all)
						dir_left = FORWARD;
						spd_left = 0;
						spd_right = 0;
					break;

					case(0x24): //E is pressed (spin right)
						dir_right = BACK;
						dir_left = FORWARD;
						spd_left = 100;
						spd_right = 100;
					break;
					case(0x25): //E is released (stop all)
						dir_right = FORWARD;
						spd_right = 0;
						spd_left = 0;
					break;

				}
			//Set motor speed to determined values.
			setSpeed(spd_left,spd_right,dir_left,dir_right);
		}

		switch(message_cpy){	//Data fetch commands
			
			case (0x08):
				//send IR Front sensor
				transmitByte_up(s_ir_front);

			break;
			
			case (0x09):
				//send IR Right Front Sensor
				transmitByte_up(s_ir_h_f);

			break;
			
			case (0x0A):
				//send IR Right Back Sensor
				transmitByte_up(s_ir_h_b);
			break;
			
			case (0x0B):
				//send IR Left Front Sensor
				transmitByte_up(s_ir_v_f);
			break;
			
			case (0x0C):
				//send IR Left Back Sensor
				transmitByte_up(s_ir_v_b);
			break;
			
			case (0x0D):
				//send distance covered
				transmitByte_up(distance_covered >> 8);
				waitForSendNext_up();
				transmitByte_up(distance_covered & 0xFF);

			break;
			
			case (0x0E):
				//send debug value (can be whatever we need it to be)
				transmitByte_up(debug);
			break;
			
			case (0x0F):
				//send IR Front token 
				transmitByte_up(t_vagg_front);
			break;
			
			case (0x10):
				//send Parallel right token
				transmitByte_up(t_p_h);
			break;
			
			case (0x11):
				//send Parallel left token
				transmitByte_up(t_p_v);
			break;
			
			case (0x13):
				//send IR Right Front token
				transmitByte_up(t_vagg_h_f);
			break;
			
			case (0x14):
				//send IR Right Back token
				transmitByte_up(t_vagg_h_b);
			break;
			
			case (0x15):
				//send IR Left Front token
				transmitByte_up(t_vagg_v_f);
			break;
			
			case (0x16):
				//Send IR Left Back Token
				transmitByte_up(t_vagg_v_b);
			break;
			
			
			case (0x18):
				//Send mapdata from the change queue
				temp = dequeue();
				transmitByte_up(temp.x);
				waitForSendNext_up();
				transmitByte_up(temp.y | (temp.t<<6));
			break;
			
			case (0x19):
				//send last motor output
				transmitByte_up(motor_l);
				waitForSendNext_up();
				transmitByte_up(motor_r);
			break;

			case (0x1A):
				//send current system position
				transmitByte_up(robot_pos_x);
				waitForSendNext_up();
				transmitByte_up(robot_pos_y);
			break;
			
			case (0x1B):
				//Send current action
				transmitByte_up(curr_action);
			break;

		}
	}		
}

void init_motors(){
	
	button_autonom = 0;
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




