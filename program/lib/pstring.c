#include "pstring.h"
#include "puitl.h"

void int2hexstr(unsigned int d, char str[11]) {
    str[0] = '0', str[1] = 'X', str[10] = 0;
    for (int i = 2; i < 10; i++) {
        str[i] = '0';
    }

    for (int p = 9; p > 1 && d > 0; p--) {
        char e = d % 16;
        d /= 16;
        if (e >= 10) {
            str[p] = 'A' + e - 10;
        } else {
            str[p] = '0' + e;
        }
    }
}

void int_to_string(int num, char *str) {
    int i = 0, is_negative = 0;

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    do {
        str[i++] = (char)(num % 10) + '0';
        num /= 10;
    } while (num > 0);

    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // 翻转字符串
    int left = 0, right = i - 1;
    while (left < right) {
        char temp = str[left];
        str[left] = str[right];
        str[right] = temp;
        left++, right--;
    }
}

char *stpcpy(char *dst, const char *src) {
    char *p = (char *)mempcpy((unsigned char *)dst, (unsigned char *)src,
                              strlen(src));
    *p = '\0';

    return p;
}

unsigned int strlen(const char *s) {
    unsigned int total = 0;
    for (; *s != 0x00; s++) {
        total++;
    }
    return total;
}

char *strcpy(char *dst, const char *src) {
    stpcpy(dst, src);
    return dst;
}

char *strcat(char *dst, const char *src) {
    stpcpy(dst + strlen(dst), src);
    return dst;
}

void clean_str(char *str) {
    unsigned int len = strlen(str);

    if (len == 0)
        return;

    if (str[len - 1] == 0x0a)
        str[len - 1] = '\0';
}
