#ifndef AUTO_H
#define AUTO_H
#include <avr/io.h>





uint8_t next_step();
void update_sensor_data(); 
void init_auto();
void autonom();
void setSpeed(uint8_t lspeed, uint8_t rspeed, uint8_t ldir, uint8_t rdir);

uint8_t state;


uint8_t s_LIDAR_u;
uint8_t s_LIDAR_l;
uint16_t s_LIDAR;
uint8_t t_LIDAR;	//Antal 40 cm rutor till vägg 0 - 20 cm   1 - 20+40cm   2 20+80cm

uint8_t s_ir_h_f;
uint8_t s_ir_h_b;
uint8_t s_ir_v_f;
uint8_t s_ir_v_b;

//	Token parallell vänster/höger

int8_t t_p_h;	// 0- ej parallell, 1 - parallell
int8_t t_p_v;	// 0- ej parallell, 1 - parallell

uint8_t t_vagg_h_f; // 0- ingen vägg , 1 - vägg inom 20 cm ish, 2 - vägg 20+40 cm
uint8_t t_vagg_h_b; // 0- ingen vägg , 1 - vägg inom 20 cm ish, 2 - vägg 20+40 cm
uint8_t t_vagg_v_f; // 0- ingen vägg , 1 - vägg inom 20 cm ish, 2 - vägg 20+40 cm
uint8_t t_vagg_v_b; // 0- ingen vägg , 1 - vägg inom 20 cm ish, 2 - vägg 20+40 cm

uint8_t s_gyro_u;
uint8_t s_gyro_l;
uint16_t s_gyro;
uint8_t t_gyro;		// bestäm vilka värden vi vill ha

uint8_t spinning;

#endif