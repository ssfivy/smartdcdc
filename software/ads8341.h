/*! -------------------------------------------------------------------------- 
  \file ads8341.h 
	ADS8341 Driver 
	 
	File name: ads8341.h 
	Author: David Snowdon   
	Date: 11/8/02           
    -------------------------------------------------------------------------- */   

#ifndef __ADS8341__
#define __ADS8341__

#include <scandal_types.h>    
    
/* Function Prototypes */
void init_ADS8341(void);
u16  ADS8341_sample(u08 channel);

#endif
