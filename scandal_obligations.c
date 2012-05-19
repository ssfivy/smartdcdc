/* ------------------------------------------------------------------------
   Scandal Obligations
   Functions which the application writer must provide.
   
   File name: scandal_obligations.c
   Author: David Snowdon + Irving Tjiptowarsono
   Date: second half of 2009
   ------------------------------------------------------------------------- */

#include <io.h>

#include "scandal_obligations.h"
#include "scandal_error.h"
#include "scandal_devices.h"
#include "scandal_led.h"

/* for the default scaling values */
#include "scandal_engine.h"

#include "config.h"
#include "config_declarations.h"

extern volatile smartdcdc_config_t config; 

/*---------------------------------------------------------------------------------------
  Scaling default values. They're accurate for the first revised version (unit 01)
  ---------------------------------------------------------------------------------------*/
#define DEFAULT_SMARTDCDC_BATT_VOLTAGE_M			1399 /* 1401 */
#define DEFAULT_SMARTDCDC_BATT_VOLTAGE_B			-56750 /* -149 */
#define DEFAULT_SMARTDCDC_WAVESCULPTOR_VOLTAGE_M		1397
#define DEFAULT_SMARTDCDC_WAVESCULPTOR_VOLTAGE_B		-68792
#define DEFAULT_SMARTDCDC_5V_VOLTAGE_M				1863
#define DEFAULT_SMARTDCDC_5V_VOLTAGE_B				0
#define DEFAULT_SMARTDCDC_12V_VOLTAGE_M				1867
#define DEFAULT_SMARTDCDC_12V_VOLTAGE_B				-5643
#define DEFAULT_SMARTDCDC_CAN_5V_VOLTAGE_M			1863
#define DEFAULT_SMARTDCDC_CAN_5V_VOLTAGE_B			0
#define DEFAULT_SMARTDCDC_CAN_12V_VOLTAGE_M			1867
#define DEFAULT_SMARTDCDC_CAN_12V_VOLTAGE_B			-5643
#define DEFAULT_SMARTDCDC_DCDC_IN_CURRENT_M			23
#define DEFAULT_SMARTDCDC_DCDC_IN_CURRENT_B			-951
#define DEFAULT_SMARTDCDC_DCDC_OUT_CURRENT_M			311
#define DEFAULT_SMARTDCDC_DCDC_OUT_CURRENT_B			3373
#define DEFAULT_SMARTDCDC_HEATSINK_TEMP_M			9834
#define DEFAULT_SMARTDCDC_HEATSINK_TEMP_B			-1071100000
#define DEFAULT_SMARTDCDC_MSP_TEMP_M				34693
#define DEFAULT_SMARTDCDC_MSP_TEMP_B				-214425600


/* Reset the node in a safe manner
	- will be called from handle_scandal */
void scandal_reset_node(void){
  /* Reset the node here */
  /* Write an invalid password to the WDT */
  WDTCTL = ~WDTPW;
}

void scandal_user_do_first_run(void){
  /* Sets the default scaling values for EVERYTHING SCALABLE*/
  scandal_set_m( SMARTDCDC_BATT_VOLTAGE , DEFAULT_SMARTDCDC_BATT_VOLTAGE_M );
  scandal_set_b( SMARTDCDC_BATT_VOLTAGE , DEFAULT_SMARTDCDC_BATT_VOLTAGE_B ); 
  scandal_set_m( SMARTDCDC_WAVESCULPTOR_VOLTAGE , DEFAULT_SMARTDCDC_WAVESCULPTOR_VOLTAGE_M );
  scandal_set_b( SMARTDCDC_WAVESCULPTOR_VOLTAGE , DEFAULT_SMARTDCDC_WAVESCULPTOR_VOLTAGE_B ); 
  scandal_set_m( SMARTDCDC_5V_VOLTAGE , DEFAULT_SMARTDCDC_5V_VOLTAGE_M );
  scandal_set_b( SMARTDCDC_5V_VOLTAGE , DEFAULT_SMARTDCDC_5V_VOLTAGE_B ); 
  scandal_set_m( SMARTDCDC_12V_VOLTAGE , DEFAULT_SMARTDCDC_12V_VOLTAGE_M );
  scandal_set_b( SMARTDCDC_12V_VOLTAGE , DEFAULT_SMARTDCDC_12V_VOLTAGE_B ); 
  scandal_set_m( SMARTDCDC_CAN_5V_VOLTAGE , DEFAULT_SMARTDCDC_CAN_5V_VOLTAGE_M );
  scandal_set_b( SMARTDCDC_CAN_5V_VOLTAGE , DEFAULT_SMARTDCDC_CAN_5V_VOLTAGE_B ); 
  scandal_set_m( SMARTDCDC_CAN_12V_VOLTAGE , DEFAULT_SMARTDCDC_CAN_12V_VOLTAGE_M );
  scandal_set_b( SMARTDCDC_CAN_12V_VOLTAGE , DEFAULT_SMARTDCDC_CAN_12V_VOLTAGE_B );
  scandal_set_m( SMARTDCDC_DCDC_IN_CURRENT , DEFAULT_SMARTDCDC_DCDC_IN_CURRENT_M );
  scandal_set_b( SMARTDCDC_DCDC_IN_CURRENT , DEFAULT_SMARTDCDC_DCDC_IN_CURRENT_B ); 
  scandal_set_m( SMARTDCDC_DCDC_OUT_CURRENT , DEFAULT_SMARTDCDC_DCDC_OUT_CURRENT_M );
  scandal_set_b( SMARTDCDC_DCDC_OUT_CURRENT , DEFAULT_SMARTDCDC_DCDC_OUT_CURRENT_B );  
  scandal_set_m( SMARTDCDC_HEATSINK_TEMP , DEFAULT_SMARTDCDC_HEATSINK_TEMP_M );
  scandal_set_b( SMARTDCDC_HEATSINK_TEMP , DEFAULT_SMARTDCDC_HEATSINK_TEMP_B ); 
  scandal_set_m( SMARTDCDC_MSP_TEMP , DEFAULT_SMARTDCDC_MSP_TEMP_M );
  scandal_set_b( SMARTDCDC_MSP_TEMP , DEFAULT_SMARTDCDC_MSP_TEMP_B ); 


  config.precharge_contact_voltage = DEFAULT_PRECHARGE_CONTACT_VOLTAGE;
  config.precharge_minimum_voltage = DEFAULT_PRECHARGE_MINIMUM_VOLTAGE;
  config.precharge_minimum_time = DEFAULT_PRECHARGE_MINIMUM_TIME;
  config.precharge_maximum_time = DEFAULT_PRECHARGE_MAXIMUM_TIME;

  config_write(); 


  return;
}

u08 scandal_user_do_config(u08 param, s32 value, s32 value2){
	switch(param){
		case SMARTDCDC_PRECHARGE_CONTACT_VOLTAGE:
			config.precharge_contact_voltage = value;
			break;

		case SMARTDCDC_PRECHARGE_MINIMUM_VOLTAGE:
			config.precharge_minimum_voltage = value;
			break;

		case SMARTDCDC_PRECHARGE_MINIMUM_TIME:
			config.precharge_minimum_time = value;
			break;

		case SMARTDCDC_PRECHARGE_MAXIMUM_TIME:
			config.precharge_maximum_time = value;
			break;

	}

	return NO_ERR;
}

u08 scandal_user_handle_message(can_msg* msg){
	return NO_ERR;
}

u08 scandal_user_handle_command(u08 command, u08* data){
  return NO_ERR; 
}
