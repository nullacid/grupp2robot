/*
 * Created: November 2015
 * Author : Mikael Å, Anton R, Michael S
 * "One reason you should not use web applications to do your computing is that you lose control." -Richard Stallman
 *
 * The spine of the robot, translating actions into motion of the wheels, while providing the "eyes" updating the map memory
 */ 
 
#include "auto.h"
#include "usart.h"
#include "mem.h"
#include <avr/io.h>
#include <util/delay.h>

//-----DEFINES------

#define UPDATE 			1
#define DONTUPDATE 		0
#define MAX_SPEED_R 	5 //5//Max speed multiplier of right motors
#define MAX_SPEED_L 	5 	//Max speed multiplier of left motors
#define PERFECT_DIST 	11	//Perfect distance from robot IR-sensors to wall	
#define LEFT 			1 	//Which side to regulate on
#define RIGHT 			2
#define NONE 			0

//-----VARIABLES------
uint8_t first_time 				= 0; //Used for startup-only inits in an action
uint8_t parallell_cnt 			= 0;
int16_t deviation_from_wall 	= 0; //PERFECT_DIST - IR front right sensor value
int16_t old_deviation_from_wall = 0; 
int16_t derivata 				= 0; //Current old_deviation_from_wall - deviation_from_wall
int16_t P 						= 0; 
int16_t D 						= 0;
uint8_t pidk 					= 4; 
uint8_t pidd 					= 4; 
uint8_t regulate_side 			= 0;
uint8_t old_action 				= 0; 

//-----PROTOTYPES------
void update_sensor_data();
void init_auto();
void action_done(uint8_t update_map);
void reset_reflex();


void init_auto(){

	first_time 		= 1;
	s_ir_front 		= 0;
	t_vagg_front  	= 0;
	s_ir_h_f 		= 0;
	s_ir_h_b 		= 0;
	s_ir_v_f 		= 0;
	s_ir_v_b 		= 0;
	t_p_h 			= 0;
	t_p_v 			= 0;
	t_vagg_h_f 		= 0;
	t_vagg_h_b 		= 0; 
	t_vagg_v_f 		= 0; 
	t_vagg_v_b 		= 0; 
	t_gyro 			= 0;
	spinning 		= 0;

	wmem_auto(FLOOR, robot_pos_x, robot_pos_y); //Mark start tile as foor
	curr_action = PARALLELIZE; //Set first action as parallelize
	dir = NORTH; //Set start direction as north

	//----------------------------
}
void update_sensor_data(){

	transmitByte_down(0x1D); //Request all sensor data

	s_ir_h_f 		= receiveByte_down();
	s_ir_h_b 		= receiveByte_down();
	s_ir_v_f 		= receiveByte_down();
	s_ir_v_b 		= receiveByte_down();
	s_ir_front 		= receiveByte_down();
	t_gyro 			= receiveByte_down();
	t_p_h 			= receiveByte_down();
	t_p_v 			= receiveByte_down();
	t_vagg_h_f 		= receiveByte_down();
	t_vagg_h_b 		= receiveByte_down();
	t_vagg_v_f 		= receiveByte_down();
	t_vagg_v_b 		= receiveByte_down();
	t_vagg_front 	= receiveByte_down();
	t_reflex 		= receiveByte_down();

	return;
}

