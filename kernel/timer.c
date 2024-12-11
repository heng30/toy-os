#include "timer.h"
#include "def.h"
#include "io.h"

#define PIC0_OCW2 0x20
#define PIC1_OCW2 0xA0

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

static unsigned char timerbuf[8];
static fifo8_t timerinfo = {
    .m_buf = timerbuf,
    .m_size = sizeof(timerbuf),
    .m_free = sizeof(timerbuf),
    .m_flags = 0,
    .m_p = 0,
    .m_q = 0,
};

timerctl_t g_timerctl = {
    .m_count = 0,
    .m_timeout = 0,
    .m_fifo = &timerinfo,
    .m_data = 0,
};

void init_pit(void) {
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
}

void set_timer(unsigned int timeout, fifo8_t *fifo, unsigned char data) {
    int eflags = io_load_eflags();
    io_cli();                       // 暂时停止接收中断信号
    g_timerctl.m_timeout = timeout; // 设定时间片
    g_timerctl.m_fifo = fifo; // 设定数据队列，内核在主循环中将监控这个队列
    g_timerctl.m_data = data;
    io_store_eflags(eflags); // 恢复接收中断信号
}

void reset_timer(unsigned int timeout, unsigned char data) {
    int eflags = io_load_eflags();
    io_cli();                       // 暂时停止接收中断信号
    g_timerctl.m_timeout = timeout; // 设定时间片
    g_timerctl.m_data = data;
    io_store_eflags(eflags); // 恢复接收中断信号
}

// 每10ms会中断一次
void int_handler_for_timer(char *esp) {
    io_out8(PIC0_OCW2, 0x60); // 每次中断后都需要重新设置
    g_timerctl.m_count++;

    if (g_timerctl.m_timeout > 0) {
        g_timerctl.m_timeout--;

        if (g_timerctl.m_timeout == 0) {
            fifo8_put(g_timerctl.m_fifo, g_timerctl.m_data);
        }
    }
}
