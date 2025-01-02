#pragma once

#include "def.h"

int max(int a, int b);

int min(int a, int b);

unsigned int max_unsigned(unsigned int a, unsigned int b);
unsigned int min_unsigned(unsigned int a, unsigned int b);

// 获取1个字符的16进制
char char2hex(char c);

// 获取1个字符的16进制字符串形式
char *char2hexstr(unsigned char c);

// 获取数字的16进制字符串形式
char *int2hexstr(unsigned int d);

// 死循环
void dead_loop(void);

// 断言
void assert(bool cond, const char *errmsg);

// 设置内存, 返回值指向buf开头
unsigned char *memset(unsigned char *buf, unsigned char c, unsigned int n);

// 内存清零
void zero(unsigned char *buf, unsigned int n);

// 复制内容，并返回dst指针
unsigned char *memcpy(unsigned char *dst, unsigned char *src, unsigned int len);

// 复制内容,并返回指向dst+len的指针
unsigned char *mempcpy(unsigned char *dst, unsigned char *src,
                       unsigned int len);

// 移动数据
void memmove(char *dst, const char *src, unsigned int size);

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

// if n <= l, return l;
// if n >= h, return h;
// if l < n < h, return n;
int bound(int n, int l, int h);
unsigned bound_unsigned(unsigned int n, unsigned int l, unsigned int h);

// 将秒数转换成时间，格式：`days hours:minutes:seconds`
char *seconds_to_time_string(unsigned int seconds, char time[16]);

// 数字转字符串
void int_to_string(int num, char *str);

// 数字转字符串
void uint_to_string(unsigned int num, char *str);
