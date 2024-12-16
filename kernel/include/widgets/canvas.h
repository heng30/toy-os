#pragma once

#include "win_sheet.h"

// 画布图层
extern win_sheet_t *g_canvas_sht;

// 创建绘制面板
void init_canvas_sheet(int z);

// 绘制字符串到调试面板
void show_string_in_canvas(unsigned int x, unsigned int y, unsigned char color,
                           const char *s);

// 清空canvas上所有的信息
void clear_canvas(void);

// 清空canvas的特定区域
void clear_canvas_space(unsigned int x, unsigned int y, unsigned int w,
                        unsigned int h);
