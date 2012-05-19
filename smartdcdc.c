/*	======================
 *	Sunswift Smart DC-DC Converter
 *	File: smartdcdc.c
 *	Author: Irving Suryajaya Tjiptowarsono
 *	Start Date: 2 May 2009
 *	======================  
 * Pardon me for the massive amount of comment that might be present,
 * I plan to document everything I learn. */
/* */

/* Include msp430-libc libraries */
#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <string.h>

/* Include scandal header files */
#include "scandal_timer.h"
#include "scandal_led.h"
#include "scandal_can.h"
#include "scandal_engine.h"
#include "scandal_spi.h"
#include "spi_devices.h"
#include "scandal_devices.h"
#include "scandal_utils.h"
#include "scandal_message.h"
#include "scandal_adc.h"
#include "scandal_error.h"

/* Include device header files */
#include "hardware.h"
#include "adc.h"
#include "smartdcdc.h"
#include "ads8341.h"
#include "config.h"
#include "config_declarations.h"
#include "sleep.h"

/* Watchdog stuff */
#define WDTCTL_INIT     WDTPW|WDTHOLD
#define WDTCONFIG	(WDTCNTCL|WDTSSEL)

volatile smartdcdc_config_t    config; 

/*------------------------------------------------------------------------------
  Initialize ports. Everything is 0 by default.
  Use | to set a bit to 1, &~ to set a bit to 0, ^= to flip a bit.
  For details see smartdcdc schematic, hardware.h and the MSP430 book.
  --------------------------------------------------------------------------------*/
void init_ports(void){
  P1OUT = 0x00;
  P1SEL = 0x00;
  P1DIR = CAN_RESET | AUX1_SW | AUX2_SW | RELAY_SW ;
  P1IES = CAN_INT;
  P1IE  = 0x00;

	P2OUT = 0x00;
	P2SEL = 0x00;
	P2DIR = 0x00;
	P2IES = 0x00;
	P2IE  = 0x00;

  	P3OUT = ADC1_CS | DCDC_SW;
  	P3SEL = SIMO0 | SOMI0 | UCLK0;
	P3DIR = SIMO0 | UCLK0 | ADC1_CS | CONTACT_SW | DCDC_SW;

	P4OUT = 0x00;
	P4SEL = 0x00;
	P4DIR = CAN_SW;

	P5OUT = CAN_CS;
	P5SEL = SIMO1 | SOMI1 | UCLK1;
	P5DIR = CAN_CS | SIMO1 | UCLK1 | REDLED | YELLOWLED;

	P6DIR = 0x00;
	P6SEL = LVISENSE | CAN5V_MEASURE | CAN12V_MEASURE | _5V_MEASURE | _12V_MEASURE ;
}

void init_clock(void){
	volatile unsigned int i;

	/* XTAL = LF crystal, ACLK = LFXT1/1, DCO Rset = 4, XT2 = ON */
	BCSCTL1 = 0x04;

	/* Clear OSCOFF flag - start oscillator */
	_BIC_SR( OSCOFF );
	do{
		/* Clear OSCFault flag */
		IFG1 &= ~OFIFG; 
		/* Wait for flag to set */
		for( i = 255; i > 0; i-- )
			;
	} while(( IFG1 & OFIFG ) != 0);

	/* Set MCLK to XT2CLK and SMCLK to XT2CLK */
	BCSCTL2 = 0x88; 
}

/*--------------------------------------------------
  Interrupt handing for CAN stuff 
  --------------------------------------------------*/
void enable_can_interrupt(){
	P1IE = CAN_INT;
}

void disable_can_interrupt(){
	P1IE = 0x00;
}

interrupt (PORT1_VECTOR) port1int(void) {
	can_interrupt();
	P1IFG = 0x00;
	//toggle_red_led();
}

/*--------------------------------------------
  Main function 
  --------------------------------------------*/
