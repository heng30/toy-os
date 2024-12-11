#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "io.h"
#include "keyboard.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"
#include "timer.h"
#include "win_sheet.h"

#include "message_box.h"

void show_mouse(void) {
    unsigned char data = fifo8_get(&g_mouseinfo);

    io_sti();

    if (mouse_decode(data) == 1) {
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

void start_kernel(void) {
    init_pit();
    init_palette();
    init_cursor();
    init_keyboard();

    init_memman();
    init_win_sheet_ctl();

    init_background_sheet();
    init_mouse_sheet();
    init_canvas_sheet(BOTTOM_WIN_SHEET_Z + 1);

    message_box_t *msg_box =
        message_box_new(80, 72, 168, 68, BOTTOM_WIN_SHEET_Z + 2, "Toy-OS");

    assert(msg_box != NULL, "msg_box is null");

    set_timer(100, g_timerctl.m_fifo, 1);

    io_sti(); // 开中断
    enable_mouse();

    for (;;) {
        char *p = int2hexstr(g_timerctl.m_timeout);
        show_string(msg_box->m_sheet, 40, 28, COL8_FFFFFF, COL8_000000, p);

        io_cli();
        if (fifo8_status(&g_keyinfo) + fifo8_status(&g_mouseinfo) +
                fifo8_status(g_timerctl.m_fifo) ==
            0) {
            io_sti(); // 保证循环不会被挂起
            // io_stihlt();
        } else if (fifo8_status(&g_keyinfo) != 0) {
            show_keyboard_input();

            if (message_box_is_visible(msg_box)) {
                message_box_hide(msg_box);
            } else {
                message_box_show(msg_box, BOTTOM_WIN_SHEET_Z + 2);
            }
        } else if (fifo8_status(&g_mouseinfo) != 0) {
            show_mouse();
        } else if (fifo8_status(g_timerctl.m_fifo) != 0) {
            static int counter = 1;
            fifo8_get(g_timerctl.m_fifo);
            io_sti();

            show_string_in_canvas(0, 0, COL8_FFFFFF, int2hexstr(counter), true);

            reset_timer(100, 1);
            counter += 1;
        }
    }
}
