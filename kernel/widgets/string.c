#include "string.h"
#include "colo8.h"
#include "draw.h"
#include "kutil.h"

char char2hex(char c) {
    if (c >= 10) {
        return 'A' + c - 10;
    }

    return '0' + c;
}

char *char2hexstr(unsigned char c) {
    static char str[5] = {'0', 'X', 0, 0, 0};
    char m = (char)(c % 16);

    str[3] = char2hex(m);
    char n = (char)(c / 16);
    str[2] = char2hex(n);

    return str;
}

char *int2hexstr(unsigned int d) {
    static char str[11];

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

    return str;
}

unsigned int strlen(const char *s) {
    unsigned int total = 0;
    for (; *s != 0x00; s++) {
        total++;
    }
    return total;
}

char *stpcpy(char *dst, const char *src) {
    char *p = (char *)mempcpy((unsigned char *)dst, (unsigned char *)src,
                              strlen(src));
    *p = '\0';

    return p;
}

char *strcpy(char *dst, const char *src) {
    stpcpy(dst, src);
    return dst;
}

char *strncpy(char *dst, unsigned int dst_size, const char *src) {
    int len = min((int)strlen(src), (int)dst_size - 1);

    for (int i = 0; i < len; i++) {
        *(dst + i) = *(src + i);
    }

    if (len > 0)
        *(dst + len) = '\0';

    return dst;
}

char *strncpy_tail(char *dst, unsigned int dst_size, const char *src) {
    int src_len = (int)strlen(src);
    int len = min(src_len, (int)dst_size - 1);

    for (int i = len - 1, j = src_len - 1; i >= 0 && j >= 0; i--, j--) {
        *(dst + i) = *(src + j);
    }

    if (len > 0)
        *(dst + len) = '\0';

    return dst;
}

char *strcat(char *dst, const char *src) {
    stpcpy(dst + strlen(dst), src);
    return dst;
}

void strdup(char *dst, const char *src) {
    for (; *src != '\0'; src++, dst++)
        *dst = *src;

    *dst = '\0';
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

char to_lowercast(char ch) {
    if (ch >= 'A' && ch <= 'Z') {
        ch = 'a' + ch - 'A';
    }

    return ch;
}

char to_uppercast(char ch) {
    if (ch >= 'a' && ch <= 'z') {
        ch = 'A' + ch - 'a';
    }

    return ch;
}

int strcmp(const char *str1, const char *str2) {
    if (!str1 && !str2)
        return 0;
    else if (!str1)
        return -1;
    else if (!str2)
        return 1;

    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }

    if (*str1 == *str2)
        return 0;
    else if (!*str1)
        return -1;
    else
        return 1;
}

char *str_trim_space(char *text) {
    char *eof_text = text + strlen(text) - 1;
    char *s = text, *e = eof_text;

    // 跳过字符串开头的空字符
    while (*s == SPACE && *s != '\0')
        s++;

    // 跳过字符串结尾的空字符
    while (e > s && *e == SPACE)
        e--;

    // 空字符串
    if (*s == '\0') {
        *text = '\0';
        return text;
    }

    // 前后没有任何空格,不需要处理
    if (s == text && e == eof_text) {
        return text;
    } else if (s == text) { // 去除尾部的空格
        *(e + 1) = '\0';
    } else { // 去除开头和结尾的空格
        *(e + 1) = '\0';

        // 连同'\0'一起移动
        unsigned int size = (unsigned int)(e + 1 - s + 1);
        memmove(text, s, size);
    }

    return text;
}

bool str_start_with(const char *src, const char *pattern) {
    if (*src == '\0' && *pattern == '\0')
        return true;
    else if (*src == '\0')
        return false;
    else if (*pattern == '\0')
        return true;

    for (; *src != '\0' && *pattern != '\0'; src++, pattern++) {
        if (*src != *pattern)
            return false;
    }

    return *pattern == '\0';
}

bool str_end_with(const char *src, const char *pattern) {
    if (*src == '\0' && *pattern == '\0')
        return true;
    else if (*src == '\0')
        return false;
    else if (*pattern == '\0')
        return true;

    unsigned int src_len = strlen(src);
    unsigned int pattern_len = strlen(pattern);

    if (src_len < pattern_len)
        return false;

    src += (src_len - pattern_len);
    for (; *src != '\0' && *pattern != '\0'; src++, pattern++) {
        if (*src != *pattern)
            return false;
    }

    return *pattern == '\0';
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

void uint_to_string(unsigned int num, char *str) {
    int i = 0;

    do {
        str[i++] = (char)(num % 10) + '0';
        num /= 10;
    } while (num > 0);

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

char *seconds_to_time_string(unsigned int seconds, char time[16]) {
    unsigned int days = seconds / (24 * 60 * 60);
    seconds %= (24 * 60 * 60);

    unsigned int hours = seconds / (60 * 60);
    seconds %= (60 * 60);

    unsigned int minutes = seconds / 60;
    unsigned int secs = seconds % 60;
    time[0] = '\0';

    // WHY: 这里不能使用栈上的空间，会有问题。
    static char _seconds_to_time_string_buf[8];
    unsigned int items[4] = {days, hours, minutes, secs};

    for (unsigned int i = 0; i < 4; i++) {
        uint_to_string(items[i], _seconds_to_time_string_buf);
        if (_seconds_to_time_string_buf[1] == '\0') {
            strpush(time, '0');
            strpush(time, _seconds_to_time_string_buf[0]);
        } else {
            strcat(time, _seconds_to_time_string_buf);
        }

        if (i == 0) {
            strpush(time, ' ');
        } else if (i != 3) {
            strpush(time, ':');
        }
    }

    return time;
}
