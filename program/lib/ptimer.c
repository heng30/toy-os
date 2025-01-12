#include "ptimer.h"
#include "api.h"
#include "puitl.h"

void timer_wait(unsigned int timer) {
    while (!api_timer_is_timeout(timer)) {
        delay(DEFAULT_DELAY_LOOP);
    }
}
