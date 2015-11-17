/*
 * lidar_pwm.c
 *
 * Created: 11/13/2015 11:58:42 AM
 *  Author: micso554 and the ericboy
 */

#include <avr/io.h>


void timer1_init(){
	// reset registers and set prescaler to 1024
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B |= (1<<CS10)|(0<<CS11)|(1<<CS12);
	
	// reset timer
	TCNT1 = 0;
}

int main(void){
	
	timer1_init();
	
	int prev = 0;
	int current = 0;
	uint16_t timeOfMyLife;
	uint8_t cm;
	
	//uint16_t hej = 0xf077;
	
    while(1){
		
		//lidar PWM PC1 (SDA PIN 22)
		current = PINC && 0x02;		
		// positive edge trigger
		if (prev == 0){
			if (current == 1){
				TCNT1 = 0;
				prev = 1;	
			}
		}
		// negative edge trigger
		else if (prev == 1){
			if (current == 0){			
					
				uint8_t low  = TCNT1;
				uint8_t high = TCNT1H;
				//PORTD = low;
				
				// timeOfMyLife = high + low;
				timeOfMyLife = high << 8 |low;
				
				// conversion to cm with number magic
				cm = timeOfMyLife *200/39;
				//display cm on PORTD
				PORTD = cm;
				prev = 0;								
			}
		}
    }
} 