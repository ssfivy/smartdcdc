/*
smartdcdc.h
Irving Suryajaya Tjiptowarsono
header file for ifdefs in smartdcdc. Includes documentation for functions and status messages.
The functions are in smartdcdc.c below the main function
*/

/*
Anything with scandal_delay in them means they are delays for sending status messages over 
the can network, as not to spam the network.
*/


/*---------------------------------------------------------------------------------------
  Handle aux outputs.
  This is ported from the switchcard code, with some modification.
  Turns aux outputs on and off.
  It will also sends status messages: 0 is off, 1 is on, fuse blows specified below.
  ---------------------------------------------------------------------------------------*/
static inline void handle_aux(u32 *channel_updated);

#define SCANDAL_DELAY_AUX		3000
#define ERROR_AUX_FUSE_BLOWS		9000

/*---------------------------------------------------------------------------------------
  Handles anything measured by the msp adc.
  This is mainly the 5V, 12V, CAN 5V, and CAN12V.
  Nonvoltage measurements are the heatsink temp sensor and the 12V current sense.
  ---------------------------------------------------------------------------------------*/
static inline void handle_adc(void);
#define SCANDAL_DELAY_ADC		3000

/*---------------------------------------------------------------------------------------
  CAN switching.
  ---------------------------------------------------------------------------------------*/
static inline void handle_can(int *can_status);
#define SCANDAL_DELAY_CAN		3000
#define CAN_PRECHARGE_DELAY		2000 /* Delay before activating the can outputs, to charge output caps. */
#define CAN_OFF				0
#define CAN_ON				1
#define CAN_DEFAULT			CAN_OFF /* Changing this requires changing resistors in the board.*/

/*---------------------------------------------------------------------------------------
  Relay switching. handle_relay is a debug function and should not be used in normal 
  conditions. However it also does sending status messages so it shouldn't be commented out.
  ---------------------------------------------------------------------------------------*/
void handle_relay(int *relay_status);
void set_relay(int relay_position);
#define RELAY_ON			1
#define RELAY_OFF			0
#define SCANDAL_DELAY_RELAY		3000

/*---------------------------------------------------------------------------------------
  Contactor switching. As with relay switching, this is a debug function and should not 
  be used in normal circumstances, nor should it be commented out.
  ---------------------------------------------------------------------------------------*/
void handle_contactor(int *contactor_status);
void set_contactor(int contactor_position);
#define SCANDAL_DELAY_CONTACTOR		3000
#define ERROR_CONTACTOR_FUSE_BLOWS 	9000
#define CONTACTOR_ON			1
#define CONTACTOR_OFF			0

/*---------------------------------------------------------------------------------------
  Vicor switching. Turns the vicor dc/dc on or off.
  ---------------------------------------------------------------------------------------*/
#define VICOR_ON			1
#define VICOR_OFF			0
#define SCANDAL_DELAY_VICOR		5000
void handle_vicor(int *vicor_status);

/*---------------------------------------------------------------------------------------
  Temperature limits. Maximum temperature where the smartdcdc will start sending errors.
  Keep in mind that temperature measurements are highly innacurate due to insufficient 
  calibration methods available sometimes they can be off +- 5 or more degrees.
  ---------------------------------------------------------------------------------------*/
#define HEATSINK_MAXIMUM_TEMPERATURE	105000 	/* 105 C */
#define MSP_MAXIMUM_TEMPERATURE		80000	/* 80 C */

/*---------------------------------------------------------------------------------------
  Timers for precharge algorithm.
  ---------------------------------------------------------------------------------------*/
#define ADC_SAMPLE_DELAY		0 /* full throttle, row row fight the powah */
#define ADC_SCALE_DELAY			100
#define PRECHARGE_SEND_DELAY		3000
#define TEMPERATURE_MEASURE_DELAY	10 /*temperature */
#define PRECHARGE_ERROR_DELAY		100


/*---------------------------------------------------------------------------------------
  definitions for precharge.
  There are two kinds of errors. Showstopper error must be manually cleared / reset, and 
  may require opening the smartdcdc (eg fuse blows.) Condition error will clear themselves 
  when their condition is met (eg undervoltage error)
  ---------------------------------------------------------------------------------------*/
#define IGNITION_OFF			0
#define IGNITION_ON			1
#define WAVESCULPTOR_OVER_VOLTAGE_THRESHOLD	5000 /* 5 Volt */

#define STATUS_IDLE			1
#define STATUS_PRECHARGE		2
#define STATUS_RUN			3


/*---------------------------------------------------------------------------------------
  Scaling default values are in scandal_obligations.c
  config stuff are in config.h 
  ---------------------------------------------------------------------------------------*/



