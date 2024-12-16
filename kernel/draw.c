#include "draw.h"
#include "colo8.h"
#include "def.h"
#include "io.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"
#include "win_sheet.h"

// 颜色板
extern unsigned char palette_table_rgb[16 * 3];

win_sheet_t *g_background_sht = NULL;
win_sheet_t *g_canvas_sht = NULL;

// 系统字体
extern char system_font[16];

// vga显示信息
boot_info_t g_boot_info = {
    .m_vga_ram = (unsigned char *)0xa0000, // 320x200显存起始地址
    .m_screen_x = 320,
    .m_screen_y = 200,
};

void init_boot_info(void) {
    // 640x480
    unsigned int *bi = get_boot_info();
    g_boot_info.m_vga_ram = (unsigned char *)(*bi);
    g_boot_info.m_screen_x = *(bi + 1);
    g_boot_info.m_screen_y = *(bi + 2);
}

void set_palette(unsigned int start, unsigned int end, unsigned char *rgb) {
    int eflags = io_load_eflags();
    io_cli();
    io_out8(0x03c8, (unsigned char)start);
    for (unsigned int i = start; i <= end; i++) {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);

        rgb += 3;
    }

    io_store_eflags(eflags);
    return;
}

void init_palette(void) {
    set_palette(0, 15, palette_table_rgb);
    return;
}

void boxfill8(unsigned char *vram, unsigned int xsize, unsigned char c,
              unsigned int x0, unsigned int y0, unsigned int x1,
              unsigned int y1) {
    for (unsigned int y = y0; y <= y1; y++)
        for (unsigned int x = x0; x <= x1; x++)
            vram[y * xsize + x] = c;
}

void put_block(unsigned char *vram, unsigned int xsize, unsigned int pxsize,
               unsigned int pysize, unsigned int px0, unsigned int py0,
               const char *buf, unsigned int bxsize) {
    for (unsigned int y = 0; y < pysize; y++)
        for (unsigned int x = 0; x < pxsize; x++) {
            vram[(py0 + y) * xsize + (px0 + x)] =
                (unsigned char)buf[y * bxsize + x];
        }
}

void show_font8(unsigned char *vram, unsigned int xsize, unsigned int x,
                unsigned int y, unsigned char c, const char *font) {
    for (unsigned int i = 0; i < FONT_HEIGHT; i++) {
        unsigned char d = (unsigned char)font[i];
        if ((d & 0x80) != 0) {
            vram[(y + i) * xsize + x + 0] = c;
        }
        if ((d & 0x40) != 0) {
            vram[(y + i) * xsize + x + 1] = c;
        }
        if ((d & 0x20) != 0) {
            vram[(y + i) * xsize + x + 2] = c;
        }
        if ((d & 0x10) != 0) {
            vram[(y + i) * xsize + x + 3] = c;
        }
        if ((d & 0x08) != 0) {
            vram[(y + i) * xsize + x + 4] = c;
        }
        if ((d & 0x04) != 0) {
            vram[(y + i) * xsize + x + 5] = c;
        }
        if ((d & 0x02) != 0) {
            vram[(y + i) * xsize + x + 6] = c;
        }
        if ((d & 0x01) != 0) {
            vram[(y + i) * xsize + x + 7] = c;
        }
    }
}

void show_string(win_sheet_t *sht, unsigned int x, unsigned int y,
                 unsigned char bg_color, unsigned char text_color,
                 const char *s) {
    unsigned int begin = x;

    for (; *s != 0x00; s++) {
        if (x + FONT_WIDTH > sht->m_bxsize) {
            break;
        }

        boxfill8(sht->m_buf, sht->m_bxsize, bg_color, x, y, x + FONT_WIDTH - 1,
                 y + FONT_HEIGHT - 1);

        show_font8(sht->m_buf, sht->m_bxsize, x, y, text_color,
                   system_font + *s * 16);

        x += FONT_WIDTH;
    }

    win_sheet_refresh(sht, begin, y, x, y + FONT_HEIGHT);
}

void show_debug_char(unsigned char data) {
    static unsigned int pos_x = 0;
    static unsigned int pos_y = 0;

    unsigned char *vram = g_boot_info.m_vga_ram;
    unsigned int xsize = g_boot_info.m_screen_x;
    unsigned int ysize = g_boot_info.m_screen_y;
    const char *pstr = char2hexstr(data);

    show_debug_string(pos_x, pos_y, COL8_FFFFFF, pstr);
    pos_x += 32;

    // 每10个data换行
    if (pos_x % xsize == 0) {
        pos_x = 0;
        pos_y += 16;
    }

    // 清除输出信息
    if (pos_y > ysize - 29) {
        pos_y = 0;
        boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
    }
}

