/* sunswift config.h*/
/* default config stuffs*/

#define DEFAULT_PRECHARGE_CONTACT_VOLTAGE	5000
#define DEFAULT_PRECHARGE_MINIMUM_VOLTAGE	40000
#define DEFAULT_PRECHARGE_MINIMUM_TIME		5000
#define DEFAULT_PRECHARGE_MAXIMUM_TIME		15000

/* ------------------------
Crazy config stuff
-------------------------*/
void config_read(void);
int config_write(void);