int main(void) {
	/*initialize everything */
	/* Declare variables*/
	sc_time_t timer_led, sample_old_time, scale_old_time, send_old_time, measure_old_time; //Scandal timer, ads8341 sampling and scaling
	u32 channel_updated[3] = {0,0,0}; //shows the last time a message is received, according to smartdcdc timestamp
	int can_status = CAN_DEFAULT; //set the default CAN status from hardware.
	int vicor_status = VICOR_ON;
	s32 msp_temp, heatsink_temp, msp_sense, heatsink_sense;

	/*high voltage variables */
	s32 hvisense, battvcc, mcvcc;
	s32 hvisense_scaled, battvcc_scaled, mcvcc_scaled;
	hvisense = battvcc = mcvcc = 0;
	hvisense_scaled = battvcc_scaled = mcvcc_scaled = 0;

	/* precharge variables */
	int precharge_status = STATUS_IDLE; //THE variable. Start with idle.
	int contactor_status = CONTACTOR_OFF; //need to be initialized, hence it's not static
	int relay_status = RELAY_OFF; 
	int ignition_command = IGNITION_OFF; // ignition switch on/off
	sc_time_t precharge_ignition_timer; //ignition input timer
	sc_time_t precharge_length_timer ; //measures how long precharge takes place
	sc_time_t precharge_error_timer; //for delay in sending error messages
	
	/*Initialization stuff*/
	P5OUT = CAN_CS;
	
	dint(); //disable interrupts
	WDTCTL = WDTCTL_INIT; //Stop watchdog timer
	init_ports();
	toggle_yellow_led();
	init_clock();
	sc_init_timer();
	init_ADS8341();
	P1OUT |= CAN_RESET; //Set the CAN controller trigger high
	scandal_init(); //init scandal
	eint(); //enable interrupts
	
	timer_led = sample_old_time = scale_old_time = send_old_time = measure_old_time = sc_get_timer(); //set timer variables
	precharge_ignition_timer = precharge_length_timer = precharge_error_timer = sc_get_timer();
	init_adc(); //
	config_read(); //load all config values
	msp_sense = heatsink_sense = 0x0000;//reset temp variables
	sleep(CAN_PRECHARGE_DELAY);//delay to let the output caps charge
	can_status = CAN_ON;
	toggle_yellow_led();
	
	// Configure watchdog timer as a 32.768 kHz counter with 1 second interrupt
	WDTCTL = WDTPW | WDTCONFIG;               // WDT 1000ms, ACLK, interval timer
	
	// Initialization complete, ready for takeoff!
	
	/*------------------------------------------------------------
	  WITH YOUR INITS COMBINED, I AM INFINITE LOOP!
	  ------------------------------------------------------------*/
	while(1){
		/* Kick ze watchdog */
		WDTCTL = WDTPW | WDTCONFIG;
		
		/*Handles scandal stuff*/
		handle_scandal();
		
		/* toggles red LED */
		if(sc_get_timer() >= timer_led + 300) {
			timer_led = sc_get_timer();
			toggle_red_led();
		}
		
		/* Handles aux outputs */
		handle_aux(channel_updated);
		
		/* Handle anything measured by the msp adc */
		handle_adc();

		/* Handle CAN switching */
		handle_can(&can_status);

		/* Handle vicor switching */
		handle_vicor(&vicor_status);
		
		/*-------------------------------------------------------------
		Get readings and measurements, in preparation for precharge.
		-------------------------------------------------------------*/

		/* 
		Get high voltage measurements.
		note that any floating pins will cause the ADC reading to 
		slowly ramp up until about ~65 then resets itself to 0
		before repeating. dont know why.
		 */

		/* Samples stuff from ADS8341 */
		if(sample_old_time + ADC_SAMPLE_DELAY < sc_get_timer()){
			s32 summand;
			sample_old_time = sc_get_timer();
			/* Input current.*/
			summand = (s32)ADS8341_sample(ADS8341_HVISENSE);
			hvisense >>= 1; 
			hvisense += summand;
			/* Battvcc */
			summand = (s32)ADS8341_sample(ADS8341_BATTVCC);
			battvcc >>= 1; 
			battvcc += summand;
			/* MCVCC */
			summand = (s32)ADS8341_sample(ADS8341_MCVCC);
			mcvcc >>= 1; 
			mcvcc += summand;
			/* Heatsink Temperature */
			summand = (s32)ADS8341_sample(ADS8341_HEATSINK_TEMP);
			heatsink_sense >>= 1; 
			heatsink_sense += summand;
		}

		/* check temp */
		if(measure_old_time + TEMPERATURE_MEASURE_DELAY < sc_get_timer()){
			measure_old_time = sc_get_timer();
			s32 summand;
			/* Heatsink temperature . Legacy code for version 00, for reference only.
			summand = (s32)sample_adc(MEAS_ADC3_TEMP);
			heatsink_sense >>= 1; 
			heatsink_sense += summand; */
			/* MSP temperature */
			summand = (s32)sample_adc(MEAS_TEMP);
			msp_sense >>= 1; 
			msp_sense += summand;
		}

		/* scale measurements */
		if(scale_old_time + ADC_SCALE_DELAY < sc_get_timer()){
			scale_old_time = sc_get_timer();
			/* Input current */
			hvisense_scaled = hvisense;
			//scales a variable according to b and m for a particular out channel
			scandal_get_scaled_value(SMARTDCDC_DCDC_IN_CURRENT, &hvisense_scaled);

			/* Bus Voltage */
			battvcc_scaled = battvcc;
			scandal_get_scaled_value(SMARTDCDC_BATT_VOLTAGE, &battvcc_scaled);

			/* Wavesculptor Voltage */
			mcvcc_scaled = mcvcc;
			scandal_get_scaled_value(SMARTDCDC_WAVESCULPTOR_VOLTAGE, &mcvcc_scaled);

			/* heatsink temp */
			heatsink_temp = heatsink_sense;
			scandal_get_scaled_value(SMARTDCDC_HEATSINK_TEMP, &heatsink_temp);

			/* msp temp */
			msp_temp = msp_sense;
			scandal_get_scaled_value(SMARTDCDC_MSP_TEMP, &msp_temp);
		}

		/* get ignition command */
		if(scandal_get_in_channel_rcvd_time(SMARTDCDC_IGNITION) > precharge_ignition_timer) { //if new message received
			if(scandal_get_in_channel_value(SMARTDCDC_IGNITION) == IGNITION_ON) { //if receives on command
				//toggle_red_led();
				ignition_command = IGNITION_ON;
			}
			else if (scandal_get_in_channel_value(SMARTDCDC_IGNITION) == IGNITION_OFF) {
				//toggle_red_led();
				ignition_command = IGNITION_OFF;
			}
			precharge_ignition_timer = scandal_get_in_channel_rcvd_time(SMARTDCDC_IGNITION); //set new timestamp.
		}

		/*-------------------------------------------------------------
		Safety check for dangerous conditions
		-------------------------------------------------------------*/
		
		if ((P3IN & CONTACT_VREF) == 0) {
			//contactor fuse blows
			if(precharge_error_timer + PRECHARGE_ERROR_DELAY < sc_get_timer()){
				precharge_error_timer = sc_get_timer();
				scandal_do_user_err(SMARTDCDC_ERROR_CONTACTOR_FUSE_BLOWS);
			}
		}
		
		if (heatsink_temp > HEATSINK_MAXIMUM_TEMPERATURE ) {
			// heatsink too hot 
			if(precharge_error_timer + PRECHARGE_ERROR_DELAY < sc_get_timer()){
				precharge_error_timer = sc_get_timer();
				scandal_do_user_err(SMARTDCDC_ERROR_HEATSINK_OVERHEAT);
			}
		}
		
		if (msp_temp > MSP_MAXIMUM_TEMPERATURE ) {
			// MSP (ambient temp) too hot 
			if(precharge_error_timer + PRECHARGE_ERROR_DELAY < sc_get_timer()){
				precharge_error_timer = sc_get_timer();
				scandal_do_user_err(SMARTDCDC_ERROR_CIRCUIT_OVERHEAT);
			}
		} 
		
		if ((battvcc_scaled + WAVESCULPTOR_OVER_VOLTAGE_THRESHOLD) <= mcvcc_scaled ) {
			//* mcvcc higher than battvcc, possible regen /contactor problem ?
			if(precharge_error_timer + PRECHARGE_ERROR_DELAY < sc_get_timer()){
				precharge_error_timer = sc_get_timer();
				scandal_do_user_err(SMARTDCDC_ERROR_WAVESCULPTOR_OVER_VOLTAGE);
			}
		}
		
		if ((contactor_status == CONTACTOR_ON) && (relay_status == RELAY_OFF)) {
			//this combination should never happen.
			if(precharge_error_timer + PRECHARGE_ERROR_DELAY < sc_get_timer()){
				precharge_error_timer = sc_get_timer();
				scandal_do_user_err(SMARTDCDC_ERROR_RELAY_POSITION_WEIRD);
			}
		}
		
		/*-------------------------------------------------------------
		Precharge control algorithm. Do not touch.
		-------------------------------------------------------------*/
		if (precharge_status == STATUS_IDLE) {
			// switch to precharging
			if (ignition_command == IGNITION_ON) {
				// check minimum voltage
				if (battvcc_scaled < config.precharge_minimum_voltage) {
					//* bus Voltage too low, no precharging!
					scandal_do_user_err(SMARTDCDC_ERROR_INPUT_UNDER_VOLTAGE);
					/* No I'm not going to start precharging, go away */
					ignition_command = IGNITION_OFF;
				}
				else {
					//its fine!
					set_relay(RELAY_ON);
					relay_status = RELAY_ON;
					precharge_length_timer = sc_get_timer(); //reset timer
					precharge_status = STATUS_PRECHARGE;
					scandal_send_channel(TELEM_LOW, SMARTDCDC_PRECHARGE_STATUS, precharge_status);
				}
			}
		}
	
		else if (precharge_status == STATUS_PRECHARGE) {
			// prepare to switch to running 
			//checks for correct voltage threshold
			if ((battvcc - mcvcc) < config.precharge_contact_voltage) {
				//* ensures precharge doesnt finish too fast.
				//also allows the contactor cap time to charge.
				if ((sc_get_timer() - precharge_length_timer) >= config.precharge_minimum_time ) {
					set_contactor(CONTACTOR_ON);
					precharge_status = STATUS_RUN;
					contactor_status = CONTACTOR_ON;
					/* need to send values immediately to driver controls? */
					scandal_send_channel(TELEM_LOW, SMARTDCDC_PRECHARGE_STATUS, precharge_status);
				}
				else {
					//ay! precharge finishes too fast! may be a problem!
					scandal_do_user_err(SMARTDCDC_ERROR_PRECHARGE_UNDERTIME);
				}
			}
	
			//* Checks if precharge happens too long. Untested.
			else if ((sc_get_timer() - precharge_length_timer) >= config.precharge_maximum_time ) {
				//* precharge is too long, something is wrong 
				if(precharge_error_timer + PRECHARGE_ERROR_DELAY < sc_get_timer()){
					precharge_error_timer = sc_get_timer();
					scandal_do_user_err(SMARTDCDC_ERROR_PRECHARGE_OVERTIME);
				}
			}
	
			//* prepare to switch to idle */
			if (ignition_command == IGNITION_OFF) {
				set_contactor(CONTACTOR_OFF);
				contactor_status = CONTACTOR_OFF;
				set_relay(RELAY_OFF);
				relay_status = RELAY_OFF;
				precharge_status = STATUS_IDLE;
				scandal_send_channel(TELEM_LOW, SMARTDCDC_PRECHARGE_STATUS, precharge_status);
			}
		}
		
		else if (precharge_status == STATUS_RUN) {
			//* prepare to switch to idle*/
			if (ignition_command == IGNITION_OFF) {
				set_contactor(CONTACTOR_OFF);
				contactor_status = CONTACTOR_OFF;
				set_relay(RELAY_OFF);
				relay_status = RELAY_OFF;
				precharge_status = STATUS_IDLE;
				scandal_send_channel(TELEM_LOW, SMARTDCDC_PRECHARGE_STATUS, precharge_status);
			}
			
			/* Auto discharge - EXPERIMENTAL, DO NOT TOUCH
			   This is to prevent blowing up the precharge caps, otherwise the scenario will be like: 
				1. battery is disconnected
				2. caps discharged (while contactor still engages)
				3. battery re-connected
				4. BOOM.
			*/
			
			// checks if low voltage is consistant and not just random noise
			// how? check both battvcc and mcvcc. Average over time would be better but needs division
			if ( (battvcc_scaled < config.precharge_minimum_voltage)  && 
			(mcvcc_scaled   < config.precharge_minimum_voltage)  )  {
				//its constant! Bus voltage drops, maybe battery disconnected?
				//do auto discharge!
				set_contactor(CONTACTOR_OFF);
				contactor_status = CONTACTOR_OFF;
				set_relay(RELAY_OFF);
				relay_status = RELAY_OFF;
				precharge_status = STATUS_IDLE;
				scandal_send_channel(TELEM_LOW, SMARTDCDC_PRECHARGE_STATUS, precharge_status);
				scandal_do_user_err(SMARTDCDC_ERROR_BUS_VOLTAGE_MISSING);
			}
		}


		/* Direct relay switching. and monitoring. Debugging only. */
		handle_relay(&relay_status);

		/* Direct contactor switching and monitoring. Debugging only. */
		handle_contactor(&contactor_status);

		/*-------------------------------------------------------------
		End of control algorithm.
		-------------------------------------------------------------*/

		/*Sends scandal messages*/
		if(send_old_time + PRECHARGE_SEND_DELAY < sc_get_timer()){
			scandal_send_channel(TELEM_LOW, SMARTDCDC_PRECHARGE_STATUS, precharge_status);
			scandal_send_channel(TELEM_LOW, SMARTDCDC_DCDC_IN_CURRENT, hvisense_scaled);
			scandal_send_channel(TELEM_LOW, SMARTDCDC_BATT_VOLTAGE, battvcc_scaled);
			scandal_send_channel(TELEM_LOW, SMARTDCDC_WAVESCULPTOR_VOLTAGE, mcvcc_scaled);
			scandal_send_channel(TELEM_LOW, SMARTDCDC_HEATSINK_TEMP, heatsink_temp);
			scandal_send_channel(TELEM_LOW, SMARTDCDC_MSP_TEMP, msp_temp);
			send_old_time = sc_get_timer();
		}
		
	}
	return 0; //should never reach here, keeps compiler happy.
}