void show_debug_int(unsigned int data) {
    static unsigned int pos_x = 0;
    static unsigned int pos_y = 0;

    unsigned char *vram = g_boot_info.m_vga_ram;
    unsigned int xsize = g_boot_info.m_screen_x;
    unsigned int ysize = g_boot_info.m_screen_y;
    char *pstr = int2hexstr(data);

    show_debug_string(pos_x, pos_y, COL8_FFFFFF, pstr);
    pos_x += 80;

    // 每4个data换行
    if (pos_x % xsize == 0) {
        pos_x = 0;
        pos_y += 16;
    }

    // 清除输出信息
    if (pos_y > ysize - 29) {
        pos_y = 0;
        boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
    }
}

void show_debug_string(unsigned int x, unsigned int y, unsigned char color,
                       const char *s) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    unsigned int xsize = g_boot_info.m_screen_x;

    for (; *s != 0x00; s++) {
        show_font8(vram, xsize, x, y, color, system_font + *s * 16);
        x += 8;
    }
}

static void _set_background_vram(unsigned char *vram, unsigned int xsize,
                                 unsigned int ysize) {
    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 28, xsize - 1, ysize - 28);
    boxfill8(vram, xsize, COL8_FFFFFF, 0, ysize - 27, xsize - 1, ysize - 27);
    boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 26, xsize - 1, ysize - 1);

    boxfill8(vram, xsize, COL8_FFFFFF, 3, ysize - 24, 59, ysize - 24);
    boxfill8(vram, xsize, COL8_FFFFFF, 2, ysize - 24, 2, ysize - 4);
    boxfill8(vram, xsize, COL8_848484, 3, ysize - 4, 59, ysize - 4);
    boxfill8(vram, xsize, COL8_848484, 59, ysize - 23, 59, ysize - 5);
    boxfill8(vram, xsize, COL8_000000, 2, ysize - 3, 59, ysize - 3);
    boxfill8(vram, xsize, COL8_000000, 60, ysize - 24, 60, ysize - 3);

    boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize - 4,
             ysize - 24);
    boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47,
             ysize - 4);
    boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize - 3, xsize - 4,
             ysize - 3);
    boxfill8(vram, xsize, COL8_FFFFFF, xsize - 3, ysize - 24, xsize - 3,
             ysize - 3);
}

void init_background_sheet(void) {
    unsigned int xsize = g_boot_info.m_screen_x, ysize = g_boot_info.m_screen_y;

    g_background_sht = win_sheet_alloc();
    assert(g_background_sht != NULL, "init_background_sht alloc sheet error");

    unsigned char *buf = (unsigned char *)memman_alloc_4k(xsize * ysize);
    assert(buf != NULL, "init_background_sht alloc 4k error");

    _set_background_vram(buf, xsize, ysize);
    win_sheet_setbuf(g_background_sht, "background", buf, xsize, ysize,
                     COLOR_INVISIBLE);
    win_sheet_slide(g_background_sht, 0, 0);
    win_sheet_show(g_background_sht, BOTTOM_WIN_SHEET_Z);
}

// vram: 需要修改的缓冲区
// size: 缓冲区大小
// c: 颜色值
void set_buffer(unsigned char *vram, unsigned int size, unsigned char c) {
    for (unsigned int i = 0; i < size; i++) {
        vram[i] = c;
    }
}

// vram: 需要修改的二维数组缓冲区
// xsize: 数组宽度
// x: 起始位置
// y: 起始位置
// w: 宽度
// h: 高度
// c: 颜色值
void set_buffer_space(unsigned char *vram, unsigned int xsize, unsigned int x,
                      unsigned int y, unsigned int w, unsigned int h,
                      unsigned char c) {

    for (unsigned int i = 0; i < h; i++) {
        for (unsigned int j = 0; j < w; j++) {
            unsigned int pos = xsize * (y + i) + x + j;
            vram[pos] = c;
        }
    }
}

unsigned int string_in_pixels(const char *s) { return strlen(s) * FONT_WIDTH; }

unsigned int max_screen_font_column(void) {
    return g_boot_info.m_screen_x / FONT_WIDTH;
}
unsigned int max_screen_font_rows(void) {
    return g_boot_info.m_screen_y / FONT_HEIGHT;
}
