#include "colo8.h"
#include "io.h"

typedef struct {
    unsigned char *m_vga_ram;
    short m_screen_x;
    short m_screen_y;
} boot_info_t;

typedef struct {
    unsigned int m_base_addr_low;
    unsigned int m_base_addr_high;
    unsigned int m_length_low;
    unsigned int m_length_high;
    unsigned int m_type;
} addr_range_desc_t;

extern char system_font[16];
extern char cursor_icon[16][16];
extern unsigned char palette_table_rgb[16 * 3];

// vga显示信息
static boot_info_t g_boot_info = {
    .m_vga_ram = (unsigned char *)0xa0000,
    .m_screen_x = 320,
    .m_screen_y = 200,
};

// 鼠标相关
static const int g_xsize = 320, g_ysize = 200; // 屏幕大小
static char g_mcursor[256];
static char g_keyval[5] = {'0', 'X', 0, 0, 0};

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

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0,
              int x1, int y1) {
    // use volatile to prevent optimise to `memset` function
    for (volatile int y = y0; y <= y1; y++)
        for (volatile int x = x0; x <= x1; x++)
            vram[y * xsize + x] = c;
}

void show_font8(unsigned char *vram, int xsize, int x, int y, char c,
                char *font) {
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

void show_string(unsigned char *vram, int xsize, int x, int y, char color,
                 const char *s) {
    for (; *s != 0x00; s++) {
        show_font8(vram, xsize, x, y, color, system_font + *s * 16);
        x += 8;
    }
}

void init_mouse_cursor(char *mouse, char bc) {
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (cursor_icon[y][x] == '*') {
                mouse[y * 16 + x] = COL8_000000;
            }
            if (cursor_icon[y][x] == 'O') {
                mouse[y * 16 + x] = COL8_FFFFFF;
            }
            if (cursor_icon[y][x] == '.') {
                mouse[y * 16 + x] = bc;
            }
        }
    }
}

void put_block(unsigned char *vram, int vxsize, int pxsize, int pysize, int px0,
               int py0, char *buf, int bxsize) {
    for (volatile int y = 0; y < pysize; y++)
        for (volatile int x = 0; x < pxsize; x++) {
            vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
        }
}

void write_vga_desktop_background(void) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;

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

char char2hex(char c) {
    if (c >= 10) {
        return 'A' + c - 10;
    }

    return '0' + c;
}

char* char2hexstr(unsigned char c) {
    char m = c % 16;
    g_keyval[3] = char2hex(m);
    c = c / 16;
    g_keyval[2] = char2hex(c);

    return g_keyval;
}

char *int2hexstr(unsigned int d) {
    static char str[11];

    str[0] = '0', str[1] = 'X', str[10] = 0;
    for (int i = 2; i < 10; i++) {
        str[i] = '0';
    }

    for (int p = 9; p > 1 && d > 0; p--) {
        int e = d % 16;
        d /= 16;
        if (e >= 10) {
            str[p] = 'A' + e - 10;
        } else {
            str[p] = '0' + e;
        }
    }

    return str;
}

void show_debug_info(unsigned char data) {
    static int pos_x = 0;
    static int pos_y = 0;
    char *pstr = char2hexstr(data);
    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;

    show_string(vram, xsize, pos_x, pos_y, COL8_FFFFFF, pstr);
    pos_x += 32;

    // 每10个字符换行
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

void show_memory_block_counts(void) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int cnt = get_memory_block_count();
    char *p = int2hexstr(cnt);
    show_string(vram, xsize, 0, 0, COL8_FFFFFF, p);
}

void show_memory_block_info(addr_range_desc_t *desc, unsigned char *vram,
                            int page, int xsize, int color) {
    int x = 0, y = 0, gap = 13 * 8;

    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize, 100);

    show_string(vram, xsize, x, y, color, "page is: ");
    char *p_page_cnt = int2hexstr(page);
    show_string(vram, xsize, gap, y, color, p_page_cnt);
    y += 16;

    show_string(vram, xsize, x, y, color, "base_addr_L: ");
    char *p_base_addr_l = int2hexstr(desc->m_base_addr_low);
    show_string(vram, xsize, gap, y, color, p_base_addr_l);
    y += 16;

    show_string(vram, xsize, x, y, color, "base_addr_h: ");
    char *p_base_addr_h = int2hexstr(desc->m_base_addr_high);
    show_string(vram, xsize, gap, y, color, p_base_addr_h);

    y += 16;
    show_string(vram, xsize, x, y, color, "length_low: ");
    char *p_length_low = int2hexstr(desc->m_length_low);
    show_string(vram, xsize, gap, y, color, p_length_low);

    y += 16;
    show_string(vram, xsize, x, y, color, "length_high: ");
    char *p_length_high = int2hexstr(desc->m_length_high);
    show_string(vram, xsize, gap, y, color, p_length_high);

    y += 16;
    show_string(vram, xsize, x, y, color, "type: ");
    char *ptype = int2hexstr(desc->m_type);
    show_string(vram, xsize, gap, y, color, ptype);
}
