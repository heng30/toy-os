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

#include "widgets/input_box.h"

void mouse_callback(void) {
    unsigned char data = fifo8_get(&g_mouseinfo);

    io_sti();

    if (mouse_decode(data) == 1) {
        draw_mouse();
    }
}

void keyboard_callback(input_box_t *input_box) {
    unsigned char data = fifo8_get(&g_keyinfo);

    io_sti();

    show_string_in_canvas(8, FONT_HEIGHT * 8, COL8_FFFFFF, char2hexstr(data));

    // 回车键
    if (is_enter_down(data)) {
        show_all_memory_block_info();
    } else if (is_backspace_down(data)) {
        input_box_pop(input_box);
    } else {
        char ch = get_pressed_char(data);
        if (ch != 0) {
            char *buf = memman_alloc(2);
            buf[0] = keydown_table[data], buf[1] = 0;
            show_string_in_canvas(8, FONT_HEIGHT * 9, COL8_FFFFFF, buf);
            memman_free(buf, 2);

            input_box_push(input_box, ch);
        }
    }
}

void timer_callback() {
    unsigned char data = fifo8_get(&g_timerctl.m_fifo);
    io_sti();

    switch (data) {
    case 1:
        show_string_in_canvas(8, 0, COL8_FFFFFF, "3 Seconds");
        break;
    case 2:
        show_string_in_canvas(8, FONT_HEIGHT, COL8_FFFFFF, "5 Seconds");
        break;
    case INPUT_BLOCK_TIMER_DATA:
        input_block_blink();
    default:
        break;
    }
}

void start_kernel(void) {
    init_pit();
    init_boot_info();
    init_palette();
    init_cursor();
    init_keyboard();

    init_memman();
    init_timer_ctl();
    init_win_sheet_ctl();

    init_background_sheet();
    init_mouse_sheet();
    init_canvas_sheet(CANVAS_WIN_SHEET_Z);

    init_input_block_sheet();
    init_input_block_timer();
    input_block_show(MOUSE_WIN_SHEET_Z - 2);

    input_box_t *input_box = input_box_new(80, 150, 168, 68, "Input-Box");
    win_sheet_show(WIN_SHEET_OBJ(input_box), BOTTOM_WIN_SHEET_Z + 3);
    set_focus_sheet(input_box->m_sheet);
    // input_box_focus(input_box);
    input_box_draw_text(input_box, "hello");

    timer_t *timer1 = timer_alloc(), *timer2 = timer_alloc();

    set_timer(timer1, 300, 1), set_timer(timer2, 500, 2);

    io_sti(); // 开中断
    enable_mouse();

    for (;;) {
        io_cli();
        if (fifo8_status(&g_keyinfo) + fifo8_status(&g_mouseinfo) +
                fifo8_status(&g_timerctl.m_fifo) ==
            0) {
            io_sti(); // 开中断，保证循环不会被挂起
        } else if (fifo8_status(&g_keyinfo) != 0) {
            keyboard_callback(input_box);

            // win_sheet_t *sht = WIN_SHEET_OBJ(input_box);
            // if (win_sheet_is_visible(sht)) {
            //     win_sheet_hide(sht);
            // } else {
            //     win_sheet_show(sht, BOTTOM_WIN_SHEET_Z + 2);
            // }
        } else if (fifo8_status(&g_mouseinfo) != 0) {
            mouse_callback();
        } else if (fifo8_status(&g_timerctl.m_fifo) != 0) {
            timer_callback();
        }
    }
}
