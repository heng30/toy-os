#include "api.h"

void main(void) {
    unsigned int win = api_new_window(100, 100, 150, 50, "NWIN");
    api_draw_text_in_window(win, 28, 28, 0, "hello, world");
}
