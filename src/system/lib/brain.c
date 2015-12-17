/*
 * Created: November 2015
 * Author : Mikael Å, Anton R, Michael S
 * "It should be working now" -Peter
 *
 * The brain of the robot, using sensory data to make a desicion on which action to take now. It also makes sense of the map-data, using it to find 
 * islands to plunder. Yar har!
 */ 
 
#include "brain.h"
#include "mem.h"
#include <avr/io.h>

//-----PROTOTYPES-----
void find_next_wall();
void find_empty_tile();
uint8_t dfs(uint8_t startx, uint8_t starty);
uint8_t dfs_help(uint8_t startx, uint8_t starty);
void gen_actions();
void mark_walls();
uint8_t done_unexp();
uint8_t done_iwall();
void purge_iwalls();


uint8_t follow_wall = 1;
uint8_t startup = 1;

//För DFS
uint8_t check = 0;
uint8_t visited[32][32];
uint8_t first_time_on_island = 0;

/*
 * Generate next action
 */
void think(){
	if((curr_action == EMPTY) && (map_complete == 0)){	

		if(follow_wall == 1){ //If in follow wall mode

			if((s_ir_front < 11) && (s_ir_front > 2)){ //If to close to the wall, back up a bit
					curr_action = BACKWARD;
			}
		
			else if(((t_vagg_h_f == 0) && (t_vagg_h_b == 0)) || ((t_vagg_h_f == 1) && (t_vagg_h_b == 1))){ //If there is no wall to the right of the robot
					curr_action = SPIN_R;
				
				if((s_ir_front > 12) && (s_ir_front < 30)){  //If we're close to wall, nudge to it.
					curr_action = NUDGE_TO_WALL;
				}

				else if((t_vagg_v_f == 1) && (t_vagg_v_b == 1)){	// parallelize against a wall far to the left
					if(t_p_v != 0){
						curr_action = P_WEAK_L;
					}
				}

			}

			else if( (t_vagg_h_f != 2) && (t_vagg_h_b == 2) ){ //If there is no wall to the right of the robot
				curr_action = NUDGE_FORWARD;
			}

			else if(t_vagg_front == 2){ //If the robot has a wall right in front of it, turn where there is an empty tile, right is prefered
				if(t_p_h != 0){
					curr_action = P_WEAK;
				}
				else{

					if((s_ir_front > 13) && (s_ir_front < 30)){
						curr_action = NUDGE_TO_WALL;
					}

					else{ //If there is a wall front and right, turn left
						curr_action = SPIN_L;
					}
				}
			}
			else{
				curr_action = FORWARD;
			}

			if(map_enclosed == 1){	//outer wall is connected, enclosing the inner area.

				uint8_t temp_x, temp_y;

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

				if((lets_go_home == 0) && (follow_island == 0)){
					if ((rmem(robot_pos_x + temp_y, robot_pos_y - temp_x) == IWALL) || 
						(rmem(robot_pos_x + temp_y * 2, robot_pos_y - temp_x * 2) == IWALL)){ 
							land_o_hoy = 0;
							first_time_on_island = 1;
							curr_action = PARALLELIZE;
							next_action = SPIN_L;
					}
				}
			}
		} 
		think_hard();
	}	

	return;
}



void think_hard(){

	if(map_enclosed == 0){ // Depth First Search to determine if the outer wall is closed
		if(dfs(robot_pos_x, robot_pos_y) == 0){
			enqueue(0xFF, 0xFF, 0xFF);		//Send special command to tell CRAY to rescale its map
			map_enclosed = 1;
			mark_walls();					//Turns all squares outside of the enclosed area to walls
		}
	}

	purge_iwalls();

	if((done_unexp() == 1) || (done_iwall() == 1)){		//If there are no unexplored tiles or no inner walls left
		if(map_enclosed == 1){							//Is the area enclosed as well
			lets_go_home = 1;							//We are finished mapping, let's return to start
		}
	}

	if(follow_island == 1){													
		if((first_time_on_island == 0) || lets_go_home){

			if((robot_pos_x == island_x) && (robot_pos_y == island_y)){
				land_o_hoy = 0;												
				curr_action = PARALLELIZE;
				next_action = SPIN_L;
			}
		}
	}

	if((lets_go_home == 1) && (robot_pos_x == home_x) && (robot_pos_y == home_y)){		//Is system done?
		enqueue(0xEE, 0xEE, 0xEE);														//Special command to tell CRAY that we're done
		setSpeed(0,0,FORWARD,FORWARD);													//Stop motors
		map_complete = 1;
	}		
}


