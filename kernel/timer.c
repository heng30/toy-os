#include "timer.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "fifo8.h"
#include "input_cursor.h"
#include "io.h"
#include "kutil.h"
#include "memory.h"
#include "multi_task.h"

#include "widgets/canvas.h"

#define PIC0_OCW2 0x20
#define PIC1_OCW2 0xA0

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define UNUSED 0
#define USED 1
#define RUNNING 2

#define TIMER_FIFO_QUEUE 40960

task_t *g_timer_task = NULL;

timerctl_t g_timerctl = {
    .m_count = 0,
    .m_fifo =
        {
            .m_buf = NULL,
            .m_size = TIMER_FIFO_QUEUE,
            .m_free = TIMER_FIFO_QUEUE,
            .m_flags = UNUSED,
            .m_p = 0,
            .m_q = 0,
        },
};

void init_pit(void) {
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
}

void init_timer_ctl(void) {
    unsigned char *buf = (unsigned char *)memman_alloc_4k(TIMER_FIFO_QUEUE);
    assert(buf != NULL, "init_timer_ctl alloc 4k buffer error");

    if (!g_timerctl.m_fifo.m_buf)
        g_timerctl.m_fifo.m_buf = buf;

    for (int i = 0; i < MAX_TIMER; i++) {
        g_timerctl.m_timer[i].m_flags = UNUSED;
    }
}

timer_t *timer_alloc(void) {
    for (int i = 0; i < MAX_TIMER; i++) {
        if (g_timerctl.m_timer[i].m_flags == UNUSED) {
            g_timerctl.m_timer[i].m_flags = USED;
            g_timerctl.m_timer[i].m_timeout = 0;
            g_timerctl.m_timer[i].m_const_timeout = 0;
            g_timerctl.m_timer[i].m_run_count = 0;
            g_timerctl.m_timer[i].m_data = 0;
            return &g_timerctl.m_timer[i];
        }
    }

    assert(false, "timer_alloc failed");
    return NULL;
}

void timer_free(timer_t *timer) { timer->m_flags = UNUSED; }

void set_timer(timer_t *timer, unsigned int timeout, unsigned int run_count,
               unsigned char data) {
    int eflags = io_load_eflags();
    io_cli(); // 暂时停止接收中断信号
    timer->m_flags = RUNNING;
    timer->m_timeout = timeout; // 设定时间片
    timer->m_const_timeout = timeout;
    timer->m_run_count = run_count;
    timer->m_data = data;
    io_store_eflags(eflags); // 恢复接收中断信号
}

// 每10ms会中断一次
void int_handler_for_timer(char *esp) {
    io_out8(PIC0_OCW2, 0x60); // 每次中断后都需要重新设置
    g_timerctl.m_count++;

    // 遍历所有定时器
    for (int i = 0; i < MAX_TIMER; i++) {
        if (g_timerctl.m_timer[i].m_flags == RUNNING) {
            g_timerctl.m_timer[i].m_timeout--;

            // 时间片用完
            if (g_timerctl.m_timer[i].m_timeout == 0) {
                // 重复次数也归零，则停止定时器
                if (g_timerctl.m_timer[i].m_run_count == 0) {
                    g_timerctl.m_timer[i].m_flags = USED;
                } else {
                    g_timerctl.m_timer[i].m_run_count--;

                    // 重新启动定时器
                    g_timerctl.m_timer[i].m_timeout =
                        g_timerctl.m_timer[i].m_const_timeout;
                }

                fifo8_put(&g_timerctl.m_fifo, g_timerctl.m_timer[i].m_data);
            }
        }
    }

    // 添加定时任务到优先任务队列
    if (!fifo8_is_empty(&g_timerctl.m_fifo)) {
        multi_task_priority_task_add(g_timer_task);
    }

    // 每次中断都进行任务调度，更新任务时间片和检查睡眠任务
    multi_task_schedul();
}

static void _timer_callback(void) {
    static unsigned int timer_callback_timer_counter = 0;

    io_cli();
    unsigned char data = (unsigned char)fifo8_get(&g_timerctl.m_fifo);
    io_sti();

    switch (data) {
    case INPUT_CURSOR_TIMER_DATA:
        input_cursor_blink();
        break;

    case MULTI_TASK_DISPLAY_STATISTICS_DATA:
        multi_task_statistics_display();
        break;

    case INFINITE_TIMER_COUNTER_DATA:
        show_string_in_canvas(g_boot_info.m_screen_x - FONT_WIDTH * 11 - 50,
                              g_boot_info.m_screen_y - FONT_HEIGHT - 5,
                              COLOR_BLACK,
                              int2hexstr(timer_callback_timer_counter++));
        break;

    default:
        break;
    }
}

static void _timer_task_main(task_t *task) {
    for (;;) {
        io_sti(); // 开中断，保证循环不会被挂起
        if (fifo8_is_empty(&g_timerctl.m_fifo))
            continue;

        _timer_callback();
    }
}

void init_timer_task(void) {
    timer_t *infinite_timer = timer_alloc();
    set_timer(infinite_timer, TIMER_ONE_SECOND_TIME_SLICE, TIMER_MAX_RUN_COUNTS,
              INFINITE_TIMER_COUNTER_DATA);

    g_timer_task = multi_task_alloc((ptr_t)_timer_task_main, 0, NULL,
                                    ONE_RUNNING_TIME_SLICE);
}
