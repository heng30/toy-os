#pragma once

#define AR_TSS32 0x0089    // 设置段描述符对应的TSS32对象标志位
#define AR_FUNCTION 0x409a // 设置段描述符对应的函数对象标志位
#define MULTI_TASK_TEST_B_MAIN_TIMER_DATA 123 // 测试任务b的定时器

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

// 设置段描述符
void set_segmdesc(segment_descriptor_t *sd, unsigned int limit, unsigned int base,
                  unsigned int ar);

// 测试
void multi_task_test(void);

// 切换到任务B
void multi_task_test_switch_to_task_b(void);

// 在main任务的定时回调函数中调用
void multi_task_test_in_main_timer_callback(void);
