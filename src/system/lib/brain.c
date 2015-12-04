#include "brain.h"
#include "mem.h"
#include <avr/io.h>

void find_next_wall();
void find_empty_tile();

void bfs();
void bfs_u();
uint8_t dfs(uint8_t startx, uint8_t starty, uint8_t target_tile);
uint8_t dfs_help(uint8_t startx, uint8_t starty, uint8_t target_tile);
void gen_adj_matrix(uint8_t home);
void gen_actions();

uint8_t adj_matrix[32][32];

uint8_t follow_wall = 1;
uint8_t startup = 1;
//För DFS
uint8_t check = 0;
//Är detta ok utan initialisering?


void think(){


	if(read_a_top() == 0){

		if(follow_wall){ //Om vi ska följa högerväggen

			if((t_vagg_h_f == 0) || (t_vagg_h_f == 2)){ //If there is no wall to the right of the robot
				paction(FORWARD);
				paction(SPIN_R);
			}
			else if(t_LIDAR == 0){ //If the robot has a wall right in front of it, turn where there is an empty tile, right is prefered
				
				if((t_vagg_v_f == 0) || (t_vagg_v_f == 2)){ //Turn left
					paction(FORWARD);
					paction(SPIN_L);
				}
				else{ //If there is a wall both left and right, turn 180 deg
					paction(FORWARD);
					paction(SPIN_180);
				}

			}
			else{
				paction(FORWARD);
			}

			//Kolla om väggen är sluten

		}
		else{ //Om vi ska kartlägga mitten

			//Hitta en tile att åka till

		}

	}

	//Om actionstacken är tom, räkna ut nästa target
	//Fyll på actionstacken

	return;
}

void find_next_wall(){

	if(startup){

		paction(PARALLELIZE);
		startup = 0;
	}
	else{
		/*	uint8_t UNEXP 		= 1;	//Tile: Unexplored
		uint8_t FLOOR 		= 2;	//Tile: Floor
		uint8_t WALL 		= 3;	//Tile: Wall
		uint8_t OUTSIDE 	= 4;	//Tile: Outside
		#define NORTH	0
		#define	EAST	1
		#define	SOUTH	2
		#define	WEST	3

		uint8_t robot_pos_x;	// Start in the middle of the map
		uint8_t robot_pos_y;

		#define EMPTY		0 //The a_stack was empty //Stand still
#define FORWARD 	1 //Go forward 1 tile
#define	SPIN_R		2 //Turn 90 right
#define SPIN_L		3 //Turn 90 left
#define	SPIN_180 	4 //Turn 180 left
#define PARALLELIZE	5 //Turn until parallel with wall on right side
#define	BACKWARD	6 //Back-up one tile

		*/
		switch(dir){
		case (0): //LIDAR to the NORTH
			if (rmem(robot_pos_x + 1, robot_pos_y)->tileType == WALL){ //Vägg till öster
				if (rmem(robot_pos_x, robot_pos_y - 1)->tileType == WALL) { // vägg till norr
					if (rmem(robot_pos_x -1 , robot_pos_y)->tileType == WALL){ // vägg till väst
						paction(SPIN_180);
						paction(FORWARD);
					}
					else{
					paction(SPIN_L);
					paction(FORWARD);
					}
				}
				else{
				paction(FORWARD); //Kör framåt
				}
			}
			else if (rmem(robot_pos_x + 1, robot_pos_y)->tileType == FLOOR){ //Öppet till öster
				paction(SPIN_R);
				paction(FORWARD);//Snurra 90 grader och åk framåt
			}

		break;
		case (1): //LIDAR to the EAST
			if (rmem(robot_pos_x, robot_pos_y + 1)->tileType == WALL){ //Vägg till söder
				if (rmem(robot_pos_x + 1, robot_pos_y)->tileType == WALL) { // vägg till öster
					if (rmem(robot_pos_x, robot_pos_y - 1)->tileType == WALL){ // vägg till norr
						paction(SPIN_180);
						paction(FORWARD);
					}
					else{
					paction(SPIN_L);
					paction(FORWARD);
					}
				}
				else{
				paction(FORWARD); //Kör framåt
				}
			}
			else if (rmem(robot_pos_x, robot_pos_y + 1)->tileType == FLOOR){ //Öppet till söder
				paction(SPIN_R);
				paction(FORWARD);//Snurra 90 grader och åk framåt
			}
		break;
		case (2): //LIDAR to the SOUTH
			if (rmem(robot_pos_x - 1, robot_pos_y)->tileType == WALL){ //Vägg till väster
				if (rmem(robot_pos_x, robot_pos_y + 1)->tileType == WALL){ // vägg till south
					if (rmem(robot_pos_x + 1, robot_pos_y)->tileType == WALL){ // vägg till öster
						paction(SPIN_180);
						paction(FORWARD);
					}
					else{
					paction(SPIN_L);
					paction(FORWARD);
					}
				}
				else{
				paction(FORWARD); //Kör framåt
				}
			}
			else if (rmem(robot_pos_x - 1, robot_pos_y)->tileType == FLOOR){ //Öppet till väster
				paction(SPIN_R);
				paction(FORWARD);//Snurra 90 grader och åk framåt
			}
		break;
		case (3): //LIDAR to the WEST
			if (rmem(robot_pos_x, robot_pos_y - 1)->tileType == WALL){ //Vägg till norr
				if (rmem(robot_pos_x - 1 , robot_pos_y)->tileType == WALL){ // vägg till väster
					if (rmem(robot_pos_x, robot_pos_y + 1)->tileType == WALL){ // vägg till söder
						paction(SPIN_180);
						paction(FORWARD);
					}
					else{
					paction(SPIN_L);
					paction(FORWARD);
					}
				}
				else{
				paction(FORWARD); //Kör framåt
				}
			}
			else if (rmem(robot_pos_x, robot_pos_y - 1)->tileType == FLOOR){ //Öppet till norr
				paction(SPIN_R);
				paction(FORWARD);//Snurra 90 grader och åk framåt
			}
		break;

		}
	}


//Kod för att hitta nästa target, en tile vi kan åka på
	//jämte nästa troliga vägg (så vi kan se om den finns där)
	return;
}



