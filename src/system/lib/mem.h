#ifndef MEM_H
#define MEM_H

#define EMPTY		0 //The a_stack was empty //Stand still
#define FORWARD 	1 //Go forward 1 tile
#define	SPIN_R		2 //Turn 90 right
#define SPIN_L		3 //Turn 90 left
#define	SPIN_180 	4 //Turn 180 left
#define PARALLELIZE	5 //Turn until parallel with wall on right side
#define	BACK		6 //Back-up one tile

#define NORTH	0
#define	EAST	1
#define	SOUTH	2	
#define	WEST	3

#include <avr/io.h>
typedef struct {
	uint8_t x;
	uint8_t y;
	uint8_t t;
} mapchange;

typedef struct node{
		uint8_t x;
		uint8_t y;
		node* parent;
		//uint8_t visited;
		uint8_t tileType;
} node;

void wmem(uint8_t data, uint8_t x, uint8_t y); 		//Write to map-memory
void wmem_auto(uint8_t data, uint8_t x, uint8_t y);
node* rmem(uint8_t x, uint8_t y); 				//Read from map-memory
uint8_t pstack(uint8_t x, uint8_t y, uint8_t t);	//Put data on change stack
mapchange gstack(); 

void init_mem();

uint8_t gaction();
uint8_t paction(uint8_t action);

uint8_t read_a_top();
void pop_a_stack();


//----------------Constants------------------------------------
uint8_t UNEXP;	//Tile: Unexplored
uint8_t FLOOR;	//Tile: Floor
uint8_t WALL;	//Tile: Wall
uint8_t OUTSIDE;	//Tile: Outside
uint8_t STACK_MAX;  //Size of stack

//				Actions


//-------------------------------------------------------------

uint8_t robot_pos_x;	// Start in the middle of the map
uint8_t robot_pos_y;

uint8_t target_x; //Target tile
uint8_t target_y;

uint8_t dir;


#endif