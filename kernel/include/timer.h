#pragma once

#include "fifo8.h"

#define MAX_TIMER 500

typedef struct {
    unsigned char m_data; // 用户数据
    unsigned int m_flags; // 标志位

    // 设定时间片, 每次10ms中断一次，并且会减一，直到零会触发定时事件
    unsigned int m_timeout;
} timer_t;

typedef struct {
    unsigned int m_count;       // 定时器数量
    fifo8_t m_fifo;             // 暂存超时中断的发送过来的数据
    timer_t m_timer[MAX_TIMER]; // 保存定时器
} timerctl_t;

extern timerctl_t g_timerctl;

// 初始化定时器硬件
void init_pit(void);

// 初始化定时器管理器
void init_timer_ctl(void);

// 分配定时器
timer_t *timer_alloc(void);

// 设置定时时间, `timeout == 100`为1秒
void set_timer(timer_t *timer, unsigned int timeout, unsigned char data);

// 释放定时器
void timer_free(timer_t *timer);

// 定时中断处理函数
void int_handler_for_timer(char *esp);