typedef struct tuple tuple;

struct tuple{
		uint8_t x;
		uint8_t y;
};



/*
 * Returns 0 if there are any UNEXP tiles on the map.
 * Otherwise returns 1.
 */
uint8_t done_unexp(){

	uint8_t answer = 1;
	uint8_t i;
	uint8_t j;

	for(i = 0; i < 32; i++){
		for(j = 0; j < 32; j++){
			if(rmem(i,j) == UNEXP){
				answer = 0;
			}			
		}
	}
	return answer;
}

/*
 * Returns 0 if there are any IWALL tiles on the map.
 * Otherwise returns 0.
 */
uint8_t done_iwall(){

	uint8_t answer = 1;
	uint8_t i;
	uint8_t j;

	for(i = 0; i < 32; i++){
		for(j = 0; j < 32; j++){
			if(rmem(i,j) == IWALL){
				answer = 0;
			}			
		}
	}
	return answer;
}

/*
 * Marks all unvisited squares as wall.
 * This is used for marking all tiles outside of the enclosed area as wall.
 */
void mark_walls(){

	uint8_t i;
	uint8_t j;

	for(i = 0; i < 32; i++){
		for(j = 0; j < 32; j++){
			if(visited[i][j] == 0){
				wmem(WALL,i,j);
			}			
		}
	}	
}


void purge_iwalls(){

	for(i = 0; i < 32; i++){
		for(j = 0; j < 32; j++){


			if(rmem(i,j) == IWALL){

				uint8_t alone_iwall = 1;

				if(rmem(i+1, j) == UNEXP){
					alone_iwall = 0;
				}
				if(rmem(i-1, j) == UNEXP){
					alone_iwall = 0;
				}
				if(rmem(i, j+1) == UNEXP){
					alone_iwall = 0;
				}
				if(rmem(i, j-1) == UNEXP){
					alone_iwall = 0;
				}

				if(alone_iwall == 1){
					wmem_auto(WALL,i,j);
				}

			}
		}
	}
}


/*
 * Depth First Search that tries to find a way out of the enclosed area to the outer line of OUTSIDE. 
 * If this can be done, the area is not closed.
 * Returns 1 if area is NOT closed.
 * Returns 0 if area is closed.
 */
uint8_t dfs(uint8_t startx, uint8_t starty){
	uint8_t i;
	uint8_t j;
	for(i = 0; i < 32; i++){
		for(j = 0; j < 32; j++){
			visited[i][j] = 0;
		}
	}
	return dfs_help(startx, starty);
}

/*
 * Recursive help function for DFS. 
 * Returns 1 if area is NOT closed.
 * Returns 0 if area is closed.
 */
uint8_t dfs_help(uint8_t startx, uint8_t starty){
	if(visited[startx][starty] == 0){
		if(rmem(startx,starty) == OUTSIDE){
			return 1;
		}
		if(rmem(startx,starty) == WALL){
			return 0;
		}
		visited[startx][starty] = 1;
		
		if(dfs_help(startx,starty-1) == 1){
			return 1;
		}
		if(dfs_help(startx+1,starty) == 1){
			return 1;
		}
		if(dfs_help(startx,starty+1) == 1){
			return 1;
		}
		if(dfs_help(startx-1,starty) == 1){
			return 1;
		}
	}
	return 0;
}

