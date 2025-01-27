#pragma once

#include "win_sheet.h"

#define TITLE_BAR_HEIGHT 20  // 窗口标题栏高度
#define WINDOW_BORDER_SIZE 4 // 窗口边框大小

// 绘制输入框
// sht: 图层 x0: 起始位置 y0: 起始位置 sx: 宽 sy: 高 c：颜色
void make_textbox8(win_sheet_t *sht, unsigned int x0, unsigned int y0,
                   unsigned int sx, unsigned int sy, unsigned char c);

// 绘制窗口标题栏
void draw_title_bar(win_sheet_t *sht, const char *title,
                    unsigned char background_color);

// 绘制窗口背景
void draw_window_background(win_sheet_t *sht);
