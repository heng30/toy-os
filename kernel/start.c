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
#include "timer.h"
#include "win_sheet.h"

#include "multi_task_test.h"
#include "widgets/input_box.h"

void mouse_callback(void) {
    unsigned char code = (unsigned char)fifo8_get(&g_mouseinfo);

    io_sti();

    if (mouse_decode(code) == 1) {
        draw_mouse();

        if (is_mouse_left_btn_pressed()) {
            // TODO: 捕获鼠标左键按下
            moving_sheet();
        }
    }
}

void keyboard_callback(input_box_t *input_box) {
    unsigned char code = (unsigned char)fifo8_get(&g_keyinfo);

    io_sti();

    set_modkey_status(code);

    show_string_in_canvas(g_boot_info.m_screen_x - FONT_WIDTH * 10, 0,
                          COL8_FFFFFF, char2hexstr(code));

    // 回车键
    if (is_enter_down(code)) {
        show_all_memory_block_info();
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

void timer_callback() {
    static unsigned int timer_callback_timer_counter = 0;
    unsigned char data = (unsigned char)fifo8_get(&g_timerctl.m_fifo);
    io_sti();

    switch (data) {
    case INPUT_CURSOR_TIMER_DATA:
        input_cursor_blink();
        break;

#ifdef __MULTI_TASK_TEST_WITHOUT_SCHEDUL__
    case MULTI_TASK_TEST_B_MAIN_TIMER_DATA:
        multi_task_test_in_main_timer_callback();
        break;

    case MULTI_TASK_TEST_B_MAIN_TIMER_AUTO_DATA:
        multi_task_test_in_main_timer_callback_auto();
        break;
#endif

    case MULTI_TASK_DISPLAY_STATISTICS_DATA:
        multi_task_statistics_display();
        break;

    case 111:
        show_string_in_canvas(FONT_WIDTH * 16, 0, COL8_FFFFFF, "infinite timer counter:");
        show_string_in_canvas(FONT_WIDTH * 40, 0, COL8_FFFFFF,
                              int2hexstr(timer_callback_timer_counter++));
        break;

    default:
        break;
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
    init_canvas_sheet(CANVAS_WIN_SHEET_Z);

    init_input_cursor();
    input_cursor_show(MOUSE_WIN_SHEET_Z - 2);

    input_box_t *input_box = input_box_new(300, 150, 168, 68, "Input-Box");
    input_box_show(input_box, BOTTOM_WIN_SHEET_Z + 3);
    win_sheet_set_moving(input_box->m_sheet);

    input_box_draw_text(input_box, "hello");

    timer_t *timer1 = timer_alloc();
    set_timer(timer1, TIMER_ONE_SECOND_TIME_SLICE, TIMER_MAX_RUN_COUNTS, 111);

    timer_t *multi_task_display_statistics_timer = timer_alloc();
    set_timer(multi_task_display_statistics_timer, TIMER_ONE_SECOND_TIME_SLICE,
              TIMER_MAX_RUN_COUNTS, MULTI_TASK_DISPLAY_STATISTICS_DATA);

    io_sti(); // 开中断
    enable_mouse();

#ifdef __MULTI_TASK_TEST_WITHOUT_SCHEDUL__
    // multi_task_test();
    multi_task_test_auto();
#endif

    unsigned int counter = 0;
    for (;;) {
        show_string_in_canvas(0, 0, COL8_FFFFFF, int2hexstr(counter++));

        io_cli();
        if (fifo8_is_empty(&g_keyinfo) && fifo8_is_empty(&g_mouseinfo) &&
            fifo8_is_empty(&g_timerctl.m_fifo)) {
            io_sti(); // 开中断，保证循环不会被挂起
        } else if (!fifo8_is_empty(&g_keyinfo)) {
            keyboard_callback(input_box);

            // if (win_sheet_is_visible(WIN_SHEET_OBJ(input_box))) {
            //     input_box_hide(input_box);
            // } else {
            //     input_box_show(input_box, BOTTOM_WIN_SHEET_Z + 2);
            // }
        } else if (!fifo8_is_empty(&g_mouseinfo)) {
            mouse_callback();
        } else if (!fifo8_is_empty(&g_timerctl.m_fifo)) {
            timer_callback();
        }
    }
}
