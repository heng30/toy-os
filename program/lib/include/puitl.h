#pragma once

int max(int a, int b);
int min(int a, int b);

// 延时
void delay(unsigned int loop);

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
