#include "api.h"
#include "colo8.h"
#include "pdef.h"
#include "puitl.h"

void main(void) {
    unsigned int win = api_new_window(100, 100, 150, 50, "Counter");
    unsigned short col = (COLOR_WHITE << 8) | COLOR_BLACK;

    while (true) {
        api_draw_text_in_window(win, 28, 28, col, "hello, world");

        if (api_is_close_window())
            break;

        delay(DEFAULT_DELAY_LOOP);
    }

    api_close_window(win);
}
