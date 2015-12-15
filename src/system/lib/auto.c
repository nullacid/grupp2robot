#include "auto.h"
#include "usart.h"
#include "mem.h"
#include <avr/io.h>
#include <util/delay.h>

#define UPDATE 1
#define DONTUPDATE 0

#define MAX_SPEED_R 5 //1 to 10, 10 is highest
#define MAX_SPEED_L 5
#define PERFECT_DIST 	11 	//12

#define GYRO_NO_TURNING 0xB5 //KOMMER NOG ÄNDRAS
#define FOLlOW_WALL 0
#define MAP_REST 1

#define LEFT 1
#define RIGHT 2
#define NONE 0
#define TURNED_RIGHT 3

int distance_LIDAR;
uint8_t first_time;
uint8_t NODBROMS = 0;
uint8_t parallell_cnt = 0;

int16_t deviation_from_wall = 0;
int16_t old_deviation_from_wall = 0;
int16_t derivata = 0;
int16_t P = 0;
int16_t D = 0;
uint8_t pidk = 4; //20 är okej
uint8_t pidd = 4; //16 ok ish
uint8_t front_sensor_active = 0;
uint8_t regulate_side = 0; 
uint8_t sensor_start = 0;
uint8_t old_action = 0;

uint16_t temptemp_action_done_c = 0;

void update_sensor_data(); 
void init_auto();
void action_done(uint8_t update_map);
void reset_reflex();


void init_auto(){
	distance_LIDAR = 0;
	first_time = 1;
	s_ir_front = 0;
	t_vagg_front  = 0;

	s_ir_h_f = 0;
	s_ir_h_b = 0;
	s_ir_v_f = 0;
	s_ir_v_b = 0;

	//	Token parallell vänster/höger

	t_p_h = 0;	// 0- parallell, 1- lite off , 2- mer off , FF - ej användbart
	t_p_v = 0;	// 0- parallell, 1- lite off , 2- mer off , FF - aj användbart

	t_vagg_h_f = 0; // 0- ingen vägg , 1 - vägg inom 20 cm ish, 2 - vägg 20+40 cm
	t_vagg_h_b = 0; // 0- ingen vägg , 1 - vägg inom 20 cm ish, 2 - vägg 20+40 cm
	t_vagg_v_f = 0; // 0- ingen vägg , 1 - vägg inom 20 cm ish, 2 - vägg 20+40 cm
	t_vagg_v_b = 0; // 0- ingen vägg , 1 - vägg inom 20 cm ish, 2 - vägg 20+40 cm

	s_gyro_u = 0;
	s_gyro_l = 0;
	s_gyro = 0;
	t_gyro = 0;		// bestäm vilka värden vi vill ha

	spinning = 0;


	wmem_auto(FLOOR, robot_pos_x, robot_pos_y); //Mark start tile as foor
	curr_action = PARALLELIZE;
	dir = NORTH;

	//----------------------------
}
void update_sensor_data(){
	//från 08 ---> 1 7 rader
	transmitByte_down(0x1D); //fråga efter all data

	s_ir_h_f = receiveByte_down();
	s_ir_h_b = receiveByte_down();
	s_ir_v_f = receiveByte_down();
	s_ir_v_b = receiveByte_down();
	s_ir_front = receiveByte_down();

	t_gyro = receiveByte_down();
	t_p_h = receiveByte_down();
	t_p_v = receiveByte_down();
	t_vagg_h_f = receiveByte_down();
	t_vagg_h_b = receiveByte_down();
	t_vagg_v_f = receiveByte_down();
	t_vagg_v_b = receiveByte_down();
	t_vagg_front = receiveByte_down();
	t_reflex = receiveByte_down();


	s_gyro = ((s_gyro_u << 8) + s_gyro_l);

	return;
}