/*--------------------------------------------------------------------------------
  Smartdcdc module functions. For what they doo see the declarations in modules.h
  --------------------------------------------------------------------------------*/

/* Handle aux outputs*/
static inline void handle_aux(u32 *channel_updated) {
	static sc_time_t timer_aux;
	int i, aux_status[2];
	
	for(i=SMARTDCDC_AUX1_SW; i<=SMARTDCDC_AUX2_SW; i++) {
		if(scandal_get_in_channel_rcvd_time(i) > channel_updated[i]) { //if new message received
			if(scandal_get_in_channel_value(0) != 0) //if channel 0 receives a nonzero
				P1OUT |= AUX1_SW; //turn on switch
			else 
				P1OUT &= ~AUX1_SW; //turn off.
			if(scandal_get_in_channel_value(1) != 0)
				P1OUT |= AUX2_SW;
			else 
				P1OUT &= ~AUX2_SW;
			channel_updated[i] = scandal_get_in_channel_rcvd_time(i); //set new timestamp.
		}
	}
	
	/* If fuse broken, send error. Else, send status, on or off. */
	if ((P1IN & AUX1_VREF) == 0) 
		aux_status[0] = ERROR_AUX_FUSE_BLOWS;
	else
		aux_status[0] = scandal_get_in_channel_value(SMARTDCDC_AUX1_SW);
	if ((P2IN & AUX2_VREF) == 0)
		aux_status[1] = ERROR_AUX_FUSE_BLOWS;
	else
		aux_status[1] = scandal_get_in_channel_value(SMARTDCDC_AUX2_SW);

	/* Send scandal messages  */
	if(sc_get_timer() >= timer_aux + SCANDAL_DELAY_AUX){
		timer_aux = sc_get_timer();
		if(aux_status[0] == ERROR_AUX_FUSE_BLOWS)
			scandal_do_user_err(SMARTDCDC_ERROR_AUX1_FUSE_BLOWS);
		if(aux_status[1] == ERROR_AUX_FUSE_BLOWS)
			scandal_do_user_err(SMARTDCDC_ERROR_AUX2_FUSE_BLOWS);
		scandal_send_channel(TELEM_LOW, SMARTDCDC_AUX1_STATUS, aux_status[0]);
		scandal_send_channel(TELEM_LOW, SMARTDCDC_AUX2_STATUS, aux_status[1]);
	}
}

