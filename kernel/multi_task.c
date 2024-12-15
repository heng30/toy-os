#include "multi_task.h"
#include "draw.h"
#include "io.h"
#include "kutil.h"
#include "memory.h"

static volatile unsigned int g_switch_task_counts = 0; // 任务切换次数

multi_task_ctl_t *g_multi_task_ctl = NULL;

void init_multi_task_ctl(void) {
    g_multi_task_ctl =
        (multi_task_ctl_t *)memman_alloc_4k(sizeof(multi_task_ctl_t));

    assert(g_multi_task_ctl != NULL, "init_multi_task_ctl alloc 4k error");

    segment_descriptor_t *gdt = (segment_descriptor_t *)get_addr_gdt();
    for (unsigned char i = 0; i < MAX_TASKS; i++) {
        g_multi_task_ctl->m_tasks0[i].m_flags = TASK_STATUS_UNUSED;
        g_multi_task_ctl->m_tasks0[i].m_tr = TASK_GDT0 + i;

        // 设置全局描述附表中的段描述符为TSS32对象
        set_segmdesc(gdt + TASK_GDT0 + i, 103,
                     (ptr_t)&g_multi_task_ctl->m_tasks0[i].m_tss, AR_TSS32);
    }

    task_t *task = multi_task_alloc(DEFAULT_RUNNING_TIME_SLICE);
    assert(task->m_tr == TASK_GDT0, "init_multi_task_ctl invalid main task tr");

    task->m_flags = TASK_STATUS_RUNNING;

    g_multi_task_ctl->m_running_task_counts = 1;
    g_multi_task_ctl->m_current_tr = TASK_GDT0;
    g_multi_task_ctl->m_next_tr = TASK_GDT0;
    g_multi_task_ctl->m_tasks[0] = task;

    load_tr(task->m_tr << 3);
}

task_t *multi_task_alloc(unsigned int running_time_slice) {
    for (unsigned char i = 0; i < MAX_TASKS; i++) {
        if (g_multi_task_ctl->m_tasks0[i].m_flags == TASK_STATUS_UNUSED) {
            task_t *task = &g_multi_task_ctl->m_tasks0[i];
            task->m_flags = TASK_STATUS_USED;
            task->m_running_time_slice = running_time_slice;
            task->m_remain_time_slice = running_time_slice;
            task->m_sleep_time_slice = 0;

            task->m_tss.m_eflags = 0x00000202;
            task->m_tss.m_eax = 0;
            task->m_tss.m_ecx = 0;
            task->m_tss.m_edx = 0;
            task->m_tss.m_ebx = 0;
            task->m_tss.m_ebp = 0;

            // 任务堆栈指针位置，需要和kernel.asm中的堆栈对应
            task->m_tss.m_esp = (unsigned int)TASK_STACK_SIZE * (i + 1);

            task->m_tss.m_esi = 0;
            task->m_tss.m_edi = 0;
            task->m_tss.m_es = 0;
            task->m_tss.m_ds = 0;
            task->m_tss.m_fs = 0;
            task->m_tss.m_gs = 0;
            task->m_tss.m_ldtr = 0;
            task->m_tss.m_iomap = 0x40000000;
            return task;
        }
    }

    assert(false, "multi_task_alloc failed");
    return NULL;
}

void multi_task_resume(task_t *task) {
    task->m_flags = TASK_STATUS_RUNNING;
    g_multi_task_ctl->m_tasks[g_multi_task_ctl->m_running_task_counts] = task;
    g_multi_task_ctl->m_running_task_counts++;
}

void multi_task_suspend(task_t *task) {

}

void multi_task_sleep(task_t *task, unsigned int sleep_time_slice) {

}

// 获取下一个可运行的任务
static unsigned char _get_next_availible_task(unsigned char current_index) {
    for (unsigned int i = current_index + 1;
         i < (unsigned int)g_multi_task_ctl->m_running_task_counts; i++) {
        if (g_multi_task_ctl->m_tasks[i]->m_flags == TASK_STATUS_RUNNING) {
            return (unsigned char)i;
        }
    }

    for (unsigned char i = 0; i < current_index; i++) {
        if (g_multi_task_ctl->m_tasks[i]->m_flags == TASK_STATUS_RUNNING) {
            return i;
        }
    }

    return current_index;
}

void multi_task_schedul(void) {
    for (unsigned char i = 0; i < g_multi_task_ctl->m_running_task_counts;
         i++) {
        task_t *task = g_multi_task_ctl->m_tasks[i];

        // 找到当前正在运行的任务
        if (task->m_tr == g_multi_task_ctl->m_current_tr) {
            // 时间片用完，进行任务切换
            if (task->m_remain_time_slice == 0) {
                unsigned char next_index = _get_next_availible_task(i);

                // 没有其他可以运行的任务，继续当前的任务
                if (next_index == i) {
                    task->m_remain_time_slice = task->m_running_time_slice;
                } else { // 进行任务切换
                    task_t *next_task = g_multi_task_ctl->m_tasks[next_index];

                    // 如果时间片用完，重新分配一个新的时间片
                    if (next_task->m_remain_time_slice == 0) {
                        next_task->m_remain_time_slice =
                            next_task->m_running_time_slice;
                    }

                    multi_task_switch(next_task->m_tr);
                }
            } else {
                task->m_remain_time_slice--;
            }
        }

        if (task->m_flags == TASK_STATUS_SLEEP) {
            if (task->m_sleep_time_slice == 0) {
                task->m_flags = TASK_STATUS_RUNNING; // 等待下一次调度
            } else {
                task->m_sleep_time_slice--;
            }
        }
    }
}

void multi_task_switch(unsigned char tr) {
    if (g_multi_task_ctl->m_current_tr == tr)
        return;

    g_multi_task_ctl->m_current_tr = tr;

    if (g_multi_task_ctl->m_current_tr > 0) {
        g_switch_task_counts++;
        farjmp(0, g_multi_task_ctl->m_current_tr << 3);
    }
}

void multi_task_yeild(unsigned char tr) {
    unsigned int counts = g_switch_task_counts;
    g_multi_task_ctl->m_next_tr = tr;

#ifdef __MULTI_TASK_DEBUG__
    static unsigned int _multi_task_yeild_debug_count = 0;
#endif

    // 等待任务切换完成, 在任务切换后g_switch_task_counts值会变。
    // 再次切换回来后判断就会为false，从而跳出死循环
    while (counts == g_switch_task_counts) {
        io_delay();

#ifdef __MULTI_TASK_DEBUG__
        show_string_in_canvas(FONT_WIDTH * 20, FONT_HEIGHT, COL8_FFFFFF,
                              int2hexstr(_multi_task_yeild_debug_count++));
#endif
    }
}

void set_segmdesc(segment_descriptor_t *sd, unsigned int limit,
                  unsigned int base, unsigned int ar) {
    if (limit > 0xfffff) {
        ar |= 0x8000; // G_bit = 1
        limit /= 0x1000;
    }

    sd->m_limit_low = limit & 0xffff;
    sd->m_base_low = base & 0xffff;
    sd->m_base_mid = (base >> 16) & 0xff;
    sd->m_access_right = ar & 0xff;
    sd->m_limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->m_base_high = (unsigned char)((base >> 24) & 0xff);
}
