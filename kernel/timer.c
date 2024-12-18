#include "timer.h"
#include "def.h"
#include "fifo8.h"
#include "io.h"
#include "kutil.h"
#include "memory.h"
#include "multi_task.h"

#define PIC0_OCW2 0x20
#define PIC1_OCW2 0xA0

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define UNUSED 0
#define USED 1
#define RUNNING 2

#define TIMER_FIFO_QUEUE 40960

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

#ifdef __MULTI_TASK_TEST_WITHOUT_SCHEDUL__
    multi_task_switch(g_multi_task_ctl->m_next_tr, NULL);
#else
    // 每次中断都进行任务调度，更新任务时间片和检查睡眠任务
    multi_task_schedul();
#endif
}
