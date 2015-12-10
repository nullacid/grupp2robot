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

	t_p_h = receiveByte_down();;
	t_p_v = receiveByte_down();
	//t_gyro = receiveByte_down();
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

	debug = deviation_from_wall;

	uint8_t direction = 1;

	switch(curr_action){
		case (EMPTY):
			setSpeed(0, 0, 0, 0);
			action_done(UPDATE);
		break;

//------------------------ÅKA FRAMMÅT--------------
		case (FORWARD):
			if (first_time){
				if(t_vagg_front == 1){
					front_sensor_active = 1;
				}
				else{
					front_sensor_active = 0;
				}
				first_time = 0;
			}

			if((t_vagg_h_f == 2) && (t_vagg_h_b == 2)){ //Decide which side to regulate on
				regulate_side = RIGHT;
			}
			else if((t_vagg_v_f == 2) && (t_vagg_v_b == 2)){
				regulate_side = NONE; //ÄNDRA MIG HALLÅ
			}
			else{
				regulate_side = NONE;
			}
			
			if(t_vagg_h_b == 2){
				if(derivata > 5){ //6
					regulate_side = NONE;
				}
			}
			

			if(regulate_side == RIGHT){ //Om det finns en vägg höger eller vänster fram, reglera efter den

				int8_t control = 0;
				uint8_t lspeed = 0;
				uint8_t rspeed = 0;
				direction = 1;
				int8_t ir_difference = 0;

				if(regulate_side == RIGHT){
					deviation_from_wall = (s_ir_h_f - PERFECT_DIST);
				}
				else{
					deviation_from_wall = -(s_ir_v_f - PERFECT_DIST);
				}

				derivata = (deviation_from_wall - old_deviation_from_wall);

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

				old_deviation_from_wall = deviation_from_wall;
				motor_r = rspeed;
				motor_l = lspeed;
				setSpeed(lspeed , rspeed,1,1);
			}

			else{
				setSpeed(50, 50, 1, 1);
			}
			
			if((t_reflex > 30)||((s_ir_front < 12)&&(s_ir_front > 1))){
				first_time = 1;

				if(t_reflex > 20){
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
				curr_action = EMPTY;
				if((s_ir_front < 12)&&(s_ir_front > 1)){ //fuckar upp vår position annars
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
				action_done(UPDATE);
			}

		break;

		case(NUDGE_TO_WALL):

			setSpeed(30,30,1,1);
			if(s_ir_front < 12){
				curr_action = EMPTY;
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
				//curr_action = EMPTY;
				//curr_action = P_WEAK; // 
				curr_action = PARALLELIZE;

				action_done(DONTUPDATE);
			}
		break;

		/*case (SPIN_180):
			
			if(first_time){
				spinning = 1;
				first_time = 0;
				setSpeed(70, 70, 1, 0); //Höger hjulpar bakåt
				transmitByte_down(0x20);
			}

			if(t_gyro == 0x44){
				dir += 2;
				first_time = 1;
				spinning = 0;
				transmitByte_down(0x1E);
				curr_action = FORWARD;
				action_done(DONTUPDATE);
			}

		break;
		*/
//------------------------------------------------------------------------

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
				action_done(UPDATE);
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
					_delay_ms(100);
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
							
			if(s_ir_front >= 9){
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

	if(dir>3){
		dir -=4;
	}

	//if (old_action != cur_action){ 
		setSpeed(0, 0, 1, 1); //Stanna om vi inte ska fortsätta i samma riktning
		_delay_ms(100);
	//}
	//------------UPPDATERA KARTDATA ----------------
	//Räknat med 0,0 i övre högra hörnet
	parallell_cnt = 0;
	old_deviation_from_wall = 0;
	derivata = 0;
	transmitByte_down(0x21); //Reset reflex-segments
	receiveByte_down();

	if(update_map){
		switch(dir){
			int i;
			case (0): //LIDAR to the NORTH
				//INFOGA SPECIALVÄRDE 
				if (t_vagg_front == 0){
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y - 1);
				}
				if (t_vagg_front == 1){ //IR WALL + 1 FLOOR
				wmem_auto(FLOOR, robot_pos_x, robot_pos_y - 1); 
				wmem_auto(WALL, robot_pos_x, robot_pos_y - 2); //LIDAR WALL
				}
				if (t_vagg_front == 2){ //IR WALL
				wmem_auto(WALL, robot_pos_x, robot_pos_y - 1); 
				}
				if (t_vagg_h_f == 0 && t_vagg_h_b == 0){ //HÖGER IR FLOOR
					wmem_auto(FLOOR, robot_pos_x +1 , robot_pos_y); 
					wmem_auto(FLOOR, robot_pos_x +2 , robot_pos_y); 
				}

				if (t_vagg_h_f == 2 && t_vagg_h_b == 2){ //HÖGER IR WALL
					wmem_auto(WALL, robot_pos_x +1 , robot_pos_y); 
				}

				if (t_vagg_h_f == 1 && t_vagg_h_b == 1){ //HÖGER IR WALL + 1 FLOOR
					wmem_auto(WALL, robot_pos_x + 2, robot_pos_y); 
					wmem_auto(FLOOR, robot_pos_x + 1, robot_pos_y); 
				}

				if (t_vagg_v_f == 0 && t_vagg_v_b == 0){ //VÄNSTER IR FLOOR
					wmem_auto(FLOOR, robot_pos_x -1 , robot_pos_y); 
					wmem_auto(FLOOR, robot_pos_x -2 , robot_pos_y); 
				}

				if (t_vagg_v_f == 2 && t_vagg_v_b == 2){ //VÄNSTER IR WALL
					wmem_auto(WALL, robot_pos_x - 1 , robot_pos_y); 
				}

				if (t_vagg_v_f == 1 && t_vagg_v_b == 1){ //VÄNSTER IR WALL + 1 FLOOR
					wmem_auto(WALL, robot_pos_x - 2, robot_pos_y); 
					wmem_auto(FLOOR, robot_pos_x - 1, robot_pos_y); 
				}
				


			break;
			case (1): //LIDAR to the EAST
				if (t_vagg_front == 0){
					wmem_auto(FLOOR, robot_pos_x +1, robot_pos_y);
				}
				if (t_vagg_front == 1){ //IR WALL + 1 FLOOR
				wmem_auto(FLOOR, robot_pos_x +1 , robot_pos_y); 
				wmem_auto(WALL, robot_pos_x +2 , robot_pos_y); //LIDAR WALL
				}
				if (t_vagg_front == 2){ //IR WALL
				wmem_auto(WALL, robot_pos_x + 1, robot_pos_y); 
				}
				if (t_vagg_h_f == 0 && t_vagg_h_b == 0){ //HÖGER IR FLOOR
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y + 1); 
					wmem_auto(FLOOR, robot_pos_x , robot_pos_y + 2); 
				}

				if (t_vagg_h_f == 2 && t_vagg_h_b == 2){ //HÖGER IR WALL
					wmem_auto(WALL, robot_pos_x, robot_pos_y +1); 
				}

				if (t_vagg_h_f == 1 && t_vagg_h_b == 1){ //HÖGER IR WALL + 1 FLOOR
					wmem_auto(WALL, robot_pos_x, robot_pos_y + 2); 
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y + 1); 
				}
				if (t_vagg_v_f == 0 && t_vagg_v_b == 0){ //VÄNSTER IR FLOOR
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y-1); 
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y-2); 
				}
				if (t_vagg_v_f == 2 && t_vagg_v_b == 2){ //VÄNSTER IR WALL
					wmem_auto(WALL, robot_pos_x, robot_pos_y - 1); 
				}

				if (t_vagg_v_f == 1 && t_vagg_v_b == 1){ //VÄNSTER IR WALL + 1 FLOOR
					wmem_auto(WALL, robot_pos_x, robot_pos_y - 2); 
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y - 1); 
				}


			break;
			case (2): //LIDAR to the SOUTH

				if (t_vagg_front == 0){
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y + 1);
				}
				if (t_vagg_front == 1){ //IR WALL + 1 FLOOR
				wmem_auto(FLOOR, robot_pos_x, robot_pos_y + 1); 
				wmem_auto(WALL, robot_pos_x, robot_pos_y + 2); //LIDAR WALL
				}
				if (t_vagg_front == 2){ //IR WALL
				wmem_auto(WALL, robot_pos_x, robot_pos_y + 1); 
				}
				if (t_vagg_h_f == 0 && t_vagg_h_b == 0){ //HÖGER IR FLOOR
					wmem_auto(FLOOR, robot_pos_x -1 , robot_pos_y); 
					wmem_auto(FLOOR, robot_pos_x -2 , robot_pos_y); 
				}

				if (t_vagg_h_f == 2 && t_vagg_h_b == 2){ //HÖGER IR WALL
					wmem_auto(WALL, robot_pos_x -1 , robot_pos_y); 
				}

				if (t_vagg_h_f == 1 && t_vagg_h_b == 1){ //HÖGER IR WALL + 1 FLOOR
					wmem_auto(WALL, robot_pos_x - 2, robot_pos_y); 
					wmem_auto(FLOOR, robot_pos_x - 1, robot_pos_y); 
				}
				if (t_vagg_v_f == 0 && t_vagg_v_b == 0){ //VÄNSTER IR FLOOR
					wmem_auto(FLOOR, robot_pos_x +1 , robot_pos_y); 
					wmem_auto(FLOOR, robot_pos_x +2 , robot_pos_y); 
				}

				if (t_vagg_v_f == 2 && t_vagg_v_b == 2){ //VÄNSTER IR WALL
					wmem_auto(WALL, robot_pos_x + 1 , robot_pos_y); 
				}

				if (t_vagg_v_f == 1 && t_vagg_v_b == 1){ //VÄNSTER IR WALL + 1 FLOOR
					wmem_auto(WALL, robot_pos_x + 2, robot_pos_y); 
					wmem_auto(FLOOR, robot_pos_x + 1, robot_pos_y); 
				}

			break;
			case (3): //LIDAR to the WEST
				if (t_vagg_front == 0){
					wmem_auto(FLOOR, robot_pos_x -1, robot_pos_y);
				}
				if (t_vagg_front == 1){ //IR WALL + 1 FLOOR
				wmem_auto(FLOOR, robot_pos_x -1 , robot_pos_y); 
				wmem_auto(WALL, robot_pos_x -2 , robot_pos_y); //LIDAR WALL
				}
				if (t_vagg_front == 2){ //IR WALL
				wmem_auto(WALL, robot_pos_x - 1, robot_pos_y); 
				}
				if (t_vagg_h_f == 0 && t_vagg_h_b == 0){ //HÖGER IR FLOOR
					wmem_auto(FLOOR, robot_pos_x , robot_pos_y -1); 
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y -2); 
				}

				if (t_vagg_h_f == 2 && t_vagg_h_b == 2){ //HÖGER IR WALL
					wmem_auto(WALL, robot_pos_x, robot_pos_y -1); 
				}

				if (t_vagg_h_f == 1 && t_vagg_h_b == 1){ //HÖGER IR WALL + 1 FLOOR
					wmem_auto(WALL, robot_pos_x, robot_pos_y - 2); 
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y - 1); 
				}
				if (t_vagg_v_f == 0 && t_vagg_v_b == 0){ //VÄNSTER IR FLOOR
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y + 1); 
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y + 2); 
				}

				if (t_vagg_v_f == 2 && t_vagg_v_b == 2){ //VÄNSTER IR WALL
					wmem_auto(WALL, robot_pos_x, robot_pos_y + 1); 
				}

				if (t_vagg_v_f == 1 && t_vagg_v_b == 1){ //VÄNSTER IR WALL + 1 FLOOR
					wmem_auto(WALL, robot_pos_x, robot_pos_y + 2); 
					wmem_auto(FLOOR, robot_pos_x, robot_pos_y + 1); 
				}


			break;
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