void autonom (){

	uint8_t direction = 1;
	old_action = curr_action;
	switch(curr_action){
		case (EMPTY):
			setSpeed(0, 0, 0, 0);
			//action_done(UPDATE);
		break;
//------------------------ÅKA FRAMMÅT--------------
		case (FORWARD):

			if((t_vagg_h_f == 2) && (t_vagg_h_b == 2)){ //Decide which side to regulate on
				regulate_side = RIGHT;
			}
			else{
				regulate_side = NONE;
			}
			
			if(t_vagg_h_b == 2){
				if(derivata > 3){ //4
					regulate_side = NONE;
				}
			}

			if((t_vagg_h_f != 2) && (t_vagg_h_b == 2)){
				curr_action = NUDGE_FORWARD;
				if(dir == NORTH){
					robot_pos_y--;
				}
				else if(dir == WEST){
					robot_pos_x--;
				}
				else if(dir == SOUTH){
					robot_pos_y++;
				}
				else{
					robot_pos_x++;
				}
				break;
			}
			
			deviation_from_wall = (s_ir_h_f - PERFECT_DIST);
			derivata = (deviation_from_wall - old_deviation_from_wall);

			if(regulate_side == RIGHT){ //Om det finns en vägg höger eller vänster fram, reglera efter den

				int8_t control = 0;
				uint8_t lspeed = 0;
				uint8_t rspeed = 0;

				P = pidk * deviation_from_wall;
				D = pidd * t_p_h;
				control = P+D;
				if(control > 0){
					rspeed = 100 - control;
					lspeed = 100 + control;
				}
				else if(control < 0){
					//nära höger vägg
					lspeed = 100 + control;
					rspeed = 100 - control;
				}
				else{
					lspeed = 100;
					rspeed = 100;
				}
				if(deviation_from_wall > 8){
					rspeed = 0;
					lspeed = 100;
				}
				else if(deviation_from_wall < -8){
					lspeed = 0;
					rspeed = 100;
				}

				motor_r = rspeed;
				motor_l = lspeed;
				setSpeed(lspeed , rspeed,1,1);
			}

			else{
				setSpeed(100, 100, 1, 1);
			}
			
			old_deviation_from_wall = deviation_from_wall;

			if( (t_reflex > 31) || ( (s_ir_front < 12) && (s_ir_front > 1) ) ){
				first_time = 1;

				if(t_reflex > 26){ //var 20
					if(dir == NORTH){
						robot_pos_y--;
					}
					else if(dir == WEST){
						robot_pos_x--;
					}
					else if(dir == SOUTH){
						robot_pos_y++;
					}
					else{
						robot_pos_x++;
					}
				}


				first_time_on_island = 0;
				curr_action = EMPTY;
				if(land_o_hoy == 0){

					next_action = SPIN_L;
					land_o_hoy = 1;
					first_time_on_island = 1;
					island_x = robot_pos_x;
					island_y = robot_pos_y;
					if(follow_island == 1){
						follow_island = 0;
					}
					else{
						follow_island = 1;
					}
					
				}

				if((s_ir_front < 14)&&(s_ir_front > 1)){ //fuckar upp vår position annars
					setSpeed(0,0,FORWARD,FORWARD);
					action_done(DONTUPDATE);
				}
				else{
					action_done(UPDATE);
				}
			}
			
		break;

		case(NUDGE_FORWARD):

			setSpeed(30,30,1,1);
			if(t_reflex > 3){
				curr_action = EMPTY;
				if(t_vagg_h_b != 2){
					next_action = LAST_NUDGE;
				}
				else{
					next_action = NUDGE_FORWARD;
				}
				action_done(DONTUPDATE);
			}

		break;

		case(NUDGE_TO_WALL):

			setSpeed(50,50,1,1);
			if(s_ir_front < 13){
				curr_action = EMPTY;
				action_done(UPDATE);
			}

		break;

		case(LAST_NUDGE):
			setSpeed(30,30,1,1);
			if(t_reflex > 3){ //3
				curr_action = EMPTY;
				next_action = SPIN_R;
				action_done(UPDATE);
			}
		break;

//---------------------------------SVÄNGA--------------------------------
		case (SPIN_R):

			if(first_time){
				first_time = 0;
				spinning = 1;
				setSpeed(70, 70, 1, 0);
				transmitByte_down(0x1C);
			}

			if(t_gyro == 0x44){
				dir++;
				first_time = 1;
				spinning = 0;
				transmitByte_down(0x1E);

				curr_action = FORWARD;
				action_done(DONTUPDATE);

			}
		break;

		case (SPIN_L):

			if(first_time){
				spinning = 1;
				first_time = 0;
				setSpeed(70, 70, 0, 1); //Höger hjulpar bakåt
				transmitByte_down(0x1F);
			}

			if(t_gyro == 0x44){

				if(dir == 0){
					dir = 3;
				}
				else{
					dir--;
				}	

				spinning = 0;
				first_time = 1;	
				transmitByte_down(0x1E);

				curr_action = EMPTY;

				if(land_o_hoy == 1){
					curr_action = PARALLELIZE;
				}
				else{
					next_action = FORWARD;
				}

				action_done(DONTUPDATE);


			}
		break;

		case(P_WEAK):

			if(t_p_h == 0){
				setSpeed(0, 0, FORWARD, FORWARD);
				curr_action = EMPTY;
				action_done(UPDATE);
			}

			else if(t_p_h == 127){
				setSpeed(50,50,0,1);
				parallell_cnt = 0;
			}
			else if (t_p_h > 0){ //påväg från väggen
				setSpeed(40 * t_p_h, 40 * t_p_h, 1, 0); //Speed till 40 eller 80 beroende på hur fel vi är
				parallell_cnt = 0;
			}	
			else if (t_p_h < 0){ //påväg in i väggen
				setSpeed(40 * (-t_p_h), 40 * (-t_p_h), 0, 1); //Speed till 40 eller 80 beroende på hur fel vi är
				parallell_cnt = 0;
			}

		break;

		case(P_WEAK_L):

			if(t_p_v == 0){
				setSpeed(0, 0, FORWARD, FORWARD);
				curr_action = EMPTY;
				action_done(DONTUPDATE);
			}

			else if(t_p_v == 127){
				setSpeed(50,50,0,1);
				parallell_cnt = 0;
			}
			else if (t_p_v > 0){ //påväg från väggen
				setSpeed(40 * t_p_v, 40 * t_p_v, 0, 1); //Speed till 40 eller 80 beroende på hur fel vi är
				parallell_cnt = 0;
			}	
			else if (t_p_v < 0){ //påväg in i väggen
				setSpeed(40 * (-t_p_v), 40 * (-t_p_v), 1, 0); //Speed till 40 eller 80 beroende på hur fel vi är
				parallell_cnt = 0;
			}

		break;

		case(PARALLELIZE):
			if (t_p_h == 0){ //Parallellt

				parallell_cnt++;

				if(parallell_cnt >= 10){
					setSpeed(0, 0, FORWARD, FORWARD);
					//dir = NORTH;
					//curr_action = EMPTY;
					curr_action = EMPTY;
					action_done(UPDATE);
				}
			}	
			else if(t_p_h == 127){
				setSpeed(50,50,1,0); // spin right
				parallell_cnt = 0;
			}
			else if (t_p_h > 1){ //påväg från väggen
				setSpeed(60, 60, 1, 0); //Speed till 40 eller 80 beroende på hur fel vi är
				parallell_cnt = 0;
			}	
			else if (t_p_h < -1){ //påväg in i väggen
				setSpeed(60, 60, 0, 1); //Speed till 40 eller 80 beroende på hur fel vi är
				parallell_cnt = 0;
			}
			
		break;

		case(BACKWARD):
			if (first_time){
				sensor_start = s_ir_front;
				first_time = 0;
			}
			//Kolla så vi åker typ parallellt

			setSpeed(30, 30, 0, 0);
							
			if(s_ir_front >= 11){
				first_time = 1;
				curr_action = EMPTY;
				action_done(UPDATE);
			}
		break;

		default:
			debug = 0xFF;
		break;
	}


	return;
}

