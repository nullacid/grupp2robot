#include <avr/io.h>
#include <avr/interrupt.h>
#include "mem.h"

//----------------Prototypes-----------------------------------
void wmem(uint8_t data, uint8_t x, uint8_t y); 		//Write to map-memory
uint8_t rmem(uint8_t x, uint8_t y); 				//Read from map-memory
uint8_t pstack(uint8_t x, uint8_t y, uint8_t t);	//Put data on change stack
mapchange gstack(); 								//Get data from stack
//-------------------------------------------------------------

//----------------Constants------------------------------------
uint8_t UNEXP 		= 1;	//Tile: Unexplored
uint8_t FLOOR 		= 2;	//Tile: Floor
uint8_t WALL 		= 3;	//Tile: Wall
uint8_t OUTSIDE 	= 4;	//Tile: Outside
uint8_t STACK_MAX 	= 100;  //Size of stack

//-------------------------------------------------------------

//----------------Variables------------------------------------
uint8_t mapmem[33][33];				//The map memory 34x34 tiles large
mapchange change_stack[99]; //A stack where changes to the map will be waiting to be sent
int8_t stack_top = -1; 				//The top of the stack
//-------------------------------------------------------------

/*
typedef struct mapchange{
	uint8_t x;
	uint8_t y;
	uint8_t t;
};
*/
uint8_t pstack(uint8_t x, uint8_t y, uint8_t t){

	if(stack_top == (STACK_MAX-1)){

		return 0; //Return false, since the stack is full

	}
	else{

		mapchange temp = {.x = x, .y = y, .t = t}; 


		stack_top += 1;
		change_stack[stack_top] = temp;
	}

	return 1;

}

mapchange gstack(){

	if (stack_top == -1){

		mapchange data = {.x = 0, .y = 0, .t = 0}; 
		return data; //Return false, since the stack is empty

	}
	else{

		mapchange data = change_stack[stack_top];
		stack_top -= 1;

		return data;

	}
}

uint8_t rmem(uint8_t x, uint8_t y){

	return mapmem[x][y];

}

void wmem(uint8_t data, uint8_t x, uint8_t y){

	mapmem[x][y] = data;

}