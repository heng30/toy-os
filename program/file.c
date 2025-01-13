#include "api.h"
#include "colo8.h"
#include "pdef.h"
#include "pstring.h"
#include "putil.h"

void main(void) {
    const char *fsize_str = "a.txt file size: ";
    char str[11], msg[64];
    unsigned int w = 200, h = 100;
    unsigned int win = api_new_window(80, 100, w, h, "File");
    unsigned short col = (COL8_C6C6C6 << 8) | COLOR_BLACK;

    int fd = api_file_open("a.txt", true);

    if (fd < 0) {
        api_console_draw_text("open file a.txt failed", true);
        return;
    }

    unsigned int fsize_str_len = api_strlen_cs(fsize_str);
    api_memcpy_cs2ds((unsigned char *)msg, (unsigned char *)fsize_str,
                     fsize_str_len + 1);

    int fsize = api_file_size(fd);
    int_to_string(fsize, str);
    strcat(msg, str);
    api_draw_text_in_window(win, 8, 28, col, msg, false);

    if (fsize > 0) {
        int rlen = api_file_read(fd, (unsigned char *)msg, sizeof(msg), 0);
        msg[rlen] = '\0';
        clean_str(msg);
        api_draw_text_in_window(win, 8, 28 + FONT_HEIGHT, col, msg, false);
    }

    api_memcpy_cs2ds(msg, "foobar!!!", 10);
    api_file_write(fd, (unsigned char*)msg, 9, 6);

    int rlen = api_file_read(fd, (unsigned char *)msg, sizeof(msg), 0);
    if (rlen > 0) {
        msg[rlen] = '\0';
        clean_str(msg);
        api_draw_text_in_window(win, 8, 28 + FONT_HEIGHT * 2, col, msg, false);
    }

    api_file_close(fd);

    while (true) {
        if (api_is_close_window())
            return;

        delay(DEFAULT_DELAY_LOOP);
    }
}
