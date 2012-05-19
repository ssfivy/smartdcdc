/*! ------------------------------------------------------------------------- 
    \file scandal_timer.c
        Scandal Timer for AVR as used in CANRefNode
	 
	File name: scandal_timer.c 
	Author: David Snowdon 
	Date: 1/7/03
    -------------------------------------------------------------------------- */  

#include <io.h>
#include <signal.h>

#include "scandal_timer.h"

static volatile u32 ms;

/* Interrupt handler associated with internal RTC */
/* Timer A overflow interrupt */
interrupt (TIMERA0_VECTOR) timera_int(void) {
  ms += 1000;
}

void sc_init_timer(void){
  /* Set ms to zero */
  ms = 0;
  
  /* Use TimerA to create periodic interrupts */
  
  /* Clear counter, input divider /1, ACLK */
  TACTL = /*TAIE |*/ TACLR | ID_DIV1 | TASSEL_ACLK;

  /* Enable Capture/Compare interrupt */
  TACCTL0 = CCIE;
  TACCR0 = 32767; /* Count 1 sec at ACLK=32768Hz */
  
  /* Start timer in up to CCR0 mode */
  TACTL |= MC_UPTO_CCR0;
}

void sc_set_timer(sc_time_t time){
  TACCTL0 &= ~CCIE;
  TACCR0 = ((time % 1000) << 15) / 1000;
  ms = time / 1000;
  TACCTL0 |= CCIE;
}

sc_time_t sc_get_timer(void){
  sc_time_t 	time;
  u32           tar_copy;

  /* Work out what the time in ms is */

  /* Turn off the timer interrupt */ 
  TACCTL0 &= ~CCIE; 

  /* Short delay so that we're sure the interrupt is off */ 
  {
    volatile int i; 
    for(i=0; i<15; i++)
      ;
  }

  /* Copy the relevant numbers */ 
  time = ms;
  tar_copy = TAR; 

  /* Turn the timer interrupt back on */ 
  TACCTL0 |= CCIE; 

  time += ((tar_copy * 1000) >> 15);

  return time;
}
