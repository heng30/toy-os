#pragma once

#include "def.h"
#include "fifo8.h"

#define MAX_TIMER 256 // 最大指出的定时器数量

// 最大定时器连续触发此时，可以看作是不断触发的定时器
#define TIMER_MAX_RUN_COUNTS 0xffffffff

#define TIMER_ONE_SECOND_TIME_SLICE 100  // 1秒的时间片数量
#define TIMER_INPUT_CURSOR_TIME_SLICE 50 // 500ms的时间片数量

// 数据范围0-255，一个共同时指出256个不同的定时器并发
#define INPUT_CURSOR_TIMER_DATA 255 // 通过这个数据开区分不同的定时器
#define MULTI_TASK_TEST_B_MAIN_TIMER_DATA 254 // 测试任务b的定时器
#define MULTI_TASK_TEST_B_MAIN_TIMER_AUTO_DATA 253 // 测试自动切换任务b的定时器
#define MULTI_TASK_DISPLAY_STATISTICS_DATA 252 // 定时显示任务统计信息
#define INFINITE_TIMER_COUNTER_DATA 251

typedef struct {
    unsigned char m_data; // 用户数据
    unsigned int m_flags; // 标志位

    // 设定时间片, 每次10ms中断一次，并且会减一，直到零会触发定时事件
    unsigned int m_timeout;

    // 保存timeout的副本，如果run_count > 1，需要使用这个值来重置timeout
    unsigned int m_const_timeout;

    unsigned int m_run_count; // 这个定时器触发多少次
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
void set_timer(timer_t *timer, unsigned int timeout, unsigned int run_count,
               unsigned char data);

// 释放定时器
void timer_free(timer_t *timer);

// 定时中断处理函数
void int_handler_for_timer(char *esp);

// 初始化定时任务
void init_timer_task(void);
