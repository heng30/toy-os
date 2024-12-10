#include "closebtn_icon.h"
#include "colo8.h"

// clang-format off
char closebtn_icon[CLOSEBTN_ICON_HEIGHT][CLOSEBTN_ICON_WIDTH] = {
    "OOOOOOOOOOOOOOO@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQ@@QQQQ@@QQ$@",
    "OQQQQ@@QQ@@QQQ$@",
    "OQQQQQ@@@@QQQQ$@",
    "OQQQQQQ@@QQQQQ$@",
    "OQQQQQ@@@@QQQQ$@",
    "OQQQQ@@QQ@@QQQ$@",
    "OQQQ@@QQQQ@@QQ$@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQQQQQQQQQQQ$@",
    "O$$$$$$$$$$$$$$@",
    "@@@@@@@@@@@@@@@@"
};

// clang-format on
const char *closebtn_icon_buf(unsigned char close_icon_color,
                              unsigned char tl_border_color,
                              unsigned char rb_border_color,
                              unsigned char background_color) {
    static char buf[CLOSEBTN_ICON_HEIGHT][CLOSEBTN_ICON_WIDTH];

    for (int y = 0; y < CLOSEBTN_ICON_HEIGHT; y++) {
        for (int x = 0; x < CLOSEBTN_ICON_WIDTH; x++) {
            char c = closebtn_icon[y][x];
            if (c == '@') {
                c = close_icon_color;
            } else if (c == '$') {
                c = rb_border_color;
            } else if (c == 'Q') {
                c = background_color;
            } else {
                c = tl_border_color;
            }

            buf[y][x] = c;
        }
    }

    return (const char *)buf;
}
