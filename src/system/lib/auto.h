/*
 * Created: November 2015
 * Author : Mikael Å, Anton R
 * 
 * Header file for brain.c
 */ 
#ifndef AUTO_H
#define AUTO_H
#include <avr/io.h>

uint8_t next_step();
void update_sensor_data(); 
void init_auto();
void autonom();
void setSpeed(uint8_t lspeed, uint8_t rspeed, uint8_t ldir, uint8_t rdir);
uint8_t spinning;

#endif