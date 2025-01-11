#pragma once

#include "def.h"

int max(int a, int b);
int min(int a, int b);

unsigned int max_unsigned(unsigned int a, unsigned int b);
unsigned int min_unsigned(unsigned int a, unsigned int b);

// if n <= l, return l;
// if n >= h, return h;
// if l < n < h, return n;
int bound(int n, int l, int h);
unsigned bound_unsigned(unsigned int n, unsigned int l, unsigned int h);

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

// 生成随机数
unsigned int gen_rand_num(unsigned int num);
