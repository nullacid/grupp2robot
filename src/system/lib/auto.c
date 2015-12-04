#include "auto.h"
#include "usart.h"
#include "mem.h"
#include <avr/io.h>
#include <util/delay.h>


#define MAX_SPEED_R 6 //1 to 10, 10 is highest
#define MAX_SPEED_L 6
#define PERFECT_DIST 	10 	//12

#define GYRO_NO_TURNING 0xB5 //KOMMER NOG ÄNDRAS
#define FOLlOW_WALL 0
#define MAP_REST 1


uint8_t cur_action = 0;
int distance_LIDAR;
uint8_t first_time;
uint8_t NODBROMS = 0;
uint8_t parallell_cnt = 0;

int16_t deviation_from_wall = 0;
int16_t old_deviation_from_wall = 0;
int16_t derivata = 0;
int16_t P = 0;
int16_t D = 0;
uint8_t pidk = 10; //10 är okej
uint8_t pidd = 35; //35 ok ish
uint8_t lidar_start = 0;

void update_sensor_data(); 
void init_auto();
void action_done();


void init_auto(){
	distance_LIDAR = 0;
	first_time = 1;
	s_LIDAR_u = 0;
	s_LIDAR_l = 0;
	s_LIDAR = 0;
	t_LIDAR = 0;	//Antal 40 cm rutor till vägg 0 - 20 cm   1 - 20+40cm   2 20+80cm

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



	paction(PARALLELIZE);

	uint8_t current_state = 0; //0 - start, 1 - stå still, 2 - köra, 3 - snurra
	//----------------------------
}
void update_sensor_data(){
	//från 08 ---> 1 7 rader
	transmitByte_down(0x1D); //fråga efter all data

	s_LIDAR_u = receiveByte_down();
	s_LIDAR_l = receiveByte_down();
	s_ir_h_f = receiveByte_down();
	s_ir_h_b = receiveByte_down();
	s_ir_v_f = receiveByte_down();
	s_ir_v_b = receiveByte_down();
	s_gyro_u = receiveByte_down();
	s_gyro_l = receiveByte_down();

	t_LIDAR = receiveByte_down();
	t_p_h = receiveByte_down();;
	t_p_v = receiveByte_down();
	t_gyro = receiveByte_down();
	t_vagg_h_f = receiveByte_down();
	t_vagg_h_b = receiveByte_down();
	t_vagg_v_f = receiveByte_down();
	t_vagg_v_b = receiveByte_down();

	s_LIDAR = ((s_LIDAR_u << 8) + s_LIDAR_l);
	s_gyro = ((s_gyro_u << 8) + s_gyro_l);

	return;
}

