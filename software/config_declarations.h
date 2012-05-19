/*config_declarations*/

typedef struct smartdcdc_config{
  int32_t precharge_contact_voltage;
  int32_t precharge_minimum_voltage;
  int32_t precharge_minimum_time;
  int32_t precharge_maximum_time;
  
  /* Checksums */ 
  uint8_t magic; 
  uint8_t checksum; 
  uint8_t checkxor; 
}smartdcdc_config_t; 


extern volatile smartdcdc_config_t config; 
