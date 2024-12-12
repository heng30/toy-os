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

// 设置内存J
void memset(unsigned char *buf, unsigned char c, unsigned int n);

// 内存清零
void zero(unsigned char *buf, unsigned int n);