void autonom (){

				debug = spinning;


	if ((s_LIDAR_u == 0) && (s_LIDAR_l < 7)){
		NODBROMS = 1;
		setSpeed(0,0,1,1);

	}
	else{
		NODBROMS = 0;
	}



	if(cur_action == EMPTY){ //Om vi inte har en order, kolla om det finns någon ny

		cur_action = read_a_top();
	}
	
	if(!NODBROMS){
		switch(cur_action){
			case (EMPTY):
				setSpeed(0, 0, 0, 0);
				action_done();
			break;

			case (FORWARD):
				if (first_time){
					distance_LIDAR = s_LIDAR - 40; //LIDAR distance - 40 cm
					lidar_start = t_LIDAR;
					first_time = 0;
				}

				if(t_vagg_h_f){ //Om det finns en vägg höger fram, reglera efter den

					int8_t control = 0;
					uint8_t lspeed = 0;
					uint8_t rspeed = 0;
					deviation_from_wall = (s_ir_h_f - PERFECT_DIST);
					derivata = (deviation_from_wall - old_deviation_from_wall);

					P = pidk * deviation_from_wall;
					D = pidd * derivata;
					control = P+D;

					if(control > 0){
						rspeed = 100 - control;
						lspeed = 100;
					}
					else if(control < 0){
						//nära höger vägg

						lspeed = 100 + control;
						rspeed = 100;
					}
					else{
						lspeed = 100;
						rspeed = 100;
					}
					if(deviation_from_wall > 20){
						rspeed = 0;
						lspeed = 100;
					}
					else if(deviation_from_wall < -7){
						lspeed = 0;
						rspeed = 100;
					}

					old_deviation_from_wall = deviation_from_wall;
					motor_r = rspeed;
					motor_l = lspeed;
					setSpeed(lspeed , rspeed,1,1);
				}
				else{
					setSpeed(100,100,1,1);
				}
				
				if (t_LIDAR <  lidar_start) {
					first_time = 1;

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

					action_done();
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
					first_time = 1;
					spinning = 0;
					dir += 1;
					transmitByte_down(0x1E);
					action_done();
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
						dir -=1;
					}			
					spinning = 0;
					first_time = 1;	
					transmitByte_down(0x1E);
					action_done();
				}
			break;

			case (SPIN_180):
				
				if(first_time){
					spinning = 1;
					first_time = 0;
					setSpeed(70, 70, 1, 0); //Höger hjulpar bakåt
					transmitByte_down(0x20);
				}

				if(t_gyro == 0x44){
					first_time = 1;
					spinning = 0;
					dir += 1;
					transmitByte_down(0x1E);
					action_done();
				}

			break;
	//------------------------------------------------------------------------
			case(PARALLELIZE):
				if (t_p_h == 0){ //Parallellt

					parallell_cnt++;

					if(parallell_cnt == 10){
						setSpeed(0, 0, 1, 1);
						_delay_ms(100);
						dir = NORTH;

						action_done();
					}
				}	
				else if(t_p_h == 127){
					setSpeed(50,50,0,1);
					parallell_cnt = 0;
				}
				else if (t_p_h > 0){ //påväg från väggen
					setSpeed(30 * t_p_h, 30 * t_p_h, 1, 0); //Speed till 40 eller 80 beroende på hur fel vi är
					parallell_cnt = 0;
				}	
				else if (t_p_h < 0){ //påväg in i väggen
					setSpeed(30 * (-t_p_h), 30 * (-t_p_h), 0, 1); //Speed till 40 eller 80 beroende på hur fel vi är
					parallell_cnt = 0;
				}
				
			break;

			case(BACKWARD):
				if (first_time){
					distance_LIDAR = s_LIDAR_u*256 + s_LIDAR_l + 40; //LIDAR distance + 40 cm
					first_time = 0;
				}
				//Kolla så vi åker typ parallellt

				setSpeed(100, 100, 0, 0);
				if ((s_LIDAR_u*256 + s_LIDAR_l) >= distance_LIDAR) {
					first_time = 1;
					action_done();
				}
			break;
		}
	}

	return;
}

