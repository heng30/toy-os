#include "api.h"
#include "colo8.h"
#include "pdef.h"
#include "puitl.h"

void main(void) {
    unsigned int win = api_new_window(100, 100, 150, 50, "NWIN");
    api_draw_box_in_window(win, 8, 36, 141, 43, COLOR_YELLOW);

    unsigned short col = (COLOR_WHITE << 8) | COLOR_BLACK;
    api_draw_text_in_window(win, 28, 28, col, "hello, world");

    while (true) {
        if (api_is_close_window())
            return;

        delay(DEFAULT_DELAY_LOOP);
    }
}
