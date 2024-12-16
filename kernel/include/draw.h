#pragma once

#include "def.h"
#include "win_sheet.h"

#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define HEX_INT_FONT_WIDTH 80

// 显示信息
typedef struct {
    unsigned char *m_vga_ram; // vga显存地址
    unsigned int m_screen_x;  // 屏幕宽
    unsigned int m_screen_y;  // 屏幕高
} boot_info_t;

// 显示信息
extern boot_info_t g_boot_info;

// 背景图层
extern win_sheet_t *g_background_sht;

// 初始化屏幕分辨率
void init_boot_info(void);

// 设置颜色板
void set_palette(unsigned int start, unsigned int end, unsigned char *rgb);

// 初始化颜色板
void init_palette(void);

// 绘制一个矩形
void boxfill8(unsigned char *vram, unsigned int xsize, unsigned char c,
              unsigned int x0, unsigned int y0, unsigned int x1,
              unsigned int y1);

// 根据buf来显示一个图像，通常用来显示鼠标
void put_block(unsigned char *vram, unsigned int xsize, unsigned int pxsize,
               unsigned int pysize, unsigned int px0, unsigned int py0,
               const char *buf, unsigned int bxsize);

// 显示1个字符
void show_font8(unsigned char *vram, unsigned int xsize, unsigned int x,
                unsigned int y, unsigned char c, const char *font);

// 显示一行字符串
void show_string(win_sheet_t *sht, unsigned int x, unsigned int y,
                 unsigned char bg_color, unsigned char text_color,
                 const char *s);

// 显示调试信息
void show_debug_char(unsigned char data);

// 显示调试信息
void show_debug_int(unsigned int data);

// 显示调试信息
void show_debug_string(unsigned int x, unsigned int y, unsigned char color,
                       const char *s);

// 绘制J背景
void init_background_sheet(void);

// 设置buffer所有值为c
void set_buffer(unsigned char *vram, unsigned int size, unsigned char c);

// 设置buffer的特定区域
void set_buffer_space(unsigned char *vram, unsigned int xsize, unsigned int x,
                      unsigned int y, unsigned int w, unsigned int h,
                      unsigned char c);

// 计算一个字符串占用的像素
unsigned int string_in_pixels(const char *s);

// 屏幕最大能显示的字体列数
unsigned int max_screen_font_column(void);

// 屏幕最大能显示的字体行数
unsigned int max_screen_font_rows(void);
