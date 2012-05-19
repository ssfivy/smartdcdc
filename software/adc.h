/* get interrupts from ADC input 9 */
#define ADC_INTERRUPT_ENABLE() (ADC12IE |= (1 << 9))
#define ADC_INTERRUPT_DISABLE() (ADC12IE &= ~(1 << 9))

/* All other ADC function prototypes are in scandal_adc.h */ 
uint16_t read_adc_value(uint8_t channel);
