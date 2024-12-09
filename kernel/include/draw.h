#pragma once

// 显示信息
typedef struct {
    unsigned char *m_vga_ram; // vga显存地址
    short m_screen_x;         // 屏幕宽
    short m_screen_y;         // 屏幕高
} boot_info_t;

// 显示信息
extern boot_info_t g_boot_info;

// 设置颜色板
void set_palette(int start, int end, unsigned char *rgb);

// 初始化颜色板
void init_palette(void);

// 绘制一个矩形
void boxfill8(unsigned char c, int x0, int y0, int x1, int y1);
void boxfill8_v2(unsigned char *buf, int xsize, unsigned char c, int x0, int y0,
                 int x1, int y1);

// 显示1个字符
void show_font8(int x, int y, char c, char *font);

// 显示字符串
void show_string(int x, int y, char color, const char *s);

// 根据buf来显示一个图像，通常用来显示鼠标
void put_block(int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);

// 绘制背景
void draw_background(void);

// 显示调试信息
void show_debug_char(unsigned char data);

// 显示调试信息
void show_debug_int(unsigned int data);
