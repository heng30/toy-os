#include "multi_task_test.h"
#include "colo8.h"
#include "draw.h"
#include "io.h"
#include "multi_task.h"

#include "widgets/canvas.h"

static task_t *g_task_print_A = NULL;
static task_t *g_task_print_B = NULL;

void multi_task_test_schedul_print_A(task_t *task) {
    unsigned int pos = 0;

    for (;;) {
        show_string_in_canvas(pos, 250, COL8_FFFFFF, "A");
        pos += FONT_WIDTH;

        if (pos == FONT_WIDTH * 5) {
            multi_task_suspend(g_task_print_A);
        } else {
            multi_task_sleep(g_task_print_A, TIMER_ONE_SECOND_TIME_SLICE * 1);
        }

        if (pos >= g_boot_info.m_screen_x) {
            clear_canvas_space(0, 250, g_canvas_sht->m_bxsize, FONT_HEIGHT);
            pos = 0;
        }
    }
}

void multi_task_test_schedul_print_B(task_t *task) {
    unsigned int pos = 0;

    for (;;) {
        show_string_in_canvas(pos, 250 + FONT_HEIGHT, COL8_FFFFFF, "B");
        pos += FONT_WIDTH;

        multi_task_sleep(g_task_print_B, TIMER_ONE_SECOND_TIME_SLICE);

        if (pos == FONT_WIDTH * 10) {
            multi_task_resume(g_task_print_A);
        }

        if (pos >= g_boot_info.m_screen_x) {
            clear_canvas_space(0, 250 + FONT_HEIGHT, g_canvas_sht->m_bxsize,
                               FONT_HEIGHT);
            pos = 0;
        }
    }
}

void multi_task_test_schedul(void) {
    g_task_print_A = multi_task_alloc((ptr_t)multi_task_test_schedul_print_A, 0,
                                      NULL, TIMER_ONE_SECOND_TIME_SLICE);

    g_task_print_B = multi_task_alloc((ptr_t)multi_task_test_schedul_print_B, 0,
                                      NULL, TIMER_ONE_SECOND_TIME_SLICE);

    multi_task_run(g_task_print_A);
    multi_task_run(g_task_print_B);
}
