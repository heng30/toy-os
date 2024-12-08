#include "colo8.h"
#include "io.h"

extern unsigned char palette_table_rgb[16 * 3];

void set_palette(int start, int end, unsigned char *rgb) {
    int eflags = io_load_eflags();
    io_cli();
    io_out8(0x03c8, start); // set  palette number
    for (int i = start; i <= end; i++) {
        io_out8(0x03c9, rgb[0]);
        io_out8(0x03c9, rgb[1]);
        io_out8(0x03c9, rgb[2]);

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

void write_vga_rectangle(void) {
    init_palette();

    unsigned char *p = (unsigned char *)0xa0000;
    boxfill8(p, 320, COL8_FF0000, 20, 20, 120, 120);
    boxfill8(p, 320, COL8_00FF00, 70, 50, 170, 150);
    boxfill8(p, 320, COL8_0000FF, 120, 80, 220, 180);

    for (;;)
        io_hlt();
}
