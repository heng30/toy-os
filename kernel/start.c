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

    // if (mouse_decode(data) == 1) {
    //     erase_mouse();
    //     compute_mouse_position();
    //     draw_mouse();
    // }
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

void start_kernel(void) {
    init_palette();
    init_keyboard();
    init_mouse_cursor();

    memman_init();

    if (win_sheet_ctl_init()) {
        show_string(g_boot_info.m_vga_ram, g_boot_info.m_screen_x, 0, 0,
                    COL8_FFFFFF, "win_sheet_ctl_init failed");
        dead_loop();
    }

    sht_background();

    // draw_background();

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
