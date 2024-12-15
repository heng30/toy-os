#pragma once

#include "timer.h"

#define AR_TSS32 0x0089    // 设置段描述符对应的TSS32对象标志位
#define AR_FUNCTION 0x409a // 设置段描述符对应的函数对象标志位

typedef struct {
    unsigned int m_backlink, m_esp0, m_ss0, m_esp1, m_ss1, m_esp2, m_ss2, m_cr3;
    unsigned int m_eip, m_eflags, m_eax, m_ecx, m_edx, m_ebx, m_esp, m_ebp, m_esi, m_edi;
    unsigned int m_es, m_cs, m_ss, m_ds, m_fs, m_gs;
    unsigned int m_ldtr, m_iomap;
} TSS32_t;

typedef struct {
    unsigned short m_limit_low, m_base_low;
    unsigned char m_base_mid, m_access_right;
    unsigned char m_limit_high, m_base_high;
} segment_descriptor_t;

extern unsigned char g_multi_task_current_tr; // 当前的gdt表偏移
extern unsigned char g_multi_task_next_tr; // 下一个要切换的gdt表偏移
extern timer_t *g_multi_task_timer; // 进程切换定时器

// 初始化,并设置当前的tr
void init_multi_task(unsigned char tr);

// 切换到tr指定的任务
void multi_task_switch(unsigned char tr);

// 设置段描述符
void set_segmdesc(segment_descriptor_t *sd, unsigned int limit,
                  unsigned int base, unsigned int ar);

// 切换到tr指定的任务
void yeild(unsigned char tr);