void autonom (){

	old_action = curr_action; //Remember the old action

	switch(curr_action){

		//-----NO ACTION-----

		case (EMPTY): //If no action, stop
			setSpeed(0, 0, FORWARD, FORWARD);
		break;

		//-----GO FORWARD-----

		case (FORWARD):

			if((t_vagg_h_f == 2) && (t_vagg_h_b == 2)){ //Decide which side to regulate on
				regulate_side = RIGHT;
			}
			else{
				regulate_side = NONE;
			}
			
			if(t_vagg_h_b == 2){ //If the derivata is too large, don't regulate
				if(derivata > 3){
					regulate_side = NONE;
				}
			}

			if((t_vagg_h_f != 2) && (t_vagg_h_b == 2)){	//If the front sensor lost its wall, break the forward action and nudge forward
				curr_action = NUDGE_FORWARD;
				if(dir == NORTH){ //Update the robots position
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
				distance_covered ++;
			}
			
			deviation_from_wall = (s_ir_h_f - PERFECT_DIST);
			derivata = (deviation_from_wall - old_deviation_from_wall);

			if(regulate_side == RIGHT){ //Normal regulation

				int8_t control = 0; //The regulate value
				uint8_t lspeed = 0;
				uint8_t rspeed = 0;

				P = pidk * deviation_from_wall; //Calculate contol value from derivata and parallel value
				D = pidd * t_p_h;
				control = P+D;

				if(control > 0){ //Add and subtract abs(control) to the wheels max speed in order to regulate
					rspeed = 100 - control;
					lspeed = 100 + control;
				}
				else if(control < 0){ //Too close to the wall, add power to the right pair of wheels
					lspeed = 100 + control;
					rspeed = 100 - control;
				}
				else{
					lspeed = 100;
					rspeed = 100;
				}
				if(deviation_from_wall > 8){ //Avoid overflow
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

			else{ //Do not regulate
				setSpeed(100, 100, 1, 1);
			}
			
			old_deviation_from_wall = deviation_from_wall;
												
			if( (t_reflex > 31) || ( (s_ir_front < 12) && (s_ir_front > 1) ) ){ //Stopping condition				

				if(t_reflex > 26){ //If the robot has traveled more than 26 reflex segments, it is considered to have moved a tile forward
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
					distance_covered++;
				}

 
				first_time_on_island = 0; //Flag to prevent the robot to immediatly return to the outside wall upon arriving at an island
				curr_action = EMPTY; //The forward action is done

				if(land_o_hoy == 0){ //If in the traversing phase

					next_action = SPIN_L; 
					land_o_hoy = 1;
					first_time_on_island = 1;
					island_x = robot_pos_x; //Save the island entry coordinates
					island_y = robot_pos_y;

					if(follow_island == 1){ //Toggle follow_island on traverse
						follow_island = 0;
					}
					else{
						follow_island = 1;
					}					
				}

				if((s_ir_front < 14)&&(s_ir_front > 1)){ //Do not look for walls if the front sensor is too close to the wall
					setSpeed(0,0,FORWARD,FORWARD);
					action_done(DONTUPDATE);
				}
				else{
					action_done(UPDATE);
				}
			}
			
		break;

		//-----NUDGE FORWARD-----

		case(NUDGE_FORWARD):

			setSpeed(30,30,1,1);

			if(t_reflex > 3){
				curr_action = EMPTY;
				if(t_vagg_h_b != 2){ //Nudge forward until the front sensor is clear of the wall
					next_action = LAST_NUDGE;
				}
				else{
					next_action = NUDGE_FORWARD;
				}
				action_done(DONTUPDATE);
			}

		break;

		//-----NUDGE TO THE WALL-----

		case(NUDGE_TO_WALL):

			setSpeed(50,50,1,1);
			if(s_ir_front < 13){ //Nudge forward until in a good distance from the wall
				curr_action = EMPTY;
				action_done(UPDATE);
			}

		break;

		//-----LAST NUDGE-----

		case(LAST_NUDGE):
			setSpeed(30,30,1,1);
			if(t_reflex > 3){ //Do a last nudge to add margin to the turn
				curr_action = EMPTY;
				next_action = SPIN_R;
				action_done(UPDATE);
			}
		break;

		//----TURN RIGHT-----

		case (SPIN_R):

			if(first_time){
				first_time = 0;
				spinning = 1;
				setSpeed(70, 70, 1, 0);
				transmitByte_down(0x1C); //Set mimer to turning mode
			}

			if(t_gyro == 0x44){ //Done turning?
				dir++;
				first_time = 1;
				spinning = 0;
				transmitByte_down(0x1E); //Tell mimer to exit turning mode

				curr_action = FORWARD; //Force a forward action since the robot wants to turn right again.
				action_done(DONTUPDATE);

			}
		break;

		//-----TURN LEFT------

		case (SPIN_L):

			if(first_time){
				spinning = 1;
				first_time = 0;
				setSpeed(70, 70, 0, 1);
				transmitByte_down(0x1F); //Set mimer to turning mode
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

				if(land_o_hoy == 1){ //If it is in traverse phase, force forward
					curr_action = PARALLELIZE;
				}
				else{
					next_action = FORWARD;
				}

				action_done(DONTUPDATE);


			}
		break;

		//-----PARALLELIZE A BIT SLOPPY-----

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

		//-----PARALLELIZE A BIT SLOPPY TO THE LEFT WALL-----

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

		//-----PARALLELIZE TO THE RIGHT WALL------

		case(PARALLELIZE):
			if (t_p_h == 0){ //Parallellt

				parallell_cnt++;

				if(parallell_cnt >= 10){ // The robot is considered parallel if the parallel-token is 0 ten times in a row
					setSpeed(0, 0, FORWARD, FORWARD);
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

		//-----BACK-----

		case(BACKWARD):

			setSpeed(30, 30, 0, 0);
							
			if(s_ir_front >= 11){ //Back until the robot stands at least 11 cm from the wall in front of it
				curr_action = EMPTY;
				action_done(UPDATE);
			}
		break;
	}


	return;
}

void action_done(uint8_t update_map){

	debug = map_enclosed;

	if(dir>3){ //Calculate dir mod 4
		dir -=4;
	}

	parallell_cnt = 0; 
	old_deviation_from_wall = 0;
	derivata = 0;
	transmitByte_down(0x21); //Reset reflex-segments
	receiveByte_down(); //Wait for the reset to be done

	if((next_action != 0) && (curr_action == 0)){ //If there is an action in queue, put it as current action
		curr_action = next_action;
		next_action = 0;
	}

	//if(old_action == !FORWARD){
		setSpeed(0,0,FORWARD,FORWARD);
		_delay_ms(100); //Allow the robot to stop
	//}	
	int8_t temp_x = 0;
	int8_t temp_y = 0;

	if(update_map == 1){ //Calculate X and Y offset ussed to update the map
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

		wmem_auto(FLOOR, robot_pos_x, robot_pos_y); //Force a floor where the robot is

		if ((t_vagg_h_f == 0) && (t_vagg_h_b == 0)){ //If there is no wall to the right of the robot
			wmem_auto(FLOOR, robot_pos_x - temp_y  , robot_pos_y + temp_x); //Add a floor to the right of the robot
			wmem_auto(FLOOR, robot_pos_x - temp_y*2 , robot_pos_y + temp_x * 2); //Add a floor 1 tile from the robot's right
		}

		if ((t_vagg_h_f == 1) && (t_vagg_h_b == 1)){ //If there is a wall 2 tiles to the right of the robot
			if(map_enclosed == 1){			
				wmem_auto(WALL, robot_pos_x - temp_y * 2, robot_pos_y + temp_x * 2);  //Add floor + wall to the right
			}
			else{		//OWALL
				wmem_auto(OWALL, robot_pos_x - temp_y * 2, robot_pos_y + temp_x * 2);  //Add floor + owall to the right1
			}
			wmem_auto(FLOOR, robot_pos_x - temp_y, robot_pos_y + temp_x); 
		}

		if ((t_vagg_h_f == 2) && (t_vagg_h_b == 2)){ //If there is a wall directly to the right of the robot
			if(map_enclosed == 1){
				wmem_auto(WALL, robot_pos_x - temp_y , robot_pos_y + temp_x); //add a wall there
			}
			else{		//OWALL
				wmem_auto(OWALL, robot_pos_x - temp_y , robot_pos_y + temp_x); //add a wall there
			}
		}	

		if ((t_vagg_v_f == 0) && (t_vagg_v_b == 0)){  //Same as abobe, but left side
			wmem_auto(FLOOR, robot_pos_x + temp_y * 2 , robot_pos_y - temp_x * 2); 
			wmem_auto(FLOOR, robot_pos_x + temp_y , robot_pos_y - temp_x); 
		}

		if ((t_vagg_v_f == 1) && (t_vagg_v_b == 1)){
			if(map_enclosed == 0){
				wmem_auto(IWALL, robot_pos_x + temp_y * 2, robot_pos_y - temp_x * 2); 				
			}
			wmem_auto(FLOOR, robot_pos_x + temp_y, robot_pos_y - temp_x); 
		}

		if ((t_vagg_v_f == 2) && (t_vagg_v_b == 2)){
			if(map_enclosed == 0){ 
				wmem_auto(IWALL, robot_pos_x + temp_y , robot_pos_y - temp_x); 
			}
		}		
	}
}

void setSpeed(uint8_t lspeed, uint8_t rspeed, uint8_t ldir , uint8_t rdir){
	
		if(ldir){ //Insert a bit in PORTA bit 7
			PORTA |= (1 << DDA7);			
		}
		else{			
			PORTA &= 0x7F;
		}

		if(rdir){ //Insert a bit in PORTA bit 6
			PORTA |= (1 << DDA6);
		}
		else{			
			PORTA &= 0xBF;
		}

	uint16_t rdone = rspeed * MAX_SPEED_R; //Multiply the regulated value with the max speed multiplier
	uint16_t ldone = lspeed * MAX_SPEED_L;

	if (rdone > 28){ // super magic number for driving straight
		rdone -= 28;
	}

	if(rdone > 1000){ //avoid overflow
		rdone = 1000;
	}
	if(ldone > 1000){
		ldone = 1000;
	}

	OCR1A = rdone;//set the duty cycle(out of 1023) right	(pin 19)
	OCR3A = ldone;//set the duty cycle(out of 1023) left (pin 7)
	
}