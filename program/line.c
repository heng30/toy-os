#include "api.h"
#include "colo8.h"
#include "pdef.h"
#include "putil.h"

void main(void) {
    unsigned int w = 150, h = 100;
    unsigned int win = api_new_window(100, 100, w, h, "Line");

    for (unsigned int i = 0; i < 8; i++) {
        api_draw_line_in_window(win, 8, 26, 77, i * 9 + 26, (unsigned char)i);
        api_draw_line_in_window(win, 88, 26, i * 9 + 88, 89, (unsigned char)i);
    }

    api_refresh_window(win, WINDOW_BORDER_SIZE, WINDOW_TITLE_BAR_HEIGHT,
                       w - WINDOW_BORDER_SIZE, h - WINDOW_BORDER_SIZE);

    while (true) {
        if (api_is_close_window())
            break;

        delay(DEFAULT_DELAY_LOOP);
    }

    api_close_window(win);
}
