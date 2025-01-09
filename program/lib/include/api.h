// 提供给C语言调用的系统调用函数

#pragma once

// 在终端打印一个字符
void api_console_draw_ch(char ch);

// 在终端打印字符串
void api_console_draw_text(const char *text);

// 创建一个新窗口, 返回窗口句柄
unsigned int api_new_window(unsigned int x, unsigned int y, unsigned int xsize,
                            unsigned int ysize, char *title);

// 刷新窗口指定区域
void api_refresh_windowwin(unsigned int win, unsigned int x0, unsigned int y0,
                           unsigned int x1, unsigned int y1);

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
