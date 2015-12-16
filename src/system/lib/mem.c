/*
 * Created: November 2015
 * Author : Mikael Å, Anton R,
 * 
 *
 * Contains the memory structure used by bjarne.c and auto.c
 */ 
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
uint8_t WALL 		= 3;	//Tile: Outer Wall
uint8_t OUTSIDE 	= 4;	//Tile: Outside
uint8_t IWALL		= 1;	//Tile: Inner wall
uint8_t C_STACK_MAX = 100;  //Size of stack
uint8_t C_QUEUE_MAX = 100;	//Size of queue

//-------------------------------------------------------------

// Initialize and set standard value on all constants and memory positions
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
	land_o_hoy = 1;
	map_enclosed = 0;
	next_action = 0;

	home_x = 16;
	home_y = 16;

	map_complete = 0;
	first_time_on_island = 0;

	distance_covered = 0;

	// Set the outer layer of the map to OUTSIDE for the dfs
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
	changeQ.sizeofOut = -1;

	return;

}

// Enqueue mapdata in the queue
int8_t enqueue(uint8_t x, uint8_t y, uint8_t t){
	if(changeQ.sizeofIn == (C_QUEUE_MAX-1)){
		return 0; //Return false, since the queue is full
	}
	else{
		mapchange temp = {.x = x, .y = y, .t = t}; 
		changeQ.sizeofIn++;
		changeQ.inbox[changeQ.sizeofIn] = temp;
	}
}
// Dequeue mapdata from the queue
mapchange dequeue(){
	if(changeQ.sizeofOut == -1){
		while(changeQ.sizeofIn > -1){
			//pop value from inbox stack
			mapchange toBeMoved = changeQ.inbox[changeQ.sizeofIn];
			changeQ.sizeofIn--;

			//add to outbox stack
			changeQ.sizeofOut++;
			changeQ.outbox[changeQ.sizeofOut] = toBeMoved;
		}
	}
	//Q still empty?
	if(changeQ.sizeofOut != -1){
		mapchange data = changeQ.outbox[changeQ.sizeofOut];
		changeQ.sizeofOut--;
		return data;
	}
	else{
		mapchange data = {.x = 0, .y = 0, .t = 0}; 
		return data; //Return false, since the Q is empty
	}
}

// Push mapdata on the stack
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

// pop something from the stack
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

// Read tiletype from map-memory on position x,y
uint8_t rmem(uint8_t x, uint8_t y){

	return mapmem[x][y].tileType;

}

// Write tiletype to map-memory on position x,y
void wmem(uint8_t data, uint8_t x, uint8_t y){

	mapmem[x][y].tileType = data;
	
	return;
}

// Write tiletype to map-memory and enqueue the element in the queue. 
// This is used in the auto-mode.
void wmem_auto(uint8_t data, uint8_t x, uint8_t y){
	// Only enqueue if the data is different from its previous value.
	if ((mapmem[x][y].tileType != data) && (mapmem[x][y].tileType != WALL)){
		mapmem[x][y].tileType = data;
		enqueue(x, y, data);
	}

	return;
}

