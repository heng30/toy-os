#pragma once

#include "def.h"

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
unsigned char* memset(unsigned char *buf, unsigned char c, unsigned int n);

// 内存清零
void zero(unsigned char *buf, unsigned int n);

// 复制内容，并返回dst指针
unsigned char* memcpy(unsigned char *dst, unsigned char *src, unsigned int len);

// 复制内容,并返回指向dst+len的指针
unsigned char *mempcpy(unsigned char *dst, unsigned char *src,
                       unsigned int len);

// 获取字符串长度
unsigned int strlen(const char *s);

// 复制字符串，并在dst末尾添加上`\0`， 返回值指向des的`\0`处
char *stpcpy(char *dst, const char *src);

// 复制字符串，并在dst末尾添加上`\0`， 返回值指向des开头处
char *strcpy(char *dst, const char *src);

// 拼接字符串，并在dst末尾添加上`\0`， 返回值指向des开头处
char *strcat(char *dst, const char *src);
