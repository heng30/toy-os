// 提供给C语言调用的系统调用函数

#pragma once

// 在终端打印一个字符
void api_console_draw_ch(char ch);

// 在终端打印字符串
void api_console_draw_text(const char *text);

// 创建一个新窗口, 返回窗口句柄
unsigned int api_new_window(unsigned int x, unsigned int y, unsigned int xsize,
                            unsigned int ysize, char *title);

// 在窗口中绘制字符串
void api_draw_text_in_window(unsigned int win, unsigned int x, unsigned int y,
                             unsigned char col, const char *str);

// 在窗口中绘制一个矩形
void api_draw_box_in_window(unsigned int win, unsigned int x0, unsigned int y0,
                            unsigned int x1, unsigned int y1,
                            unsigned char col);
