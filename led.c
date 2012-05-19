/* --------------------------------------------------------------------------
	LED Control
	File name: led.c
	Author: David Snowdon

	Date: 1/7/03
   -------------------------------------------------------------------------- */ 
 
#include <io.h>
#include <signal.h>
#include <iomacros.h>

#include "scandal_led.h"

#include "hardware.h"
    
#define BIT(x) (1<<x)


void yellow_led(u08 on){
	if(!on)
		P5OUT |= YELLOWLED;
	else
		P5OUT &= ~YELLOWLED;	
}

void toggle_yellow_led(void){
	P5OUT ^= YELLOWLED;
}


void red_led(u08 on){
	if(!on)
		P5OUT |= REDLED;
	else
		P5OUT &= ~REDLED;	
}

void toggle_red_led(void){
	P5OUT ^= REDLED;
}
