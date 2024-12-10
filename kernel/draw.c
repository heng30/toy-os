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

void show_string(win_sheet_t *sht, int x, int y, char color, const char *s) {
    int begin = x;
    int max_size = sht->m_bxsize * sht->m_bysize;

    for (; *s != 0x00; s++) {
        if (x + FONT_WIDTH > sht->m_bxsize) {
            break;
        }

        show_font8(sht->m_buf, sht->m_bxsize, x, y, color,
                   system_font + *s * 16);
        x += 8;
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

void draw_background(void) {
    int xsize = g_boot_info.m_screen_x, ysize = g_boot_info.m_screen_y;

    static unsigned char *buf = NULL;
    static win_sheet_t *sht = NULL;

    if (!buf) {
        buf = (unsigned char *)memman_alloc_4k(xsize * ysize);

        if (!buf) {
            return;
        }
    }

    if (!sht) {
        sht = win_sheet_alloc();

        if (!sht) {
            memman_free_4k(buf, xsize * ysize);
            return;
        }

        _set_background_vram(buf, xsize, ysize);
        win_sheet_setbuf(sht, buf, xsize, ysize, COLOR_INVISIBLE);
        win_sheet_slide(sht, 0, 0);
        win_sheet_updown(sht, BOTTOM_WIN_SHEET_HEIGHT);
    }
}

void draw_mouse(void) {
    static win_sheet_t *sht = NULL;

    if (!sht) {
        sht = win_sheet_alloc();
        win_sheet_setbuf(sht, g_mdec.m_cursor, CURSOR_ICON_SIZE,
                         CURSOR_ICON_SIZE, COLOR_INVISIBLE);

        win_sheet_slide(sht, g_mdec.m_abs_x, g_mdec.m_abs_y);
        win_sheet_updown(sht, TOP_WIN_SHEET_HEIGHT);
    } else {
        compute_mouse_position();
        win_sheet_slide(sht, g_mdec.m_abs_x, g_mdec.m_abs_y);
    }
}

void clear_win_sheet(unsigned char *vram, int size) {
    for (int i = 0; i < size; i++) {
        vram[i] = COLOR_INVISIBLE;
    }
}

void show_string_in_canvas(int x, int y, char color, const char *s,
                           bool is_clear, int height) {
    int xsize = g_boot_info.m_screen_x, ysize = g_boot_info.m_screen_y;

    static unsigned char *buf = NULL;
    static win_sheet_t *sht = NULL;

    if (!buf) {
        buf = (unsigned char *)memman_alloc_4k(xsize * ysize);

        if (!buf) {
            return;
        }
    }

    if (is_clear)
        clear_win_sheet(buf, xsize * ysize);

    if (!sht) {
        sht = win_sheet_alloc();

        if (!sht) {
            memman_free_4k(buf, xsize * ysize);
            return;
        }

        win_sheet_setbuf(sht, buf, xsize, ysize, COLOR_INVISIBLE);
        win_sheet_slide(sht, 0, 0);

        if (height >= TOP_WIN_SHEET_HEIGHT)
            height = TOP_WIN_SHEET_HEIGHT - 1;

        win_sheet_updown(sht, height);
    }

    show_string(sht, x, y, color, s);
}
