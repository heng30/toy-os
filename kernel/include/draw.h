#pragma once

#include "def.h"
#include "win_sheet.h"

#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define HEX_INT_FONT_WIDTH 80

// 显示信息
typedef struct {
    unsigned char *m_vga_ram; // vga显存地址
    short m_screen_x;         // 屏幕宽
    short m_screen_y;         // 屏幕高
} boot_info_t;

// 显示信息
extern boot_info_t g_boot_info;

// 背景图层
extern win_sheet_t *g_background_sht;

// 画布图层
extern win_sheet_t *g_canvas_sht;

// 设置颜色板
void set_palette(int start, int end, unsigned char *rgb);

// 初始化颜色板
void init_palette(void);

// 绘制一个像素
void draw_pixel(unsigned char *vram, int pos, unsigned char c);

// 绘制一个矩形
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0,
              int x1, int y1);

// 根据buf来显示一个图像，通常用来显示鼠标
void put_block(unsigned char *vram, int xsize, int pxsize, int pysize, int px0,
               int py0, char *buf, int bxsize);

// 显示1个字符
void show_font8(unsigned char *vram, int xsize, int x, int y, char c,
                char *font);

// 显示一行字符串
void show_string(win_sheet_t *sht, int x, int y, char bg_color, char text_color, const char *s);

// 显示调试信息
void show_debug_char(unsigned char data);

// 显示调试信息
void show_debug_int(unsigned int data);

// 显示调试信息
void show_debug_string(int x, int y, char color, const char *s);

// 绘制J背景
void init_background_sheet(void);

// 创建绘制面板
void init_canvas_sheet(int z);

// 清空图层所有数据，使图层透明
void clear_sheet(unsigned char *vram, int size, unsigned char c);

// 绘制字符串到调试面板
void show_string_in_canvas(int x, int y, char color, const char *s);
