/* config.c 
 * David Snowdon, 28 March, 2008
 */
#include <io.h>
#include <signal.h>
#include <iomacros.h>

#include "scandal_devices.h"
#include "scandal_eeprom.h"
#include "scandal_types.h"
#include "scandal_error.h"
#include "config.h"
#include "config_declarations.h"

/* Magic number to make sure EEPROM has been programmed */
/* Appearantly this can be any magic number. Its magic! d(^_^)b */
#define MPPTNG_CONFIG_MAGIC 0xAA

static inline void
calculate_checksum(smartdcdc_config_t config, uint8_t *sum, uint8_t *xor){
    uint8_t* array; 
    uint16_t i; 
    
    *sum = *xor = 0;
    array = (uint8_t*)(&config); 
    for(i=0; i<sizeof(config); i++){
        *sum += *array; 
        *xor ^= *array; 
        array++; 
    }
}

void
config_read(void){
  uint8_t sum, xor; 
  uint8_t insum, inxor; 

  sc_user_eeprom_read_block(0, (uint8_t*)&config, sizeof(config)); 
    
  insum = config.checksum; 
  inxor = config.checkxor; 
    
  config.checksum = config.checkxor = 0; 

  calculate_checksum(config, &sum, &xor); 

  if( (insum != sum) || (inxor != xor) ){
    scandal_do_user_err(SMARTDCDC_ERROR_EEPROM); 
  }
}

int config_write(void){
  uint8_t sum, xor; 

  config.checksum = config.checkxor = 0; 
  
  calculate_checksum(config, &sum, &xor); 
  
  config.checksum = sum; 
  config.checkxor = xor; 

  sc_user_eeprom_write_block(0, (u08*)&config, sizeof(config)); 

  return 0; 
}
