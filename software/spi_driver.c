/*! ------------------------------------------------------------------------- 
   MSPGCC SPI Master Driver 
    
   File name: spi_driver.c 
   Author: David Snowdon 
   Date: 12/04/02 
   -------------------------------------------------------------------------- */ 
#include <io.h>

#include "spi_devices.h" 
#include "scandal_spi.h"
#include "other_spi.h"

#include "scandal_led.h"

/* Temporary definitions */
/* Hardware definitions */
#define SPI0_CLK_H()       (P3OUT |= UCLK0)
#define SPI0_CLK_L()       (P3OUT &= ~(UCLK0))
#define SPI0_SIMO_H()      (P3OUT |= SIMO0)
#define SPI0_SIMO_L()      (P3OUT &= ~(SIMO0))
#define SPI0_SIMO_VAL(x)   (P3OUT = (P3OUT & ~SIMO0) | (x << 1))
#define SPI0_SOMI_VAL()    ((P3IN & SOMI0) >> 2)

/* Initialize the SPI driver */         

/* Local variables */ 
u08 spi_selected_device, spi0_selected_device; 

/*! Initialise the SPI driver to the default values */ 
u08 init_spi(){
  ME2 |= USPIE1;
  
  U1CTL  = SYNC+MM+CHAR;
  U1TCTL = STC | SSEL1 | SSEL0 | CKPL ;
  UBR01 = 0x04;
  UBR11 = 0x00;
  UMCTL1 = 0x00;

  spi_selected_device = SPI_DEVICE_NONE; 
  spi_deselect_all(); 
  return(0); 
}  

u08 init_spi0(){ 
  //  ME1 |= USPIE0;
  
  //  U0CTL  = SYNC+MM+CHAR;
  //  U0TCTL = STC | SSEL1|SSEL0  ;//| CKPL ;//| CKPH;
  //  UBR00 = 0x04;
  //  UBR10 = 0x00;
  //  UMCTL1 = 0x00;

  P3OUT |= ADC1_CS;
  P3SEL &= ~(SIMO0 | SOMI0 | UCLK0 | ADC1_CS);
  P3DIR |= SIMO0 | UCLK0 | ADC1_CS;

  spi0_selected_device = SPI0_DEVICE_NONE; 
  spi0_deselect_all(); 
  return(0); 
}  

/*! Select a particular device */ 
u08 spi_select_device(u08	device){  
  if(spi_selected_device != SPI_DEVICE_NONE) 
    return(1); 
   
  spi_deselect_all(); 
   
  switch(device){ 
  case MCP2510:  
    ENABLE_MCP2510();
    break; 
  } 
   
  spi_selected_device = device; 	 
  return(0); 
} 

/*! Select a particular device */ 
u08 spi0_select_device(u08	device){  
  if(spi0_selected_device != SPI0_DEVICE_NONE) 
    return(1); 
   
  spi0_deselect_all(); 
  
  switch(device){ 
  case ADC1:
    ENABLE_ADC1();
    break;
  } 
   
  spi0_selected_device = device; 	 
  return(0); 
} 

/*! Deselect all devices */ 
void spi_deselect_all(){ 
  DISABLE_MCP2510();
  spi_selected_device = SPI_DEVICE_NONE; 
} 

void spi0_deselect_all(){ 
  DISABLE_ADC1();
  spi0_selected_device = SPI0_DEVICE_NONE; 
} 

/*! Perform an SPI read/write */ 
u08 spi_transfer(u08 out_data){ 
  u08	value; 

  IFG2 &= ~URXIFG1;

  while((IFG2 & UTXIFG1) == 0)
      ;
  
  TXBUF1 = out_data;

  while((IFG2 & URXIFG1) == 0)
      ;

  value = RXBUF1;

  return(value); 
} 

u08 spi0_transfer(u08 out_data){ 
  /*  u08	value; 
      
  IFG2 &= ~URXIFG0;
  
  while((IFG1 & UTXIFG0) == 0)
  ;
  
  TXBUF0 = out_data;
  
  while((IFG1 & URXIFG0) == 0)
  ;

      value = RXBUF0;

  return(value); */

  u08 in_data = 0, i;


  for(i=0; i< 8; i++){
    SPI0_CLK_L(); /* Falling edge of clock */
    SPI0_SIMO_VAL((out_data & 0x80) >> 7);
    out_data <<= 1;

    SPI0_CLK_H(); /* Rising edge of clock */
    in_data <<= 1;                /* In data */
    in_data |= SPI0_SOMI_VAL();
  }

  SPI0_CLK_L();

  /* Leave with SPI0_SIMO_L() */
  SPI0_SIMO_L();

  return in_data;
} 

