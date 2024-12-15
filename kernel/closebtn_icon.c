#include "closebtn_icon.h"
#include "colo8.h"

// clang-format off
unsigned char closebtn_icon[CLOSEBTN_ICON_HEIGHT][CLOSEBTN_ICON_WIDTH] = {
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
    static unsigned char _closebtn_icon_buf[CLOSEBTN_ICON_HEIGHT][CLOSEBTN_ICON_WIDTH];

    for (unsigned int y = 0; y < CLOSEBTN_ICON_HEIGHT; y++) {
        for (unsigned int x = 0; x < CLOSEBTN_ICON_WIDTH; x++) {
            unsigned char c = closebtn_icon[y][x];
            if (c == '@') {
                c = close_icon_color;
            } else if (c == '$') {
                c = rb_border_color;
            } else if (c == 'Q') {
                c = background_color;
            } else {
                c = tl_border_color;
            }

            _closebtn_icon_buf[y][x] = c;
        }
    }

    return (const unsigned char *)_closebtn_icon_buf;
}
