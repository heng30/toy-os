#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "input_cursor.h"
#include "io.h"
#include "keyboard.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"
#include "multi_task.h"
#include "ring.h"
#include "timer.h"
#include "win_sheet.h"

#include "multi_task_test.h"
#include "widgets/canvas.h"
#include "widgets/console.h"
#include "widgets/input_box.h"

static void _test(void) {
#ifdef __RING_TEST__
    ring_test();
#endif

    multi_task_test_schedul();
}

void keyboard_callback(input_box_t *input_box) {
    unsigned char code = (unsigned char)fifo8_get(&g_keyinfo);

    io_sti();

    set_modkey_status(code);

    show_string_in_canvas(g_boot_info.m_screen_x - FONT_WIDTH * 10, 0,
                          COL8_FFFFFF, char2hexstr(code));

    // 回车键
    if (is_enter_down(code)) {
        // show_all_memory_block_info();
    } else if (is_backspace_down(code)) {
        input_box_pop(input_box);
    } else {
        char ch = get_pressed_char(code);
        if (ch != 0) {
            char *buf = memman_alloc(2);
            buf[0] = keydown_code2char_table[code], buf[1] = 0;
            show_string_in_canvas(g_boot_info.m_screen_x - FONT_WIDTH * 10,
                                  FONT_HEIGHT, COL8_FFFFFF, buf);
            memman_free(buf, 2);

            input_box_push(input_box, ch);
        }
    }
}

void start_kernel(void) {
    init_pit();
    init_boot_info();
    init_palette();
    init_mouse();
    init_keyboard();

    init_memman();
    init_timer_ctl();
    init_multi_task_ctl();
    init_win_sheet_ctl();

    init_background_sheet();
    init_mouse_sheet();
    init_input_cursor();
    init_canvas_sheet(CANVAS_WIN_SHEET_Z);

    init_mouse_task();
    init_timer_task();

    input_cursor_show(MOUSE_WIN_SHEET_Z - 2);

    timer_t *multi_task_display_statistics_timer = timer_alloc();
    set_timer(multi_task_display_statistics_timer, TIMER_ONE_SECOND_TIME_SLICE,
              TIMER_MAX_RUN_COUNTS, MULTI_TASK_DISPLAY_STATISTICS_DATA);

    io_sti(); // 开中断
    enable_mouse();

    show_memman_info();
    _test();

    input_box_t *input_box = input_box_new(300, 300, 168, 52, "Input");
    window_ctl_add(input_box->m_win);

    console_t *console = console_new(300, 50, 240, 200, "Console");
    window_ctl_add(console->m_win);

    unsigned int counter = 0;
    for (;;) {
        show_string_in_canvas(0, 0, COL8_FFFFFF, int2hexstr(counter++));

        io_cli();
        if (fifo8_is_empty(&g_keyinfo)) {
            io_sti(); // 开中断，保证循环不会被挂起
        } else if (!fifo8_is_empty(&g_keyinfo)) {
            keyboard_callback(input_box);
        }
    }
}
