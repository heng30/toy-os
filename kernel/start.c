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

void mouse_callback(void) {
    unsigned char data = fifo8_get(&g_mouseinfo);

    io_sti();

    if (mouse_decode(data) == 1) {
        draw_mouse();
    }
}

void keyboard_callback(void) {
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

void timer_callback(timer_t *timer) {
    unsigned char data = fifo8_get(&g_timerctl.m_fifo);
    io_sti();

    switch (data) {
    case 1:
        show_string_in_canvas(8, 0, COL8_FFFFFF, "3 Seconds");
        break;
    case 2:
        show_string_in_canvas(8, FONT_HEIGHT, COL8_FFFFFF, "5 Seconds");
        break;
    default:
        if (data == 3) {
            show_string_in_canvas(8, FONT_HEIGHT * 2, COL8_FFFFFF, "0");
            set_timer(timer, 50, 4);
        } else {
            show_string_in_canvas(8, FONT_HEIGHT * 2, COL8_FFFFFF, "1");
            set_timer(timer, 50, 3);
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
    // init_timer_ctl();

    init_background_sheet();
    init_mouse_sheet();
    init_canvas_sheet(CANVAS_WIN_SHEET_Z);

    message_box_t *msg_box =
        message_box_new(80, 72, 168, 68, BOTTOM_WIN_SHEET_Z + 2, "Toy-OS");

    assert(msg_box != NULL, "msg_box is null");

    timer_t *timer1 = timer_alloc(), *timer2 = timer_alloc(),
            *timer3 = timer_alloc();

    set_timer(timer1, 300, 1), set_timer(timer2, 500, 2),
        set_timer(timer3, 50, 3);

    io_sti(); // 开中断
    enable_mouse();

    for (;;) {
        io_cli();
        if (fifo8_status(&g_keyinfo) + fifo8_status(&g_mouseinfo) +
                fifo8_status(&g_timerctl.m_fifo) ==
            0) {
            io_sti(); // 开中断，保证循环不会被挂起
        } else if (fifo8_status(&g_keyinfo) != 0) {
            keyboard_callback();

            if (message_box_is_visible(msg_box)) {
                message_box_hide(msg_box);
            } else {
                message_box_show(msg_box, BOTTOM_WIN_SHEET_Z + 2);
            }
        } else if (fifo8_status(&g_mouseinfo) != 0) {
            mouse_callback();
        } else if (fifo8_status(&g_timerctl.m_fifo) != 0) {
            timer_callback(timer3);
        }
    }
}
