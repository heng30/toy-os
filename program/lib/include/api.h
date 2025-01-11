// 提供给C语言调用的系统调用函数

#pragma once

#include "pdef.h"

// 在终端打印一个字符
void api_console_draw_ch(char ch);

// 在终端打印字符串
void api_console_draw_text(const char *text);

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

// 在窗口中绘制字符串
// col 的高8字节为背景色，低8字节为前景色
void api_draw_text_in_window(unsigned int win, unsigned int x, unsigned int y,
                             unsigned short col, const char *str);

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

// 获取一个随机数
unsigned int api_rand_uint(unsigned int seed);

// 显示一个调试数字
void api_show_debug_uint(unsigned int x, unsigned int y, unsigned int num);

// 创建一个定时器
unsigned int api_timer_alloc(void);

// 释放一个定时器
void api_timer_free(unsigned int timer);

// 设置定时器
void api_timer_set(unsigned int timer, unsigned int timeout, unsigned int run_count);

// 判断定时器是否超时了
bool api_timer_is_timeout(unsigned int timer);
