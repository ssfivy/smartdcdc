/* -------------------------------------------------------------------------- 
	SPI Devices 
	 
	File name: spi_devices.h
	Author: David Snowdon 
	Date: 12/04/02 
   ------------------------------------------------------------------------*/ 
  
#ifndef __SPIDEVICES__ 
#define __SPIDEVICES__ 

#include <io.h>

#define BIT(x) (1<<x)

/* Port 3 */
#define SIMO0           BIT(1)
#define SOMI0           BIT(2)
#define UCLK0           BIT(3)
#define ADC1_CS         BIT(4) /* Note: re-defined in scandal_devices.h */
#define UCLK2           BIT(7) /* Also connected to BIT(6) */

#define MCP2510			0              
#define SPI_NUM_DEVICES         1
#define SPI_DEVICE_NONE		SPI_NUM_DEVICES 

#define ADC1                    0
#define SPI0_NUM_DEVICES	1
#define SPI0_DEVICE_NONE        SPI0_NUM_DEVICES     

/* MCP2510 */
#define ENABLE_MCP2510()        (P5OUT &= ~BIT(0))
#define DISABLE_MCP2510()       (P5OUT |= BIT(0))

/* ADC1 */
#define ENABLE_ADC1()           (P3OUT &= ~0x10)
#define DISABLE_ADC1()          (P3OUT |= 0x10)

#endif
