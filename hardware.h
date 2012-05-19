/* Hardware definitions */
#define BIT(x) (1<<x)

#define CLOCK_SPEED 7372800

#define AUXSW(a)	BIT(a)

/* Port 1 */
#define CAN_RESET	BIT(0)
#define AUX1_SW		BIT(1)
#define AUX1_VREF	BIT(2)
#define AUX2_SW		BIT(3)
#define CAN_INT		BIT(4)
#define RELAY_SW	BIT(5)

/* Port 2 */
#define AUX2_VREF	BIT(1)

/* Port 3 */

#define ADC_SIMO0	BIT(1)
#define SOMI0		BIT(2)
#define UCLK0		BIT(3)
#define ADC_CS		BIT(4)
#define DCDC_SW		BIT(5)
#define CONTACT_VREF	BIT(6)
#define CONTACT_SW	BIT(7)

/* Port 4 */
#define CAN_SW		BIT(0)

/* Port 5 */
#define CAN_CS          BIT(0) /* Note: re-defined in scandal_devices.h */
#define SIMO1           BIT(1)
#define SOMI1           BIT(2)
#define UCLK1           BIT(3)
#define REDLED		BIT(6)
#define YELLOWLED	BIT(7)

/* Port 6 / ADC */
#define LVISENSE	BIT(0)
//#define ADC3_TEMP	BIT(1)
#define CAN5V_MEASURE	BIT(4)
#define CAN12V_MEASURE	BIT(5)
#define _5V_MEASURE	BIT(6) /* appearantly, C hates definitions that start with numbers */
#define _12V_MEASURE	BIT(7)

/* ADC channel definitions */
#define MEAS_LVISENSE	0
//#define MEAS_ADC3_TEMP	1
#define MEAS_CAN5V	4
#define MEAS_CAN12V	5
#define MEAS_5V		6
#define MEAS_12V	7
#define MEAS_TEMP       8
#define MEAS_3V3        9

/* ADS8341 Current sensor channel #defs */
#define ADS8341_HVISENSE    	0
#define ADS8341_BATTVCC    	1
#define ADS8341_MCVCC    	2
#define ADS8341_HEATSINK_TEMP	3