void bfs(){
//Används när vi vill hitta närmsta vägen till känd position
	gen_adj_matrix(0); //Generates the adjacency matrix and sets target_x and y
	gen_actions();	  //to the closest undiscovered tile.
	return;
}

void bfs_u(){
//Används när vi vill hitta närmsta vägen till en UNEXP-tile

	return;
}

typedef struct tuple tuple;

struct tuple{
		uint8_t x;
		uint8_t y;
};

void gen_actions(){

	uint8_t temp_x = target_x;
	uint8_t temp_y = target_y;
	uint8_t next_val = 255;
	uint8_t next_dir = 0;
	uint8_t old_dir = 0;
	uint8_t old_x = target_x;
	uint8_t old_y = target_y;
	uint8_t next_x = 0;
	uint8_t next_y = 0;

	while((temp_x != robot_pos_x) && (temp_y != robot_pos_y)){ //While not at robot position

		if((temp_x != target_x) && (temp_y != target_y)){ //Decide the initial dir of the robot

			if(adj_matrix[temp_x-1][temp_y] < next_val){ //Left of target
				next_dir = WEST;
				next_val = adj_matrix[temp_x-1][temp_y];
			}
			else if(adj_matrix[temp_x+1][temp_y] < next_val){ //Right of target
				next_dir = EAST;
				next_val = adj_matrix[temp_x+1][temp_y];
			}
			else if(adj_matrix[temp_x][temp_y-1] < next_val){ //north of target
				next_dir = NORTH;
				next_val = adj_matrix[temp_x][temp_y-1];
			}
			else{ //south of target
				next_dir = SOUTH; 
				next_val = adj_matrix[temp_x][temp_y+1];
			}

			//Now the dir of the next tile has been decided
			if(old_x > temp_x){ 
				old_dir = EAST;
			}
			else if(old_x < temp_x){
				old_dir = WEST;
			}
			else{ //old_x == temp_x => vi rör oss i y-led
				if(old_y > temp_y){
					old_dir = SOUTH;
				}
				else{
					old_dir = NORTH;
				}
			}
			//Båda directions är bestämda

			if(old_y == SOUTH && next_x == WEST){
				paction(SPIN_R);
			}
			else if(old_y == SOUTH && next_x == EAST){
				paction(SPIN_L);
			}
			else if(old_y == WEST && next_x == NORTH){
				paction(SPIN_R);
			}
			else if(old_y == WEST && next_x == SOUTH){
				paction(SPIN_L);
			}
			else if(old_y == NORTH && next_x == EAST){
				paction(SPIN_R);
			}
			else if(old_y == NORTH && next_x == WEST){
				paction(SPIN_L);
			}
			else if(old_y == EAST && next_x == SOUTH){
				paction(SPIN_R);
			}
			else if(old_y == EAST && next_x == NORTH){
				paction(SPIN_L);
			}
		}
		paction(FORWARD); //The first (last) action is always FORWARD;

		old_x = temp_x;
		old_y = temp_y;

		temp_x = next_x;
		temp_y = next_y;
	}
}

