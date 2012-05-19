#include "scandal_timer.h"

void sleep(sc_time_t miliseconds) {
	sc_time_t timer_sleep = sc_get_timer();
	timer_sleep = timer_sleep + miliseconds;
	while (1) {
		if (timer_sleep <= sc_get_timer() )
			break;
	}
}