/* Handle anything measured by the msp adc, 
   except heatsink temp as it is needed for precharge control. */
static inline void handle_adc() {
	static sc_time_t timer_adc;
	if(sc_get_timer() >= timer_adc + SCANDAL_DELAY_ADC){
		timer_adc = sc_get_timer();
		scandal_send_scaled_channel(TELEM_LOW, SMARTDCDC_12V_VOLTAGE, sample_adc(MEAS_12V));
		scandal_send_scaled_channel(TELEM_LOW, SMARTDCDC_5V_VOLTAGE, sample_adc(MEAS_5V));
		scandal_send_scaled_channel(TELEM_LOW, SMARTDCDC_CAN_12V_VOLTAGE, sample_adc(MEAS_CAN12V));
		scandal_send_scaled_channel(TELEM_LOW, SMARTDCDC_CAN_5V_VOLTAGE, sample_adc(MEAS_CAN5V));
		scandal_send_scaled_channel(TELEM_LOW, SMARTDCDC_DCDC_OUT_CURRENT, sample_adc(MEAS_LVISENSE));
	}
}

/* Handles the contactor. Debug function, do not touch. */
void handle_contactor(int *contactor_status) {
	static sc_time_t timer_contactor;
	
	//not yet implemented
	static u32 channel_updated;
	if(scandal_get_in_channel_rcvd_time(SMARTDCDC_CONTACTOR_SWITCH) > channel_updated) { //if new message received
		if(scandal_get_in_channel_value(SMARTDCDC_CONTACTOR_SWITCH) != *contactor_status) { //if receives different command
			*contactor_status = scandal_get_in_channel_value(SMARTDCDC_CONTACTOR_SWITCH);
			set_contactor(*contactor_status);
		}
		channel_updated = scandal_get_in_channel_rcvd_time(SMARTDCDC_CONTACTOR_SWITCH); //set new timestamp.
	}

	/* Send scandal messages  */
	if(sc_get_timer() >= timer_contactor + SCANDAL_DELAY_CONTACTOR) {
		timer_contactor = sc_get_timer();
		if ((P3IN & CONTACT_VREF) == 0) {
			*contactor_status = ERROR_CONTACTOR_FUSE_BLOWS;
			scandal_do_user_err(SMARTDCDC_ERROR_CONTACTOR_FUSE_BLOWS);
		}
		scandal_send_channel(TELEM_LOW, SMARTDCDC_CONTACTOR_STATUS, *contactor_status);
	}
}

