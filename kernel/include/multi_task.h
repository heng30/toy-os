#pragma once

#include "ring.h"
#include "timer.h"

#define LIMIT_TSS32 103
#define AR_TSS32 0x0089    // 设置段描述符对应的TSS32对象标志位
#define AR_FUNCTION 0x409a // 设置段描述符对应的函数对象标志位
#define AR_FUNCTION_DS 0x4092 // 设置段描述符对应的函数对象段描述符标志位

// 最大支持的任务数, 数量小于256
// 需要根据kernel.asm的TSS32描述符和堆栈数量确定
#define MAX_TASKS 10
#define TASK_GDT0 10 // 开始的tr值, 也是主任务的tr

// 任务堆栈大小，需要和kernel.asm中分配的堆栈大小一致
#define TASK_STACK_SIZE 1024

#define DEFAULT_RUNNING_TIME_SLICE 1 // TIMER_ONE_SECOND_TIME_SLICE // 1秒
#define ONE_RUNNING_TIME_SLICE 1

#define TASK_STATUS_UNUSED 0  // 没有分配
#define TASK_STATUS_USED 1    // 已经分配，但没有运行过
#define TASK_STATUS_RUNNING 2 // 正在运行
#define TASK_STATUS_SLEEP 3   // 正在睡眠
#define TASK_STATUS_SUSPEND 4 // 挂起

// 段描述符
typedef struct {
    unsigned short m_limit_low, m_base_low;
    unsigned char m_base_mid, m_access_right;
    unsigned char m_limit_high, m_base_high;
} segment_descriptor_t;

// 任务却换需要保存的寄存器
typedef struct {
    unsigned int m_backlink, m_esp0, m_ss0, m_esp1, m_ss1, m_esp2, m_ss2, m_cr3;
    unsigned int m_eip, m_eflags, m_eax, m_ecx, m_edx, m_ebx, m_esp, m_ebp,
        m_esi, m_edi;
    unsigned int m_es, m_cs, m_ss, m_ds, m_fs, m_gs;
    unsigned int m_ldtr, m_iomap;
} TSS32_t;

// 任务
typedef struct {
    unsigned char m_tr;    // gdt对应的TSS32位置下标, 用于任务切换
    unsigned char m_flags; // 任务状态

    // 该任务设置的运行时间片.
    // 当remain_time_slice归零，并且重新运行后，会使用这个值重新设置remain_time_slice
    unsigned int m_running_time_slice;

    // 任务剩余的时间片，时间片归零进行任务切换
    unsigned int m_remain_time_slice;

    // 任务剩余的睡眠时间片，时间片归零恢复任务
    unsigned int m_sleep_time_slice;

    // 是否在优先任务队列中
    bool m_is_priority_task;

    // 任务引用次数
    unsigned int m_ref;

    TSS32_t m_tss;
} task_t;

typedef struct {
    unsigned int m_total_task_counts;   // 所有可分配任务数
    unsigned int m_used_task_counts;    // 已经使用的任务数
    unsigned int m_running_task_counts; // 正在运行的任务数量
    unsigned int m_sleep_task_counts;   // 睡眠任务数量
    unsigned int m_suspend_task_counts; // 挂起任务数量
} multi_task_statistics_t;

// 任务管理器
typedef struct {
    // 运行的任务数量, 包括TASK_STATUS_RUNNING,
    // TASK_STATUS_SLEEP, TASK_STATUS_SUSPEND
    unsigned int m_tasks_counts;
    task_t *m_current_task; // 当前任务

    // 优先任务
    // 优先任务只会运行1个时间片就会被切换,
    // 所以普通任务不应该放到优先队列中。
    // 只有类似于鼠标移动的任务才能放到其中。
    // 因为鼠标是通过中断函数将鼠标任务放到其中的，
    // 而且鼠标任务处理的时间也很短，1个时间片可以处理完。
    ring_t *m_priority_tasks;

    multi_task_statistics_t m_statistics; // 任务统计

    // 任务列表，长度为running_task_counts
    // 普通任务队列，在这个队列中的任务会运行完对应的时间片
    // 才会运行下一个任务，所以应该任务考虑任务时间片
    task_t *m_tasks[MAX_TASKS];
    task_t m_tasks0[MAX_TASKS]; // 预分配任务内存
} multi_task_ctl_t;

extern multi_task_ctl_t *g_multi_task_ctl;

// 设置段描述符
void set_segmdesc(segment_descriptor_t *sd, unsigned int limit,
                  unsigned int base, unsigned int ar);

// 初始化
void init_multi_task_ctl();

// 分配一个任务
// task_main: 任务函数
// argc: task_main函数参数个数，不包括第一个task_t*类型的参数
// argv: task_main函数参数列表，不包括第一个task_t*类型的参数
// running_time_slice: 一次连续运行的时间片
task_t *multi_task_alloc(ptr_t task_main, unsigned int argc, void *argv[],
                         unsigned int running_time_slice);

// 释放一个任务
void multi_task_free(task_t *task);

// 启动一个任务，这个任务必须是刚创建的
void multi_task_run(task_t *task);

// 恢复一个任务, 这个任务必须是suspend的
void multi_task_resume(task_t *task);

// 挂起一个任务, 使用resume进行恢复. 只有TASK_STATUS_RUNNING任务才能挂起
void multi_task_suspend(task_t *task);

// 任务睡眠, 只有TASK_STATUS_RUNNING任务才能睡眠
void multi_task_sleep(task_t *task, unsigned int sleep_time_slice);

// 任务调度
void multi_task_schedul(void);

// 切换到tr指定的任务, 该函数只能在中断函数中调用
void multi_task_switch(task_t *task);

// 将任务添加到优先任务队列中
bool multi_task_priority_task_add(task_t *task);

// 增加1次引用
void multi_task_ref_inc(task_t *task);

// 减少1次引用
void multi_task_ref_dec(task_t *task);

// 显示任务统计信息
void multi_task_statistics_display(void);
