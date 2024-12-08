#include "colo8.h"
#include "io.h"

typedef struct boot_info {
    unsigned char *m_vga_ram;
    short m_screen_x;
    short m_screen_y;
} boot_info_t;

extern char system_font[16];
extern unsigned char palette_table_rgb[16 * 3];
extern char cursor_icon[16][16];

static char mcursor[256];
static boot_info_t boot_info;

void init_boot_info(boot_info_t *p) {
    p->m_vga_ram = (unsigned char *)0xa0000;
    p->m_screen_x = 320;
    p->m_screen_y = 200;
}

void set_palette(int start, int end, unsigned char *rgb) {
    int eflags = io_load_eflags();
    io_cli();
    io_out8(0x03c8, start); // set  palette number
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

void write_vga_desktop_int_handler(void) {
    init_boot_info(&boot_info);

    unsigned char *vram = boot_info.m_vga_ram;
    int xsize = boot_info.m_screen_x;
    int ysize = boot_info.m_screen_y;

    init_palette();

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

    init_mouse_cursor(mcursor, COL8_008484);
    put_block(vram, xsize, 16, 16, 80, 80, mcursor, 16);

    // 开中断
    io_sti();

    for (;;)
        io_hlt();
}

void int_handler_from_c(char *esp) {
    unsigned char *vram = boot_info.m_vga_ram;
    int xsize = boot_info.m_screen_x;
    int ysize = boot_info.m_screen_y;

    boxfill8(vram, xsize, COL8_000000, 0, 0, 32 * 8 - 1, 15);
    show_string(vram, xsize, 0, 0, COL8_FFFFFF, "PS/2 keyboard");
    for (;;)
        io_hlt();
}
