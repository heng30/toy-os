#include "kutil.h"
#include "colo8.h"
#include "draw.h"
#include "io.h"

int max(int a, int b) { return a > b ? a : b; }

int min(int a, int b) { return a > b ? b : a; }

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

unsigned char *memset(unsigned char *buf, unsigned char c, unsigned int n) {
    for (unsigned int i = 0; i < n; i++) {
        *(buf + i) = c;
    }
    return buf;
}

void zero(unsigned char *buf, unsigned int n) {
    for (unsigned int i = 0; i < n; i++) {
        *(buf + i) = 0;
    }
}

unsigned char *memcpy(unsigned char *dst, unsigned char *src,
                      unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
        *(dst + i) = *(src + i);
    }
    return dst;
}

unsigned char *mempcpy(unsigned char *dst, unsigned char *src,
                       unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
        *(dst + i) = *(src + i);
    }
    return dst + len;
}

unsigned int strlen(const char *s) {
    unsigned int total = 0;
    for (; *s != 0x00; s++) {
        total++;
    }
    return total;
}

char *stpcpy(char *dst, const char *src) {
    char *p = mempcpy((unsigned char *)dst, (unsigned char *)src, strlen(src));
    *p = '\0';

    return p;
}

char *strcpy(char *dst, const char *src) {
    stpcpy(dst, src);
    return dst;
}

char *strncpy(char *dst, unsigned int dst_size, const char *src) {
    unsigned int len = min(strlen(src), dst_size - 1);

    for (unsigned int i = 0; i < len; i++) {
        *(dst + i) = *(src + i);
    }

    if (len > 0)
        *(dst + len) = '\0';

    return dst;
}

char *strcat(char *dst, const char *src) {
    stpcpy(dst + strlen(dst), src);
    return dst;
}

char *strpush(char *dst, const char c) {
    unsigned int len = strlen(dst);
    *(dst + len) = c;
    *(dst + len + 1) = '\0';
    return dst;
}

char *strpop(char *dst) {
    unsigned int len = strlen(dst);
    if (len == 0)
        *dst = '\0';
    else
        *(dst + len - 1) = '\0';

    return dst;
}