void gen_adj_matrix(uint8_t home){

	uint8_t i;
	uint8_t j;
	uint8_t list_front = 0;
	tuple adj_list[99];

	for(i = 0; i < 0; i++){
		for(j = 0; j < 0; j++){

			uint8_t temp = rmem(i,j)->tileType;

			if((temp == FLOOR) || (temp == UNEXP)){
				tuple temptup = {.x = i, .y = j};
				adj_list[list_front] = temptup;
				list_front +=1;

			}
		}
	}

	uint8_t target_found = 0;

	while(!target_found){

		int i;
		for(i = 0; i < list_front; i++){
			if(adj_list[i].x != 0){
				uint8_t tempx = adj_list[i].x;
				uint8_t tempy = adj_list[i].y;

				uint8_t checkL = adj_matrix[tempx-1][tempy]; //Chack left
				uint8_t checkR = adj_matrix[tempx+1][tempy]; //Check right
				uint8_t checkU = adj_matrix[tempx][tempy-1]; //Check up
				uint8_t checkD = adj_matrix[tempx][tempy+1]; //Check down

				uint8_t lowestN = checkL;

				if((checkL != 254)||(checkR != 254)||(checkU != 254)||(checkD != 254)){

					if((checkR != 254) && (checkR < lowestN)){
						lowestN = checkR;
					}
					if((checkU != 254) && (checkU < lowestN)){
						lowestN = checkU;
					}
					if((checkD != 254) && (checkD < lowestN)){
						lowestN = checkR;
					}
					adj_matrix[tempx][tempy] = (lowestN +1);

					if((rmem(tempx, tempy)->tileType == UNEXP) && !home){
						//target_x = tempx;
						//target_y = tempy;
						target_found = 1;

					}
					else if(home && (tempx = home_x) && (tempy = home_y)){
						
						target_found = 1;

					}


					adj_list[i].x = 0;
					adj_list[i].y = 0;
				}
			}
		}
	}

return;

}



uint8_t visited[32][32];

uint8_t dfs(uint8_t startx, uint8_t starty, uint8_t target_tile){
//Används för att kolla om väggen runt är sluten.
	//Return 1 om sökning lyckades, 0 om ingen väg finns

	/*	uint8_t UNEXP 		= 1;	//Tile: Unexplored
		uint8_t FLOOR 		= 2;	//Tile: Floor
		uint8_t WALL 		= 3;	//Tile: Wall
		uint8_t OUTSIDE 	= 4;	//Tile: Outside*/


	uint8_t temp = 0;
	temp = dfs_help(startx, starty, target_tile);


	int i;
	int j;
	for(i = 0; i < 32; i++){
		for(j = 0; j < 32; j++){
			visited[i][j] = 0;
		}
	}

	return temp;


}

uint8_t dfs_help(uint8_t startx, uint8_t starty, uint8_t target_tile){ //Kanske behöver göras iterativt
	if(visited[startx][starty] != 1){

		if (rmem(startx, starty)->tileType == target_tile){
			return 1;

		}
		else if(rmem(startx, starty)->tileType == WALL){
			return 0;
		}
		else{
			check = dfs_help(startx-1, starty, target_tile);
			if (check == 1){return 1;}
			check = dfs_help(startx, starty+1, target_tile);
			if (check == 1){return 1;}
			check = dfs_help(startx+1, starty, target_tile);
			if (check == 1){return 1;}
			check = dfs_help(startx, starty-1, target_tile);
			if (check == 1){return 1;}
			return 0;
		}
		visited[startx][starty] = 1;
	}
	return 0;

}
