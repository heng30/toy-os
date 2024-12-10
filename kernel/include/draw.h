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

// 绘制一个像素
void draw_pixel(unsigned char *vram, int pos, unsigned char c);

// 绘制一个矩形
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0,
                 int x1, int y1);

// 根据buf来显示一个图像，通常用来显示鼠标
void put_block(unsigned char *vram, int xsize, int pxsize, int pysize, int px0,
               int py0, char *buf, int bxsize);

// 显示1个字符
void show_font8(unsigned char *vram, int xsize, int x, int y, char c, char *font);

// 显示字符串
void show_string(unsigned char* vram, int xisize, int x, int y, char color, const char *s);

// 显示调试信息
void show_debug_char(unsigned char data);

// 显示调试信息
void show_debug_int(unsigned int data);

// 设置背景图层
void set_background_vram(unsigned char *vram, int xsize, int ysize);

// 绘制J背景
void draw_background(void);

// 绘制背景
void draw_mouse(void);
