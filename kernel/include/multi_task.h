#pragma once

#include "timer.h"

#define AR_TSS32 0x0089    // 设置段描述符对应的TSS32对象标志位
#define AR_FUNCTION 0x409a // 设置段描述符对应的函数对象标志位

// 最大支持的任务数, 数量小于256
// 需要根据kernel.asm的TSS32描述符和堆栈数量确定
#define MAX_TASKS 2
#define TASK_GDT0 7 // 开始的tr值, 也是主任务的tr

// 任务堆栈大小，需要和kernel.asm中分配的堆栈大小一致
#define TASK_STACK_SIZE 1024

#define DEFAULT_RUNNING_TIME_SLICE TIMER_ONE_SECOND_TIME_SLICE // 1秒

#define TASK_STATUS_UNUSED 0  // 没有分配
#define TASK_STATUS_USED 1    // 已经分配，当没有运行过
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
    unsigned int
        m_running_time_slice; // 该任务设置的运行时间片，时间片用完就会进行任务切换
    unsigned int
        m_remain_time_slice; // 任务剩余的时间片，时间片归零进行任务切换
    unsigned int
        m_sleep_time_slice; // 任务剩余的睡眠时间片，时间片归零恢复任务
    TSS32_t m_tss;
} task_t;

// 任务管理器
typedef struct {
    int m_running_task_counts;  // 正在运行的任务数量
    unsigned char m_current_tr; // 当前任务的tr
    unsigned char m_next_tr;    // 下一个要运行的任务tr
    task_t *m_tasks[MAX_TASKS]; // 任务列表，长度为running_task_counts
    task_t m_tasks0[MAX_TASKS]; // 预分配内存
} multi_task_ctl_t;

extern multi_task_ctl_t *g_multi_task_ctl;

// 初始化
void init_multi_task_ctl();

// 分配一个任务
task_t *multi_task_alloc(unsigned int running_time_slice);

// 恢复一个任务, 这个任务会添加到任务队列, 等待任务调度
void multi_task_resume(task_t *task);

// 切换到tr指定的任务
void multi_task_switch(unsigned char tr);

// 设置段描述符
void set_segmdesc(segment_descriptor_t *sd, unsigned int limit,
                  unsigned int base, unsigned int ar);

// 切换到tr指定的任务
void multi_task_yeild(unsigned char tr);