/* Handles can switching */
static inline void handle_can(int *can_status) {
	static sc_time_t timer_can;

	static u32 channel_updated;
	if(scandal_get_in_channel_rcvd_time(SMARTDCDC_CAN_POWER) > channel_updated) { //if new message received
		if(scandal_get_in_channel_value(SMARTDCDC_CAN_POWER) != *can_status){ //if receives different
			toggle_yellow_led();
			*can_status = scandal_get_in_channel_value(SMARTDCDC_CAN_POWER);
		}

		channel_updated = scandal_get_in_channel_rcvd_time(SMARTDCDC_CAN_POWER); //set new timestamp.
	}
	
	/* Switch CAN, if the hardware is set to turn on CAN by default */
	if (CAN_DEFAULT == CAN_ON) {
		if (*can_status == CAN_ON) 
			P4OUT |= CAN_SW;
		else if (*can_status == CAN_OFF) 
			P4OUT &= ~CAN_SW;
	}
	
	
	/* Switch CAN, if the hardware is set to turn off CAN by default */
	else if (CAN_DEFAULT == CAN_OFF) {
		if (*can_status == CAN_OFF)
			P4OUT &= ~CAN_SW;
		else if (*can_status == CAN_ON)
			P4OUT |= CAN_SW;
	}
	

	/* Send scandal messages  */
	if(sc_get_timer() >= timer_can + SCANDAL_DELAY_CAN) {
		timer_can = sc_get_timer();
		scandal_send_channel(TELEM_LOW, SMARTDCDC_CAN_STATUS, *can_status);
	}
}

