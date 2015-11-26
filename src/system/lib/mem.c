
#include "mem.h"


//----------------Variables------------------------------------
node mapmem[32][32];		//The map memory 33x33 tiles large
mapchange change_stack[99];	//A stack where changes to the map will be waiting to be sent
int8_t c_stack_top = -1; 		//The top of the stack
int8_t a_stack_top = -1;


uint8_t action_s[50]; //The actions required to get to target

//-------------------------------------------------------------


//----------------Constants------------------------------------
uint8_t UNEXP 		= 1;	//Tile: Unexplored
uint8_t FLOOR 		= 2;	//Tile: Floor
uint8_t WALL 		= 3;	//Tile: Wall
uint8_t OUTSIDE 	= 4;	//Tile: Outside
uint8_t C_STACK_MAX = 100;  //Size of stack
uint8_t A_STACK_MAX = 50;  //Size of stack

//-------------------------------------------------------------

void init_mem(){

	robot_pos_x = 16;	// Start in the middle of the map
	robot_pos_y = 16;
	target_x = 0; //Target tile
	target_y = 0;
	dir = 0;

	//OKÄND SKA VARA 1, fixa
	return;

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

	return;
}

node* rmem(uint8_t x, uint8_t y){

	return &mapmem[x][y];

}

void wmem(uint8_t data, uint8_t x, uint8_t y){

	mapmem[x][y].tileType = data;
	
	return;
}

void wmem_auto(uint8_t data, uint8_t x, uint8_t y){

	//Denna kallar auto på, lägger till i change-stacken om ny data
	if (mapmem[x][y].tileType != data){ //NY DATA, ska skickas upp
		
		mapmem[x][y].tileType = data;
		pstack(x, y, data);
	}
	return;
}

uint8_t paction(uint8_t action){

	if(a_stack_top == (A_STACK_MAX-1)){

		return 0; //Return false, since the stack is full

	}
	else{

		a_stack_top += 1;
		action_s[a_stack_top] = action;
		return 1;
	}

	
	return 0;
}

uint8_t read_a_top(){ //Läs action stack top utn att ändra den

	if((a_stack_top == -1)){

		return 0; //Om stacken är tom, returna 0

	}
	else{

		return action_s[a_stack_top];

	}

	return 0;
}

void pop_a_stack(){

	if((a_stack_top != -1)){
		a_stack_top -= 1;
	}

	return;
}
