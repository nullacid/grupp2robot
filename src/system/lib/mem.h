#ifndef MEM_H
#define MEM_H

typedef struct mapchange{
	uint8_t x;
	uint8_t y;
	uint8_t t;
};

void wmem(uint8_t data, uint8_t x, uint8_t y); 		//Write to map-memory
uint8_t rmem(uint8_t x, uint8_t y); 				//Read from map-memory
uint8_t pstack(uint8_t x, uint8_t y, uint8_t t);	//Put data on change stack
mapchange gstack(); 




#endif