/* Debug function. Flips the relay. */
void handle_relay(int *relay_status) {
	static sc_time_t timer_relay;

	static u32 channel_updated;
	if(scandal_get_in_channel_rcvd_time(SMARTDCDC_RELAY_SWITCH) > channel_updated) { //if new message received
		if(scandal_get_in_channel_value(SMARTDCDC_RELAY_SWITCH) != *relay_status) { //if receives on command
			*relay_status = scandal_get_in_channel_value(SMARTDCDC_RELAY_SWITCH);
			set_relay(*relay_status);
		}
		else if (scandal_get_in_channel_value(SMARTDCDC_RELAY_SWITCH) == RELAY_OFF) {
			//toggle_red_led();
			*relay_status = RELAY_OFF;
		}
		channel_updated = scandal_get_in_channel_rcvd_time(SMARTDCDC_RELAY_SWITCH); //set new timestamp.
	}
	

	/* Send scandal messages  */
	if(sc_get_timer() >= timer_relay + SCANDAL_DELAY_RELAY) {
		timer_relay = sc_get_timer();
		scandal_send_channel(TELEM_LOW, SMARTDCDC_RELAY_STATUS, *relay_status);
	}
}

/* Sets the relay to on/off position*/
 void set_relay(int relay_position) {
	if (relay_position == RELAY_ON) {
		P1OUT |= RELAY_SW; //PRECHARGE
	}
	else if (relay_position == RELAY_OFF )
		P1OUT &= ~RELAY_SW; //discharge.
}

