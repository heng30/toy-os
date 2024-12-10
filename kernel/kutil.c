#include "kutil.h"
#include "colo8.h"
#include "draw.h"
#include "io.h"

char char2hex(char c) {
    if (c >= 10) {
        return 'A' + c - 10;
    }

    return '0' + c;
}

char *char2hexstr(unsigned char c) {
    static char str[5] = {'0', 'X', 0, 0, 0};
    char m = c % 16;

    str[3] = char2hex(m);
    c = c / 16;
    str[2] = char2hex(c);

    return str;
}

char *int2hexstr(unsigned int d) {
    static char str[11];

    str[0] = '0', str[1] = 'X', str[10] = 0;
    for (int i = 2; i < 10; i++) {
        str[i] = '0';
    }

    for (int p = 9; p > 1 && d > 0; p--) {
        int e = d % 16;
        d /= 16;
        if (e >= 10) {
            str[p] = 'A' + e - 10;
        } else {
            str[p] = '0' + e;
        }
    }

    return str;
}

void dead_loop(void) {
    for (;;)
        io_hlt();
}

void assert(bool cond, const char *errmsg) {
    if (!cond) {
        boxfill8(g_boot_info.m_vga_ram, g_boot_info.m_screen_x, COL8_000000, 0,
                 0, g_boot_info.m_screen_x, g_boot_info.m_screen_y);
        show_debug_string(0, 0, COL8_FFFFFF, errmsg);
        dead_loop();
    }
}
