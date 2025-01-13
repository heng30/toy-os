#include "api.h"
#include "colo8.h"
#include "pdef.h"
#include "putil.h"
#include "pstring.h"
#include "ptimer.h"

void main(void) {
    char text[11];
    int counter = 0;
    unsigned short col = (COL8_C6C6C6 << 8) | COLOR_BLACK;
    unsigned int win = api_new_window(100, 100, 150, 50, "Counter");

    unsigned int timer = api_timer_alloc();
    api_timer_set(timer, TIMER_ONE_SECOND_TIME_SLICE, TIMER_MAX_RUN_COUNTS);

    while (true) {
        counter++;
        int_to_string(counter, text);
        api_draw_text_in_window(win, 60, 28, col, text, false);

        if (api_is_close_window())
            break;

        timer_wait(timer);
    }

    api_close_window(win);
    api_timer_free(timer);
}
