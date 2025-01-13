#pragma once

void int2hexstr(unsigned int d, char *str);

void int_to_string(int num, char *str);

char *stpcpy(char *dst, const char *src);

unsigned int strlen(const char *s);

char *strcpy(char *dst, const char *src);

char *strcat(char *dst, const char *src);

// 清楚字符串结尾处的'0x0a'字符
void clean_str(char* str);
