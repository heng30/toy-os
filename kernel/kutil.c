#include "kutil.h"
#include "colo8.h"
#include "draw.h"
#include "io.h"

int max(int a, int b) { return a > b ? a : b; }
int min(int a, int b) { return a > b ? b : a; }
unsigned int max_unsigned(unsigned int a, unsigned int b) {
    return a > b ? a : b;
}
unsigned int min_unsigned(unsigned int a, unsigned int b) {
    return a > b ? b : a;
}

int bound(int n, int l, int h) {
    assert(l <= h, "bound assert failed");
    return min(max(n, l), h);
}

unsigned bound_unsigned(unsigned int n, unsigned int l, unsigned int h) {
    assert(l <= h, "bound assert failed");
    return min_unsigned(max_unsigned(n, l), h);
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

void memmove(char *dst, const char *src, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        dst[i] = src[i];
    }
}
