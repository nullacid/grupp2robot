#include "brain.h"
#include "mem.h"
#include <avr/io.h>

void find_next_wall();
void find_empty_tile();

void bfs(uint8_t startx, uint8_t starty, uint8_t targetx, uint8_t targety);
void bfs_t(uint8_t startx, uint8_t starty, uint8_t target_tile);
void dfs(uint8_t startx, uint8_t starty, target_tile);

uint8_t follow_wall = 1;
uint8_t startup = 1;


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
		


	}


//Kod för att hitta nästa target, en tile vi kan åka på 
	//jämte nästa troliga vägg (så vi kan se om den finns där)
	return;
}

void find_empty_tile(){

//Hitta en unexplored tile vi kan åka til

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

void dfs(uint8_t startx, uint8_t starty, uint8_t target_tile){
//Används för att kolla om väggen runt är sluten.	

	return;
}
