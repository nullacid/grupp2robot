#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 20000000
#include <util/delay.h>

#define FORWARD 1
#define BACK 0
#define false 0
#define true 1
#define bool uint8_t

void setSpeed(uint8_t lspeed, uint8_t rspeed, uint8_t ldir, uint8_t rdir);
void init_motors();
void handle_messages();
uint8_t decide_if_repeated();


uint8_t dir_left = 1; //0 or 1 = back or forward
uint8_t dir_right = 1;
uint8_t spd_left = 0; //0 to 100 = procent of full speed
uint8_t spd_right = 0;
uint8_t button_autonom = 0; // 0 om manuellt läge, 1 om autonomt läge
uint8_t message = 1;
bool new_message = false;

int main(){
	
	init_motors();
	
	while(1){
		
		handle_messages();
		//updatera sensordata
		
		if (button_autonom == 1){
			
			//tolka sensordata
			//reagera på sensordata
			
		}
		
		
	
	
		setSpeed(50,100,FORWARD,BACK);
	}
}

uint8_t decide_if_repeated(){
	
	
	
}

void handle_messages(){
	
	uint8_t message_cpy = message;
	uint8_t repeated = decide_if_repeated();
	
	//plocka ut OP-koden
	message_cpy &= 31;
	
	
	if (button_autonom == 0){ //Manuellt läge
		if (!repeated){
			switch(message_cpy){
				case (0): //pil UPP trycks ner
				dir_left = 1;
				dir_right = 1;
				spd_left = spd_left + 50;
				spd_right = spd_right + 50;
				break;
				case(1): //pil UPP släpps
				spd_left = spd_left - 50;
				spd_right = spd_right - 50;
				break;
				case(2): //pil VÄNSTER trycks ner
				spd_right = spd_right + 50;
				break;
				case(3): //pil VÄNSTER släpps
				spd_right = spd_right - 50;
				break;
				case (4): //pil NER trycks ner
				dir_left = 0;
				dir_right = 0;
				spd_left = spd_left + 50;
				spd_right = spd_right + 50;
				break;
				case(5): //pil NER släpps
				dir_left = 1;
				dir_right = 1;
				spd_left = spd_left - 50;
				spd_right = spd_right - 50;
				break;
				case(6): //pil HÖGER trycks ner
				spd_left = spd_left + 50;
				break;
				case(7): //pil HÖGER släpps
				spd_left = spd_left - 50;
				break;				
			}
		}
		//setSpeed(spd_left,spd_right,dir_left,dir_right);
	}

	switch(message_cpy){
		
		case (0x08):
			//lägg lidardata i send-buffern
		break;
		
		case (0x09):
		//lägg sensordata IR höger fram-data i send-buffern
		break;
		
		case (0x0A):
		//lägg sensordata IR höger bak-data i send-buffern
		break;
		
		case (0x0B):
		//lägg sensordata IR vänster fram-data i send-buffern
		break;
		
		case (0x0C):
		//lägg sensordata IR vänster bak-data i send-buffern
		break;
		
		case (0x0D):
		//lägg gyro-data i send-buffern
		break;
		
		case (0x0E):
		//lägg reflexsensor-data i send-buffern
		break;
		
		case (0x0F):
		//lägg lidar-token i send-buffern
		break;
		
		case (0x10):
		//lägg parallell höger-token i send-buffern
		break;
		
		case (0x11):
		//lägg parallell vänster-token i send-buffern
		break;
		
		case (0x12):
		//lägg gyro-token i send-buffern
		break;
		
		case (0x13):
		//lägg vägg höger fram-token i send-buffern
		break;
		
		case (0x14):
		//lägg vägg höger bak-token i send-buffern
		break;
		
		case (0x15):
		//lägg vägg vänster fram-token i send-buffern
		break;
		
		case (0x16):
		//lägg vägg vänster bak-token i send-buffern
		break;
		
		case (0x17):
		//lägg reflex-token i send-buffern
		break;
		
		case (0x18):
		//lägg kartdata i send-buffern
		break;
		
		case (0x19):
		//senaste styrbeslut i send-buffern
		break;

		case (0x1A):
		//pos karta X och Y i send-buffern
		break;
		
		case (0x1B):
		//pos i algoritm i send-buffern
		break;
		
		case (0x1C):
		//lägg tempKartdata i send-buffern
		break;
		
	}		
}


void init_motors(){
	
	PORTB=0x00; //Reset any output
	PORTA=0x00; //Port A sets dir of motors (PA6, PA7)
	DDRA |=(1 << DDA6) | (1 << DDA7);
	DDRB|=(1 << DDB6 );//set OC3A as output.
	DDRD|=(1 << DDD5 );//set OC1A as output.
	
	//Setup timer1
	TCCR1A |= (1 << WGM10) | (1 << WGM11) | (1 << COM1A1);
	TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);
		
	//Setup timer3
	TCCR3A |= (1 << WGM30) | (1 << WGM31) | (1 << COM3A1);
	TCCR3B |= (1 << WGM32) | (1 << CS31) | (1 << CS30);
}

void setSpeed(uint8_t lspeed, uint8_t rspeed, uint8_t ldir , uint8_t rdir){
	
	PORTA |= (dir_left << DDA7) | (dir_right << DDA6); //DDA7 är vänster, DDA6 är höger 	
	OCR1A = 10*rspeed;//set the duty cycle(out of 1023) Höger	
	OCR3A = 10*lspeed;//set the duty cycle(out of 1023) Vänster
	
}


