#pragma once

#include "timer.h"

#define AR_TSS32 0x0089    // 设置段描述符对应的TSS32对象标志位
#define AR_FUNCTION 0x409a // 设置段描述符对应的函数对象标志位

typedef struct {
    unsigned int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    unsigned int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    unsigned int es, cs, ss, ds, fs, gs;
    unsigned int ldtr, iomap;
} TSS32_t;

typedef struct {
    unsigned short limit_low, base_low;
    unsigned char base_mid, access_right;
    unsigned char limit_high, base_high;
} segment_descriptor_t;

extern unsigned char g_multi_task_current_tr; // 当前的gdt表偏移
extern unsigned char g_multi_task_next_tr; // 下一个要切换的gdt表偏移
extern timer_t *g_multi_task_timer; // 进程切换定时器

// 初始化,并设置当前的tr
void init_multi_task(unsigned char tr);

// 进程切换到tr指定的任务
void multi_task_switch(unsigned char tr);

// 设置段描述符
void set_segmdesc(segment_descriptor_t *sd, unsigned int limit,
                  unsigned int base, unsigned int ar);

/*************************************单次任务切换测试****************************/
// 在main任务的定时回调函数中调用
void multi_task_test_in_main_timer_callback(void);

// 测试
void multi_task_test(void);

/*************************************交替打印AB测试****************************/
// 在main任务的定时回调函数中调用
void multi_task_test_in_main_timer_callback_auto(void);

// 测试
void multi_task_test_auto(void);