/* Sets the contactor to on/off position */
 void set_contactor(int contactor_position) {
	if (contactor_position == CONTACTOR_ON) {
		P3OUT |= CONTACT_SW; //turn on switch
	}
	else if (contactor_position == CONTACTOR_OFF )
		P3OUT &= ~CONTACT_SW; //turn off.
}

/*turns the vicor box off/on */
void handle_vicor(int *vicor_status) {
	static sc_time_t timer_vicor;

	static u32 channel_updated;
	if(scandal_get_in_channel_rcvd_time(SMARTDCDC_VI_J00) > channel_updated) { //if new message received
		if(scandal_get_in_channel_value(SMARTDCDC_VI_J00) != 0) { //if receives on command
			P3OUT |= DCDC_SW; //turn on vicor box
		}
		else {
			P3OUT &= ~DCDC_SW; //turn off vicor
		}
		*vicor_status = scandal_get_in_channel_value(SMARTDCDC_VI_J00);
		channel_updated = scandal_get_in_channel_rcvd_time(SMARTDCDC_VI_J00); //set new timestamp.
	}
	
	/* Send scandal messages  */
	if(sc_get_timer() >= timer_vicor + SCANDAL_DELAY_VICOR) {
		timer_vicor = sc_get_timer();
		scandal_send_channel(TELEM_LOW, SMARTDCDC_VICOR_STATUS, *vicor_status);
	}
}
