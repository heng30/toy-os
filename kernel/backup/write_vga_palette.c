#include "io.h"

extern unsigned char palette_table_rgb[16 * 3];

void set_palette(int start, int end, unsigned char *rgb) {
    int eflags = io_load_eflags();
    io_cli(); // 关中断

    io_out8(0x03c8, start); // set  palette number
    for (int i = start; i <= end; i++) {
        io_out8(0x03c9, rgb[0]);
        io_out8(0x03c9, rgb[1]);
        io_out8(0x03c9, rgb[2]);

        rgb += 3;
    }

    // 恢复eflags，会设置中断寄存器位，从而开中断
    io_store_eflags(eflags);
    return;
}

void init_palette(void) {
    set_palette(0, 15, palette_table_rgb);
    return;
}

void write_vga_palette(void) {
    init_palette();

    for (int i = 0xa0000; i <= 0xaffff; i++)
        *(unsigned char *)i = i & 0x0f;

    for (;;)
        io_hlt();
}
