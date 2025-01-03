#pragma once

#include "def.h"

// 获取1个字符的16进制
char char2hex(char c);

// 获取1个字符的16进制字符串形式
char *char2hexstr(unsigned char c);

// 获取数字的16进制字符串形式
char *int2hexstr(unsigned int d);

// 获取字符串长度
unsigned int strlen(const char *s);

// 复制字符串，并在dst末尾添加上`\0`， 返回值指向des的`\0`处
char *stpcpy(char *dst, const char *src);

// 复制n个字符，并在dst末尾添加上`\0`
char *strncpy(char *dst, unsigned int dst_size, const char *src);

// 复制最后n个字符，并在dst末尾添加上`\0`
char *strncpy_tail(char *dst, unsigned int dst_size, const char *src);

// 复制字符串，并在dst末尾添加上`\0`， 返回值指向des开头处
char *strcpy(char *dst, const char *src);

// 拼接字符串，并在dst末尾添加上`\0`， 返回值指向des开头处
char *strcat(char *dst, const char *src);

// 复制字符串
void strdup(char *dst, const char *src);

// 拼接字符串，并在dst末尾添加上`\0`， 返回值指向des开头处
char *strpush(char *dst, char c);

// 移除尾部一个字符， 返回值指向des开头处
char *strpop(char *dst);

// 转为小写字符
char to_lowercast(char ch);

// 转为大写字符
char to_uppercast(char ch);

// 判断字符串大小， 1: str1 > str2; 0: str1 == str2; 1: str1 < str2
int strcmp(const char *str1, const char *str2);

// 移除字符串前后的空字符
char* str_trim_space(char *text);

// 判断一个字符是否以某个字符串开头
bool str_start_with(const char *src, const char *pattern);

// 判断一个字符是否以某个字符串结尾
bool str_end_with(const char *src, const char *pattern);

// 将秒数转换成时间，格式：`days hours:minutes:seconds`
char *seconds_to_time_string(unsigned int seconds, char time[16]);

// 数字转字符串
void int_to_string(int num, char *str);

// 数字转字符串
void uint_to_string(unsigned int num, char *str);
