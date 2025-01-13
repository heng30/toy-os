#include "api.h"
#include "pdef.h"
#include "putil.h"

int max(int a, int b) { return a > b ? a : b; }
int min(int a, int b) { return a > b ? b : a; }

void delay(unsigned int loop) {
    for (unsigned int i = 0; i != loop; i++)
        ;
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

void window_exit_wait(void) {
    while (true) {
        if (api_is_close_window())
            break;

        delay(DEFAULT_DELAY_LOOP);
    }
}
