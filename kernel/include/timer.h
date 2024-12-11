#pragma once

#include "fifo8.h"

typedef struct {
    unsigned int m_count; // 定时中断次数
    unsigned int m_timeout; // 设定时间片, 每次10ms中断一次，并且会减一，直到零会触发定时事件
    fifo8_t *m_fifo; // 暂存超时中断的发送过来的数据
    unsigned char m_data;
} timerctl_t;

extern timerctl_t g_timerctl;

// 初始化定时器硬件
void init_pit(void);

// 设置定时时间, `timeout == 100`为1秒
void set_timer(unsigned int timeout, fifo8_t *fifo, unsigned char data);

// 重新设置定时时间
void reset_timer(unsigned int timeout, unsigned char data);

// 定时中断处理函数
void int_handler_for_timer(char *esp);
