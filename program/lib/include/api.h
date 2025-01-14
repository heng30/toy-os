// 提供给C语言调用的系统调用函数

#pragma once

#include "pdef.h"

// 在终端打印一个字符
void api_console_draw_ch(char ch);

// 在终端打印字符串
void api_console_draw_text(const char *text, bool is_literal);

// 创建一个新窗口, 返回窗口句柄
unsigned int api_new_window(unsigned int x, unsigned int y, unsigned int xsize,
                            unsigned int ysize, char *title);

// 关闭窗口
void api_close_window(unsigned int win);

// 刷新窗口指定区域
void api_refresh_window(unsigned int win, unsigned int x0, unsigned int y0,
                        unsigned int x1, unsigned int y1);

// 检查是否应该关闭程序
bool api_is_close_window(void);

// 在窗口中绘制字符串, 这个字符串要在代码段
// col 的高8字节为背景色，低8字节为前景色
void api_draw_text_in_window(unsigned int win, unsigned int x, unsigned int y,
                             unsigned short col, const char *str,
                             bool is_literal);

// 在窗口中绘制一个矩形
void api_draw_box_in_window(unsigned int win, unsigned int x0, unsigned int y0,
                            unsigned int x1, unsigned int y1,
                            unsigned char col);

// 在窗口绘制一个像素点
void api_draw_point_in_window(unsigned int win, unsigned int x, unsigned int y,
                              unsigned char col);

// 在窗口绘制一条直线
void api_draw_line_in_window(unsigned int win, unsigned int x0, unsigned int y0,
                             unsigned int x1, unsigned int y1,
                             unsigned char col);

// 获取图层的副本
bool api_dump_window_sheet(unsigned int win, unsigned int x0, unsigned int y0,
                           unsigned int x1, unsigned int y1,
                           unsigned char *buf);

// 覆盖指定图层区域
void api_cover_window_sheet(unsigned int win, unsigned int x0, unsigned int y0,
                            unsigned int x1, unsigned int y1,
                            unsigned char *buf);

// 获取一个随机数
unsigned int api_rand_uint(unsigned int seed);

// 显示一个调试数字
void api_show_debug_uint(unsigned int x, unsigned int y, unsigned int num);

// 创建一个定时器
unsigned int api_timer_alloc(void);

// 释放一个定时器
void api_timer_free(unsigned int timer);

// 设置定时器
void api_timer_set(unsigned int timer, unsigned int timeout,
                   unsigned int run_count);

// 判断定时器是否超时了
bool api_timer_is_timeout(unsigned int timer);

// 打开文件
// is_literal: 传入的文件名是否字符串常量，字符串常量在代码段, 否则在数据段
int api_file_open(const char *filename, bool is_literal);

// 读取文件
// buf: 存放内容的缓冲区
// len: 读取数据量
// pos：开始读取的位置, 从0开始
int api_file_read(int fd, unsigned char *buf, unsigned int len,
                  unsigned int pos);

// 写入文件
// buf: 存放内容的缓冲区
// len: 写入数据量
// pos：开始写入的位置, 从0开始
int api_file_write(int fd, unsigned char *buf, unsigned int len,
                   unsigned int pos);

// 获取文件大小
int api_file_size(int fd);

// 关闭文件
void api_file_close(int fd);

// 获取代码段的常量字符串长度
unsigned int api_strlen_cs(const char *s);

// 复制代码段常量字符串到数据段空间
unsigned char *api_memcpy_cs2ds(unsigned char *dst, unsigned char *src,
                                unsigned int len);
