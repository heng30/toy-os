#include "pstring.h"

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