void action_done(){

	if(dir>3){
		dir -=4;
	}


	uint8_t old_action = cur_action;
	pop_a_stack(); //Ta bort actionen från actionstacken
	cur_action = read_a_top();	//Ta in actionen under
	if (old_action != cur_action){ 
		setSpeed(0, 0, 1, 1); //Stanna om vi inte ska fortsätta i samma riktning
		_delay_ms(200);
	}
	//------------UPPDATERA KARTDATA ----------------
	//Räknat med 0,0 i övre högra hörnet
	parallell_cnt = 0;

	switch(dir){
		int i;
		case (0): //LIDAR to the NORTH
			wmem_auto(WALL, robot_pos_x, robot_pos_y - t_LIDAR); //LIDAR WALL
			for (i = 1; i < t_LIDAR; i++){ //LIDAR SEES FLOOR
				wmem_auto(FLOOR, robot_pos_x, robot_pos_y - i);
			}

			if (t_vagg_h_f + t_vagg_h_b == 4){ //HÖGER IR WALL + 1 FLOOR
				wmem_auto(WALL, robot_pos_x + 2, robot_pos_y); 
				wmem_auto(FLOOR, robot_pos_x + 1, robot_pos_y);
			}

			if (t_vagg_h_f + t_vagg_h_b == 2){ //HÖGER IR WALL
				wmem_auto(WALL, robot_pos_x + 1, robot_pos_y); 
			}

			if (t_vagg_v_f + t_vagg_v_b == 4){ //VÄNSTER IR WALL + 1 FLOOR
				wmem_auto(WALL, robot_pos_x - 2, robot_pos_y); 
				wmem_auto(FLOOR, robot_pos_x - 1, robot_pos_y);
			}

			if (t_vagg_v_f + t_vagg_v_b == 2){ //VÄNSTER IR WALL
				wmem_auto(WALL, robot_pos_x - 1, robot_pos_y); 
			}
			


		break;
		case (1): //LIDAR to the EAST
			wmem_auto(WALL, robot_pos_x + t_LIDAR, robot_pos_y); //LIDAR WALL
			for (i = 1; i < t_LIDAR; i++){ //LIDAR SEES FLOOR
				wmem_auto(FLOOR, robot_pos_x + i, robot_pos_y);
			}

			if (t_vagg_h_f + t_vagg_h_b == 4){ //HÖGER IR WALL + 1 FLOOR
				wmem_auto(WALL, robot_pos_x, robot_pos_y+2); 
				wmem_auto(FLOOR, robot_pos_x, robot_pos_y+1);
			}

			if (t_vagg_h_f + t_vagg_h_b == 2){ //HÖGER IR WALL
				wmem_auto(WALL, robot_pos_x, robot_pos_y+1); 
			}

			if (t_vagg_v_f + t_vagg_v_b == 4){ //VÄNSTER IR WALL + 1 FLOOR
				wmem_auto(WALL, robot_pos_x, robot_pos_y-2); 
				wmem_auto(FLOOR, robot_pos_x, robot_pos_y-1);
			}

			if (t_vagg_v_f + t_vagg_v_b == 2){ //VÄNSTER IR WALL
				wmem_auto(WALL, robot_pos_x, robot_pos_y-1); 
			}



		break;
		case (2): //LIDAR to the SOUTH
			wmem_auto(WALL, robot_pos_x, robot_pos_y + t_LIDAR); //LIDAR WALL
			for (i = 1; i < t_LIDAR; i++){ //LIDAR SEES FLOOR
				wmem_auto(FLOOR, robot_pos_x, robot_pos_y + i);
			}

			if (t_vagg_h_f + t_vagg_h_b == 4){ //HÖGER IR WALL + 1 FLOOR
				wmem_auto(WALL, robot_pos_x - 2, robot_pos_y); 
				wmem_auto(FLOOR, robot_pos_x - 1, robot_pos_y);
			}

			if (t_vagg_h_f + t_vagg_h_b == 2){ //HÖGER IR WALL
				wmem_auto(WALL, robot_pos_x - 1, robot_pos_y); 
			}

			if (t_vagg_v_f + t_vagg_v_b == 4){ //VÄNSTER IR WALL + 1 FLOOR
				wmem_auto(WALL, robot_pos_x + 2, robot_pos_y); 
				wmem_auto(FLOOR, robot_pos_x + 1, robot_pos_y);
			}

			if (t_vagg_v_f + t_vagg_v_b == 2){ //VÄNSTER IR WALL
				wmem_auto(WALL, robot_pos_x + 1, robot_pos_y); 
			}


		break;
		case (3): //LIDAR to the WEST
			wmem_auto(WALL, robot_pos_x - t_LIDAR, robot_pos_y); //LIDAR WALL
			for (i = 1; i < t_LIDAR; i++){ //LIDAR SEES FLOOR
				wmem_auto(FLOOR, robot_pos_x - i, robot_pos_y);
			}

			if (t_vagg_h_f + t_vagg_h_b == 4){ //HÖGER IR WALL + 1 FLOOR
				wmem_auto(WALL, robot_pos_x, robot_pos_y-2); 
				wmem_auto(FLOOR, robot_pos_x, robot_pos_y-1);
			}

			if (t_vagg_h_f + t_vagg_h_b == 2){ //HÖGER IR WALL
				wmem_auto(WALL, robot_pos_x, robot_pos_y-1); 
			}

			if (t_vagg_v_f + t_vagg_v_b == 4){ //VÄNSTER IR WALL + 1 FLOOR
				wmem_auto(WALL, robot_pos_x, robot_pos_y+2); 
				wmem_auto(FLOOR, robot_pos_x, robot_pos_y+1);
			}

			if (t_vagg_v_f + t_vagg_v_b == 2){ //VÄNSTER IR WALL
				wmem_auto(WALL, robot_pos_x, robot_pos_y+1); 
			}

		break;
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

	if (rdone > 38){
		rdone -= 38;
	}

	OCR1A = rdone;//set the duty cycle(out of 1023) Höger	(pin 19)
	OCR3A = ldone;//set the duty cycle(out of 1023) Vänster (pin 7)
	
}