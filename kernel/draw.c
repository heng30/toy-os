#include "draw.h"
#include "colo8.h"
#include "io.h"
#include "kutil.h"

// 颜色板
extern unsigned char palette_table_rgb[16 * 3];

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

void boxfill8(unsigned char c, int x0, int y0, int x1, int y1) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;

    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
            vram[y * xsize + x] = c;
}

void show_font8(int x, int y, char c, char *font) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;

    for (int i = 0; i < 16; i++) {
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

void show_string(int x, int y, char color, const char *s) {
    for (; *s != 0x00; s++) {
        show_font8(x, y, color, system_font + *s * 16);
        x += 8;
    }
}

void put_block(int pxsize, int pysize, int px0, int py0, char *buf,
               int bxsize) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;

    for (int y = 0; y < pysize; y++)
        for (int x = 0; x < pxsize; x++) {
            vram[(py0 + y) * xsize + (px0 + x)] = buf[y * bxsize + x];
        }
}

void draw_background(void) {
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;

    boxfill8(COL8_008484, 0, 0, xsize - 1, ysize - 29);
    boxfill8(COL8_C6C6C6, 0, ysize - 28, xsize - 1, ysize - 28);
    boxfill8(COL8_FFFFFF, 0, ysize - 27, xsize - 1, ysize - 27);
    boxfill8(COL8_C6C6C6, 0, ysize - 26, xsize - 1, ysize - 1);

    boxfill8(COL8_FFFFFF, 3, ysize - 24, 59, ysize - 24);
    boxfill8(COL8_FFFFFF, 2, ysize - 24, 2, ysize - 4);
    boxfill8(COL8_848484, 3, ysize - 4, 59, ysize - 4);
    boxfill8(COL8_848484, 59, ysize - 23, 59, ysize - 5);
    boxfill8(COL8_000000, 2, ysize - 3, 59, ysize - 3);
    boxfill8(COL8_000000, 60, ysize - 24, 60, ysize - 3);

    boxfill8(COL8_848484, xsize - 47, ysize - 24, xsize - 4, ysize - 24);
    boxfill8(COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize - 4);
    boxfill8(COL8_FFFFFF, xsize - 47, ysize - 3, xsize - 4, ysize - 3);
    boxfill8(COL8_FFFFFF, xsize - 3, ysize - 24, xsize - 3, ysize - 3);
}

void show_debug_info(unsigned char data) {
    static int pos_x = 0;
    static int pos_y = 0;
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;
    char *pstr = char2hexstr(data);

    show_string(pos_x, pos_y, COL8_FFFFFF, pstr);
    pos_x += 32;

    // 每10个字符换行
    if (pos_x % xsize == 0) {
        pos_x = 0;
        pos_y += 16;
    }

    // 清除输出信息
    if (pos_y > ysize - 29) {
        pos_y = 0;
        boxfill8(COL8_008484, 0, 0, xsize - 1, ysize - 29);
    }
}
