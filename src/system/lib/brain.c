#include "brain.h"
#include "mem.h"
#include <avr/io.h>

void find_next_wall();
void find_empty_tile();

void bfs(uint8_t startx, uint8_t starty, uint8_t targetx, uint8_t targety);
void bfs_t(uint8_t startx, uint8_t starty, uint8_t target_tile);
uint8_t dfs(uint8_t startx, uint8_t starty, uint8_t target_tile);
uint8_t dfs_help(uint8_t startx, uint8_t starty, uint8_t target_tile);

uint8_t follow_wall = 1;
uint8_t startup = 1;
//För DFS
uint8_t check = 0;
//Är detta ok utan initialisering?


void think(){


	if(read_a_top() == 0){

		if(follow_wall){ //Om vi ska följa högerväggen

			find_next_wall();

		}
		else{ //Om vi ska kartlägga mitten

			find_empty_tile();

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

void find_empty_tile(){


//Hitta en unexplored tile vi kan åka til
	bfs_t(robot_pos_x, robot_pos_y, UNEXP);

	return;
}

void bfs(uint8_t startx, uint8_t starty, uint8_t targetx, uint8_t targety){
//Används när vi vill hitta närmsta vägen till känd position

	return;
}

void bfs_t(uint8_t startx, uint8_t starty, uint8_t target_tile){
//Används när vi vill hitta närmsta vägen till en speciell tile-typ

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