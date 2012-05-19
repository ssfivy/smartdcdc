/*! -------------------------------------------------------------------------- 
  \file ads8341.h
	ADS8341 Driver 
	 
	File name: ADS8431.c 
	Author: David Snowdon   
	Date: 11/8/02           
    -------------------------------------------------------------------------- */   

#include <io.h>

#include "scandal_spi.h"
#include "other_spi.h"
#include "spi_devices.h"
#include "ads8341.h"
#include "scandal_engine.h"    
#include "scandal_led.h"

void init_ADS8341(void){
//  init_spi();
  init_spi0();
}

u16 ADS8341_sample(u08 channel){
	u08 val;
	u16 result = 0;
	u16 i;
	
	spi0_select_device(ADC1);
	val = 1 << 7; /* Should be defined, but its late and it has to work tomorrow. 
				So I've written all this in the time it would have taken
				me to define this constant properly, hey...ARGH!?!?!? Sleeeeep. */
	
	/* Weird channel selection values. See the datasheet for details */
	switch(channel){
		case 0:
			val |= (0x01<<4);
			break;
		case 1:
			val |= (0x05<<4);
			break;
		case 2: 
			val |= (0x02<<4);
			break;
		case 3: 
			val |= (0x06<<4);
			break;
	}
	
	val |= (1<<2); /* Select single ended (rather than differential) mode */
	
	spi0_transfer(val); /* First byte is the control byte */

	for(i=0; i<200; i++)
	  _NOP();
	

	val = spi0_transfer(0); /* First 7 bits of the value */
	if(val != 0)
	  //toggle_red_led(); 
	result = ((u16)val&0x007F) << 9;

	val = spi0_transfer(0);
	if(val != 0)
	  //toggle_red_led(); 
	result |= (((u16)val&0x00FF)) << 1;

	val = spi0_transfer(0);
	if(val != 0)
	  //toggle_red_led(); 
	result |= (((u16)val>>7) & 0x0001) ;

	spi0_deselect_all();
	
	return result;
}

