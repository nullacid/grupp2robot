
#include "mem.h"


//----------------Variables------------------------------------
node mapmem[32][32];		//The map memory 33x33 tiles large
mapchange change_stack[99];	//A stack where changes to the map will be waiting to be sent
int8_t a_stack_top = -1;

uint8_t action_s[50]; //The actions required to get to target

//-------------------------------------------------------------


//----------------Constants------------------------------------
uint8_t UNEXP 		= 0;	//Tile: Unexplored
uint8_t FLOOR 		= 2;	//Tile: Floor
uint8_t WALL 		= 3;	//Tile: Wall
uint8_t OUTSIDE 	= 4;	//Tile: Outside
uint8_t IWALL		= 1;
uint8_t C_STACK_MAX = 100;  //Size of stack
uint8_t C_QUEUE_MAX = 100;

//-------------------------------------------------------------

void init_mem(){

	robot_pos_x = 16;	// Start in the middle of the map
	robot_pos_y = 16;

	follow_island = 0;
	lets_go_home = 0;
	target_x = 0; //Target tile
	target_y = 0;
	dir = 0;
	c_stack_top = -1;
	debug = 0;
	//OKÄND SKA VARA 1, fixa
	land_o_hoy = 1;
	map_enclosed = 0;
	next_action = 0;

	home_x = 16;
	home_y = 16;

	map_complete = 0;
	first_time_on_island = 0;


	uint8_t i = 0;
	for(i = 0; i < 32; i++){

		wmem(OUTSIDE, i, 0);
		wmem(OUTSIDE, i, 32);
		wmem(OUTSIDE, 0, i);
		wmem(OUTSIDE, 32, i);

	}

	mapchange temp = {.x = 0, .y = 0, .t = 0}; 

	for(i = 0; i < 100; i++){
		changeQ.inbox[i] = temp;
		changeQ.outbox[i] = temp;

	}

	changeQ.sizeofIn = -1;
	changeQ.sizeofOut = 0;

	return;

}

int8_t enqueue(uint8_t x, uint8_t y, uint8_t t){
	if(changeQ.sizeofIn == (C_QUEUE_MAX-1)){
		return -1; //Return false, since the queue is full
	}
	else{
		mapchange temp = {.x = x, .y = y, .t = t}; 
		changeQ.sizeofIn++;
		changeQ.inbox[changeQ.sizeofIn] = temp;
	}
}

mapchange dequeue(){


	if(changeQ.sizeofOut == 0){
		while(!changeQ.sizeofIn == 0){
			//pop value from inbox stack
			changeQ.sizeofIn--;
			mapchange toBeMoved = changeQ.inbox[changeQ.sizeofIn];

			//add to outbox stack
			changeQ.sizeofOut++;
			changeQ.outbox[changeQ.sizeofOut] = toBeMoved;
		}
	}

	changeQ.sizeofOut--;
	mapchange data = changeQ.outbox[changeQ.sizeofOut];
	return data;
}


uint8_t pstack(uint8_t x, uint8_t y, uint8_t t){


	if(c_stack_top == (C_STACK_MAX-1)){

		return 0; //Return false, since the stack is full

	}
	else{

		mapchange temp = {.x = x, .y = y, .t = t}; 
		c_stack_top += 1;
		change_stack[c_stack_top] = temp;
	}
	return 1;

}

mapchange gstack(){

	if (c_stack_top == -1){

		mapchange data = {.x = 0, .y = 0, .t = 0}; 
		return data; //Return false, since the stack is empty

	}
	else{

		mapchange data = change_stack[c_stack_top];
		c_stack_top -= 1;

		return data;
	}
}

uint8_t rmem(uint8_t x, uint8_t y){

	return mapmem[x][y].tileType;

}

void wmem(uint8_t data, uint8_t x, uint8_t y){

	mapmem[x][y].tileType = data;
	
	return;
}

void wmem_auto(uint8_t data, uint8_t x, uint8_t y){
	//Denna kallar auto på, lägger till i change-stacken om ny data
	if ((mapmem[x][y].tileType != data) && (mapmem[x][y].tileType != WALL)){ //NY DATA, ska skickas upp
		mapmem[x][y].tileType = data;
		//pstack(x, y, data);
		enqueue(x, y, data);
	}


	return;
}

