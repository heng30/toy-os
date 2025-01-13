#include "api.h"
#include "colo8.h"
#include "pdef.h"
#include "puitl.h"

void main(void) {
    unsigned int w = 144, h = 164;
    unsigned int win = api_new_window(100, 100, w, h, "Pallete");

    for (unsigned char y = 0; y < 128; y++) {
        for (unsigned char x = 0; x < 128; x++) {
            unsigned char r = x * 2, g = y * 2, b = 0;
            unsigned char color =
                (unsigned char)(16 + (r / 43) + (g / 43) * 6 + (b / 43) * 36);
            api_draw_point_in_window(win, x + 7, y + 27, color);
        }
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