void action_done(uint8_t update_map){

	temptemp_action_done_c++;

	if(dir>3){
		dir -=4;
	}

	//------------UPPDATERA KARTDATA ----------------
	//Räknat med 0,0 i övre högra hörnet
	parallell_cnt = 0;
	old_deviation_from_wall = 0;
	derivata = 0;
	transmitByte_down(0x21); //Reset reflex-segments
	receiveByte_down();

	if((next_action != 0) && (curr_action == 0)){
		curr_action = next_action;
		next_action = 0;
	}

	setSpeed(0,0,FORWARD,FORWARD);
	_delay_ms(100);

	int8_t temp_x = 0;
	int8_t temp_y = 0;

	if(update_map == 1){
		switch(dir){
			case(0):
				temp_x = 0;
				temp_y = -1;
			break;

			case(1):
				temp_x = 1;
				temp_y = 0;
			break;

			case(2):
				temp_x = 0;
				temp_y = 1;
			break;

			case(3):
				temp_x = -1;
				temp_y = 0;
			break;
		}

		wmem_auto(FLOOR, robot_pos_x, robot_pos_y);

		if(t_vagg_front != 2){
			//wmem_auto(FLOOR, robot_pos_x + temp_x, robot_pos_y + temp_y);
		}
		else if((t_vagg_front == 2) || (old_action == BACKWARD) || (old_action == NUDGE_TO_WALL)){ //IR WALL
			//wmem_auto(IWALL, robot_pos_x + temp_x, robot_pos_y + temp_y); 
			
		}
		
		if ((t_vagg_h_f == 0) && (t_vagg_h_b == 0)){ //HÖGER IR FLOOR
			wmem_auto(FLOOR, robot_pos_x - temp_y  , robot_pos_y + temp_x); 
			wmem_auto(FLOOR, robot_pos_x - temp_y*2 , robot_pos_y + temp_x * 2); 
		}

		if ((t_vagg_h_f == 1) && (t_vagg_h_b == 1)){ //HÖGER IR WALL + 1 FLOOR
			wmem_auto(WALL, robot_pos_x - temp_y * 2, robot_pos_y + temp_x * 2); 
			wmem_auto(FLOOR, robot_pos_x - temp_y, robot_pos_y + temp_x); 
		}

		if ((t_vagg_h_f == 2) && (t_vagg_h_b == 2)){ //HÖGER IR WALL
			wmem_auto(WALL, robot_pos_x - temp_y , robot_pos_y + temp_x); 
		}	

		if ((t_vagg_v_f == 0) && (t_vagg_v_b == 0)){ //VÄNSTER IR FLOOR
			wmem_auto(FLOOR, robot_pos_x + temp_y * 2 , robot_pos_y - temp_x * 2); 
			wmem_auto(FLOOR, robot_pos_x + temp_y , robot_pos_y - temp_x); 
		}

		if ((t_vagg_v_f == 1) && (t_vagg_v_b == 1)){ //VÄNSTER IR WALL + 1 FLOOR
			wmem_auto(IWALL, robot_pos_x + temp_y * 2, robot_pos_y - temp_x * 2); 				
			wmem_auto(FLOOR, robot_pos_x + temp_y, robot_pos_y - temp_x); 
		}

		if ((t_vagg_v_f == 2) && (t_vagg_v_b == 2)){ //VÄNSTER IR WALL
			wmem_auto(IWALL, robot_pos_x + temp_y , robot_pos_y - temp_x); 
			
		}		
	}
}

void setSpeed(uint8_t lspeed, uint8_t rspeed, uint8_t ldir , uint8_t rdir){
	
		if(ldir){
			PORTA |= (1 << DDA7);			
		}
		else{			
			PORTA &= 0x7F;
		}

		if(rdir){
			PORTA |= (1 << DDA6);
		}
		else{			
			PORTA &= 0xBF;
		}

	uint16_t rdone = rspeed * MAX_SPEED_R;
	uint16_t ldone = lspeed * MAX_SPEED_L;

	if (rdone > 28){ // super magic number for driving straight
		rdone -= 28;
	}

	// to not crash
	if(rdone > 1000){
		rdone = 1000;
	}
	if(ldone > 1000){
		ldone = 1000;
	}

	OCR1A = rdone;//set the duty cycle(out of 1023) Höger	(pin 19)
	OCR3A = ldone;//set the duty cycle(out of 1023) Vänster (pin 7)
	
}