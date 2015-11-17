#ifndef MEM_H
#define MEM_H

#include <avr/io.h>
typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t t;
} mapchange;

void wmem(uint8_t data, uint8_t x, uint8_t y); 		//Write to map-memory
uint8_t rmem(uint8_t x, uint8_t y); 				//Read from map-memory
uint8_t pstack(uint8_t x, uint8_t y, uint8_t t);	//Put data on change stack
mapchange gstack(); 

//----------------Constants------------------------------------
uint8_t UNEXP;	//Tile: Unexplored
uint8_t FLOOR;	//Tile: Floor
uint8_t WALL;	//Tile: Wall
uint8_t OUTSIDE;	//Tile: Outside
uint8_t STACK_MAX;  //Size of stack

//-------------------------------------------------------------




#endif