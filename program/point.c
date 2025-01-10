#include "api.h"
#include "colo8.h"
#include "pdef.h"

void main(void) {
    unsigned int w = 150, h = 80;
    unsigned int win = api_new_window(100, 100, w, h, "Point");

    for (unsigned int i = 0; i < 50; i++) {
        unsigned int x =
            api_rand_uint(i * 347702432) % (w - WINDOW_BORDER_SIZE * 2) +
            WINDOW_BORDER_SIZE;

        unsigned int y =
            api_rand_uint(i * 92443887) %
                (h - WINDOW_TITLE_BAR_HEIGHT - WINDOW_BORDER_SIZE * 2) +
            WINDOW_TITLE_BAR_HEIGHT + WINDOW_BORDER_SIZE;

        api_draw_point_in_window(win, x, y, COLOR_YELLOW);
    }

    api_refresh_window(win, WINDOW_BORDER_SIZE, WINDOW_TITLE_BAR_HEIGHT,
                       w - WINDOW_BORDER_SIZE, h - WINDOW_BORDER_SIZE);

    while (true)
        ;
}
