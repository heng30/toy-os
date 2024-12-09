#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "io.h"
#include "keyboard.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"
#include "win_sheet.h"

void show_mouse_info(void) {
    unsigned char data = fifo8_get(&g_mouseinfo);

    io_sti();

    if (mouse_decode(data) == 1) {
        erase_mouse();
        compute_mouse_position();
        draw_mouse();
    }
}

void show_keyboard_input(void) {
    static int count = 0;
    unsigned int data = fifo8_get(&g_keyinfo);

    io_sti();

    int mem_count = get_memory_block_count();
    addr_range_desc_t *mem_desc =
        (addr_range_desc_t *)get_memory_block_buffer();

    // 回车键
    if (data == 0x1C) {
        show_memory_block_info(mem_desc + count, count, COL8_FFFFFF);
        count = (count + 1);
        if (count >= mem_count) {
            count = 0;
        }
    }
}

void set_background_buf(unsigned char *buf, int xsize, int ysize) {
    boxfill8_v2(buf, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
    boxfill8_v2(buf, xsize, COL8_C6C6C6, 0, ysize - 28, xsize - 1, ysize - 28);
    boxfill8_v2(buf, xsize, COL8_FFFFFF, 0, ysize - 27, xsize - 1, ysize - 27);
    boxfill8_v2(buf, xsize, COL8_C6C6C6, 0, ysize - 26, xsize - 1, ysize - 1);

    boxfill8_v2(buf, xsize, COL8_FFFFFF, 3, ysize - 24, 59, ysize - 24);
    boxfill8_v2(buf, xsize, COL8_FFFFFF, 2, ysize - 24, 2, ysize - 4);
    boxfill8_v2(buf, xsize, COL8_848484, 3, ysize - 4, 59, ysize - 4);
    boxfill8_v2(buf, xsize, COL8_848484, 59, ysize - 23, 59, ysize - 5);
    boxfill8_v2(buf, xsize, COL8_000000, 2, ysize - 3, 59, ysize - 3);
    boxfill8_v2(buf, xsize, COL8_000000, 60, ysize - 24, 60, ysize - 3);

    boxfill8_v2(buf, xsize, COL8_848484, xsize - 47, ysize - 24, xsize - 4,
                ysize - 24);
    boxfill8_v2(buf, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47,
                ysize - 4);
    boxfill8_v2(buf, xsize, COL8_FFFFFF, xsize - 47, ysize - 3, xsize - 4,
                ysize - 3);
    boxfill8_v2(buf, xsize, COL8_FFFFFF, xsize - 3, ysize - 24, xsize - 3,
                ysize - 3);
}

win_sheet_t *sht_background(void) {
    int xsize = g_boot_info.m_screen_x, ysize = g_boot_info.m_screen_y;

    unsigned char *buf = (unsigned char *)memman_alloc_4k(xsize * ysize);
    if (!buf) {
        return NULL;
    }

    win_sheet_t *sht = win_sheet_alloc();
    if (!sht) {
        memman_free(buf, xsize * ysize);
        return NULL;
    }

    set_background_buf(buf, xsize, ysize);
    win_sheet_setbuf(sht, buf, xsize, ysize, COLOR_INVISIBLE);

    return sht;
}

void start_kernel(void) {
    init_palette();
    init_keyboard();
    init_mouse_cursor();

    memman_init();

    if (win_sheet_ctl_init()) {
        show_string(0, 0, COL8_FFFFFF, "win_sheet_ctl_init failed");
    } else {
        show_string(0, 0, COL8_FFFFFF, "win_sheet_ctl_init successful");
    }

    win_sheet_t *sht_bg = sht_background();
    win_sheet_updown(sht_bg, 0);
    win_sheet_slide(sht_bg, 0, 0);

    // win_sheet_t *sht_mouse = win_sheet_alloc();
    // win_sheet_setbuf(sht_mouse, g_mouseinfo.m_buf, 16, 16, COLOR_INVISIBLE);

    // draw_background();
    // draw_mouse();

    // show_memman_info();
    // memman_test();

    io_sti(); // 开中断
    enable_mouse();

    for (;;) {
        io_cli();
        if (fifo8_status(&g_keyinfo) + fifo8_status(&g_mouseinfo) == 0) {
            io_stihlt();
        } else if (fifo8_status(&g_keyinfo) != 0) {
            show_keyboard_input();
        } else if (fifo8_status(&g_mouseinfo) != 0) {
            show_mouse_info();
        }
    }
}
