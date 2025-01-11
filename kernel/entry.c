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
#include "string.h"
#include "system_call.h"
#include "timer.h"
#include "win_sheet.h"

#include "widgets/canvas.h"
#include "widgets/console.h"
#include "widgets/input_box.h"

static void _test(void) {
#ifdef __RING_TEST__
    ring_test();
#endif
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

    io_sti(); // 开中断
    enable_mouse();

    _test();

    task_t *input_box_task = init_input_box_task();
    task_t *console_task = init_console_task();

    unsigned int counter = 0;
    for (;;) {
        io_sti();
        show_string_in_canvas(0, 0, COL8_FFFFFF, int2hexstr(counter++));

        // 更新随机数
        g_rand_number = g_rand_number ^ (counter * 10000799);
    }
}
