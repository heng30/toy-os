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
    .m_vga_ram = (unsigned char *)0xa0000,
    .m_screen_x = 320,
    .m_screen_y = 200,
};

void set_palette(int start, int end, unsigned char *rgb) {
    int eflags = io_load_eflags();
    io_cli();
    io_out8(0x03c8, start);
    for (int i = start; i <= end; i++) {
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

void draw_pixel(unsigned char *vram, int pos, unsigned char c) {
    vram[pos] = c;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0,
              int x1, int y1) {
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
            vram[y * xsize + x] = c;
}

void put_block(unsigned char *vram, int xsize, int pxsize, int pysize, int px0,
               int py0, char *buf, int bxsize) {
    for (int y = 0; y < pysize; y++)
        for (int x = 0; x < pxsize; x++) {
            vram[(py0 + y) * xsize + (px0 + x)] = buf[y * bxsize + x];
        }
}

void show_font8(unsigned char *vram, int xsize, int x, int y, char c,
                char *font) {
    for (int i = 0; i < FONT_HEIGHT; i++) {
        char d = font[i];
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

void show_string(win_sheet_t *sht, int x, int y, char bg_color, char text_color,
                 const char *s) {
    int begin = x;

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
    static int pos_x = 0;
    static int pos_y = 0;

    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;
    char *pstr = char2hexstr(data);

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
    static int pos_x = 0;
    static int pos_y = 0;

    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;
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

void show_debug_string(int x, int y, char color, const char *s) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;

    for (; *s != 0x00; s++) {
        show_font8(vram, xsize, x, y, color, system_font + *s * 16);
        x += 8;
    }
}

static void _set_background_vram(unsigned char *vram, int xsize, int ysize) {
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
    int xsize = g_boot_info.m_screen_x, ysize = g_boot_info.m_screen_y;

    g_background_sht = win_sheet_alloc();
    assert(g_background_sht != NULL, "init_background_sht alloc sheet error");

    unsigned char *buf = (unsigned char *)memman_alloc_4k(xsize * ysize);
    assert(buf != NULL, "init_background_sht alloc 4k error");

    _set_background_vram(buf, xsize, ysize);
    win_sheet_setbuf(g_background_sht, buf, xsize, ysize, COLOR_INVISIBLE);
    win_sheet_slide(g_background_sht, 0, 0);
    win_sheet_updown(g_background_sht, BOTTOM_WIN_SHEET_Z);
}

void clear_sheet(unsigned char *vram, int size, unsigned char c) {
    for (int i = 0; i < size; i++) {
        vram[i] = c;
    }
}

void init_canvas_sheet(int z) {
    int xsize = g_boot_info.m_screen_x, ysize = g_boot_info.m_screen_y;
    unsigned char *buf = (unsigned char *)memman_alloc_4k(xsize * ysize);
    assert(buf != NULL, "show_string_in_canvas memory alloc error");

    clear_sheet(buf, xsize * ysize, COLOR_INVISIBLE);

    g_canvas_sht = win_sheet_alloc();
    assert(g_canvas_sht != NULL, "show_string_in_canvas sheet alloc error");

    g_canvas_sht->m_is_transparent_layer = true;
    win_sheet_setbuf(g_canvas_sht, buf, xsize, ysize, COLOR_INVISIBLE);
    win_sheet_slide(g_canvas_sht, 0, 0);
    win_sheet_updown(g_canvas_sht, z);
}

void show_string_in_canvas(int x, int y, char color, const char *s) {
    if (!g_canvas_sht)
        return;

    show_string(g_canvas_sht, x, y, COLOR_INVISIBLE, color, s);
}
