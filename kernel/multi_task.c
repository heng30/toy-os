#include "multi_task.h"
#include "colo8.h"
#include "draw.h"
#include "io.h"
#include "kutil.h"
#include "memory.h"
#include "string.h"

#include "widgets/canvas.h"

static volatile unsigned int g_switch_task_counts = 0; // 任务切换次数

multi_task_ctl_t *g_multi_task_ctl = NULL;

static void _wait_task_schedul(void) {
    unsigned int counts = g_switch_task_counts;

    // 等待任务切换完成, 在任务切换后g_switch_task_counts值会变。
    // 再次切换回来后判断就会为false，从而跳出死循环
    while (counts == g_switch_task_counts) {
        io_delay();
    }
}

// 获取下一个可运行的任务
static unsigned char _get_next_availible_task(unsigned char current_index) {
    for (unsigned int i = current_index + 1;
         i < (unsigned int)g_multi_task_ctl->m_tasks_counts; i++) {
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

// 获取下一个可运行的任务
static unsigned char _get_next_availible_tr(unsigned char current_tr) {
    for (unsigned char i = 0; i < g_multi_task_ctl->m_tasks_counts; i++) {
        task_t *task = g_multi_task_ctl->m_tasks[i];
        if (task->m_tr != current_tr && task->m_flags == TASK_STATUS_RUNNING) {
            return i;
        }
    }

    return current_tr;
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

void set_gate(gate_descriptor_t *sg, unsigned int offset,
              unsigned short selector, unsigned short ar) {
    sg->m_offset_low = offset & 0xffff;
    sg->m_selector = selector;
    sg->m_dw_count = (unsigned char)(ar >> 8) & 0xff;
    sg->m_access_right = (unsigned char)ar & 0xff;
    sg->m_offset_hight = (unsigned short)(offset >> 16) & 0xffff;
}

void init_multi_task_ctl(void) {
    assert(MAX_TASKS <= 256, "init_multi_task_ctl MAX_TASKS is more than 256");

    g_multi_task_ctl =
        (multi_task_ctl_t *)memman_alloc_4k(sizeof(multi_task_ctl_t));

    assert(g_multi_task_ctl != NULL, "init_multi_task_ctl alloc 4k error");

    g_multi_task_ctl->m_priority_tasks = ring_alloc(MAX_TASKS);

    segment_descriptor_t *gdt = (segment_descriptor_t *)get_addr_gdt();
    for (unsigned char i = 0; i < MAX_TASKS; i++) {
        g_multi_task_ctl->m_tasks0[i].m_flags = TASK_STATUS_UNUSED;
        g_multi_task_ctl->m_tasks0[i].m_tr = TASK_GDT0 + i;

        // 设置全局描述附表中的段描述符为TSS32对象
        set_segmdesc(gdt + TASK_GDT0 + i, LIMIT_TSS32,
                     (ptr_t)&g_multi_task_ctl->m_tasks0[i].m_tss, AR_TSS32);
    }

    g_multi_task_ctl->m_tasks_counts = 0;
    g_multi_task_ctl->m_statistics.m_total_task_counts = MAX_TASKS;
    g_multi_task_ctl->m_statistics.m_used_task_counts = 0;
    g_multi_task_ctl->m_statistics.m_running_task_counts = 0;
    g_multi_task_ctl->m_statistics.m_sleep_task_counts = 0;
    g_multi_task_ctl->m_statistics.m_suspend_task_counts = 0;

    // 此处传入NULL不会有问题，因为这个是主任务，也是正在运行的任务。
    // 在进行第一次任务切换时，会更新eip的值。
    task_t *task =
        multi_task_alloc((ptr_t)NULL, 0, NULL, DEFAULT_RUNNING_TIME_SLICE);
    assert(task->m_tr == TASK_GDT0, "init_multi_task_ctl invalid main task tr");
    g_multi_task_ctl->m_current_task = task;
    multi_task_run(task);

    // 在处理器切换到保护模式之后，可以用LTR指令把TSS段描述符的选择符加载到任务寄存器TR中。这个指令会把TSS标记成忙状态（B=1），但是并不执行任务切换操作。然后处理器可以使用这个TSS来定位特权级0、1和2的堆栈。在保护模式中，软件进行第一次任务切换之前必须首先加载TSS段的选择符，因为任务切换会把当前任务状态复制到该TSS中。在LTR指令执行之后，随后对任务寄存器的操作由任务切换进行。
    // 确保第一次任务切换时，当前的TSS32值保存到`task->m_tss`中
    load_tr(task->m_tr << 3);
}

task_t *multi_task_alloc(ptr_t task_main, unsigned int argc, void *argv[],
                         unsigned int running_time_slice) {
    for (unsigned char i = 0; i < MAX_TASKS; i++) {
        if (g_multi_task_ctl->m_tasks0[i].m_flags == TASK_STATUS_UNUSED) {
            unsigned int addr_code32 = get_code32_addr();
            unsigned int addr_stack_stack = get_stack_start_addr();

            task_t *task = &g_multi_task_ctl->m_tasks0[i];
            task->m_flags = TASK_STATUS_USED;
            task->m_running_time_slice = running_time_slice;
            task->m_remain_time_slice = running_time_slice;
            task->m_sleep_time_slice = 0;
            task->m_is_priority_task = false;
            task->m_ref = 1;

            memset((unsigned char *)&task->m_tss, 0, sizeof(task->m_tss));

            task->m_tss.m_eflags = 0x00000202;
            task->m_tss.m_eax = 0;
            task->m_tss.m_ecx = 0;
            task->m_tss.m_edx = 0;
            task->m_tss.m_ebx = 0;
            task->m_tss.m_ebp = 0; // 帧寄存器
            task->m_tss.m_esp0 = 0;
            task->m_tss.m_ss0 = 0;

            // 任务堆栈指针位置，需要和kernel.asm中的堆栈对应
            task->m_tss.m_esp = (unsigned int)TASK_STACK_SIZE * (i + 1);

            // 函数调用堆栈结构:
            // 函数参数从右到左依次入栈，调用者下一条执行代码地址入栈
            // 参数从右到左入栈
            if (argc > 0 && argv) {
                for (unsigned int i = argc; i != 0; i--) {
                    task->m_tss.m_esp -= sizeof(unsigned int);
                    *(unsigned int *)(addr_stack_stack + task->m_tss.m_esp) =
                        (unsigned int)argv[i - 1];
                }
            }

            // 第一个参数入栈
            task->m_tss.m_esp -= sizeof(unsigned int);
            *(unsigned int *)(addr_stack_stack + task->m_tss.m_esp) =
                (unsigned int)task;

            // 调用者下一条执行代码地址。任务是不会返回的，所以不需要关心。这里直接跳过
            task->m_tss.m_esp -= sizeof(unsigned int);

            task->m_tss.m_esi = 0;
            task->m_tss.m_edi = 0;
            task->m_tss.m_ldtr = 0;
            task->m_tss.m_iomap = 0x40000000;

            task->m_tss.m_eip = task_main - addr_code32;

            task->m_tss.m_es = 0;
            task->m_tss.m_fs = 0;

            // 每个段描述符占8字节
            task->m_tss.m_cs = 1 * 8; // 代码段描述符在全局描述符表的第1位
            task->m_tss.m_gs = 2 * 8; // 显存段描述符在全局描述符表的第2位
            task->m_tss.m_ds = 3 * 8; // 数据段描述符在全局描述符表的第3位
            task->m_tss.m_ss = 4 * 8; // 堆栈段描述符在全局描述符表的第4位

            g_multi_task_ctl->m_statistics.m_used_task_counts++;
            return task;
        }
    }

    assert(false, "multi_task_alloc failed");
    return NULL;
}

void multi_task_free(task_t *task) {
    for (unsigned char i = 0; i < g_multi_task_ctl->m_tasks_counts; i++) {
        if (task == g_multi_task_ctl->m_tasks[i]) {

            int eflags = io_load_eflags();
            io_cli(); // 暂时停止接收中断信号

            // 更新统计信息
            g_multi_task_ctl->m_statistics.m_used_task_counts--;
            if (task->m_flags == TASK_STATUS_RUNNING) {
                g_multi_task_ctl->m_statistics.m_running_task_counts--;
            } else if (task->m_flags == TASK_STATUS_SLEEP) {
                g_multi_task_ctl->m_statistics.m_sleep_task_counts--;
            } else if (task->m_flags == TASK_STATUS_SUSPEND) {
                g_multi_task_ctl->m_statistics.m_suspend_task_counts--;
            }

            task->m_flags = TASK_STATUS_UNUSED;

            // 剩余的任务迁移一位，覆盖掉被移除的任务
            for (unsigned char j = i; j < g_multi_task_ctl->m_tasks_counts - 1;
                 j++) {
                g_multi_task_ctl->m_tasks[j] = g_multi_task_ctl->m_tasks[j + 1];
            }

            g_multi_task_ctl->m_tasks_counts--;

            io_store_eflags(eflags); // 恢复接收中断信号

            return;
        }
    }
}

void multi_task_run(task_t *task) {
    int eflags = io_load_eflags();
    io_cli(); // 暂时停止接收中断信号

    assert(task->m_flags == TASK_STATUS_USED,
           "multi_task_run wrong task status. only `used task` can call this "
           "function");

    task->m_flags = TASK_STATUS_RUNNING;
    g_multi_task_ctl->m_tasks[g_multi_task_ctl->m_tasks_counts] = task;
    g_multi_task_ctl->m_tasks_counts++;
    g_multi_task_ctl->m_statistics.m_running_task_counts++;
    io_store_eflags(eflags); // 恢复接收中断信号
}

void multi_task_resume(task_t *task) {
    int eflags = io_load_eflags();
    io_cli(); // 暂时停止接收中断信号

    assert(task->m_flags == TASK_STATUS_SUSPEND,
           "multi_task_resume wrong task status. only suspend task can resume");

    task->m_flags = TASK_STATUS_RUNNING;
    g_multi_task_ctl->m_statistics.m_suspend_task_counts--;
    g_multi_task_ctl->m_statistics.m_running_task_counts++;
    io_store_eflags(eflags); // 恢复接收中断信号
}

void multi_task_suspend(task_t *task) {
    int eflags = io_load_eflags();
    io_cli(); // 暂时停止接收中断信号

    assert(
        task->m_flags == TASK_STATUS_RUNNING,
        "multi_task_suspend wrong task status. only running task can suspend");

    assert(task == g_multi_task_ctl->m_current_task,
           "multi_task_suspend can only make the current task suspend");

    task->m_flags = TASK_STATUS_SUSPEND;
    g_multi_task_ctl->m_statistics.m_running_task_counts--;
    g_multi_task_ctl->m_statistics.m_suspend_task_counts++;
    io_store_eflags(eflags); // 恢复接收中断信号

    _wait_task_schedul();
}

void multi_task_sleep(task_t *task, unsigned int sleep_time_slice) {
    int eflags = io_load_eflags();
    io_cli(); // 暂时停止接收中断信号

    assert(task->m_flags == TASK_STATUS_RUNNING,
           "multi_task_sleep wrong task status. only running task can sleep");

    assert(task == g_multi_task_ctl->m_current_task,
           "multi_task_sleep can only make the current task sleep");

    unsigned char tr = task->m_tr;
    task->m_flags = TASK_STATUS_SLEEP;
    task->m_sleep_time_slice = sleep_time_slice;
    g_multi_task_ctl->m_statistics.m_running_task_counts--;
    g_multi_task_ctl->m_statistics.m_sleep_task_counts++;
    io_store_eflags(eflags); // 恢复接收中断信号

    _wait_task_schedul();
}

// 减少睡眠任务的睡眠时间片
static void _decrease_sleep_task_time_slice(void) {
    for (unsigned char i = 0; i < g_multi_task_ctl->m_tasks_counts; i++) {
        task_t *task = g_multi_task_ctl->m_tasks[i];

        if (task->m_flags == TASK_STATUS_SLEEP) {
            if (task->m_sleep_time_slice == 0) {
                task->m_flags = TASK_STATUS_RUNNING; // 等待下一次调度
                g_multi_task_ctl->m_statistics.m_running_task_counts++;
                g_multi_task_ctl->m_statistics.m_sleep_task_counts--;
            } else {
                task->m_sleep_time_slice--;
            }
        }
    }
}

void multi_task_schedul(void) {
    static task_t *task_interupt_by_priority_task = NULL;

    task_t *priority_task =
        (task_t *)ring_get(g_multi_task_ctl->m_priority_tasks);

    // 找到优先任务
    if (priority_task) {
        _decrease_sleep_task_time_slice();

        // 设置被中断任务，以便下次恢复
        if (!task_interupt_by_priority_task) {
            task_interupt_by_priority_task = g_multi_task_ctl->m_current_task;
        }

        priority_task->m_is_priority_task = false;
        multi_task_switch(priority_task);

        return;
    }

    // 恢复被中断的任务
    if (task_interupt_by_priority_task) {
        _decrease_sleep_task_time_slice();

        if (task_interupt_by_priority_task->m_flags == TASK_STATUS_RUNNING) {
            task_t *tk = task_interupt_by_priority_task;
            task_interupt_by_priority_task = NULL;
            multi_task_switch(tk);
        } else { // 被中断的任务不能执行
            task_interupt_by_priority_task = NULL;

            // 获取一个可以运行的任务
            if (g_multi_task_ctl->m_tasks_counts > 0) {
                unsigned char next_index = _get_next_availible_task(0);
                task_t *tk = g_multi_task_ctl->m_tasks[next_index];
                multi_task_switch(tk);
            }
        }

        return;
    }

    // 运行普通任务
    for (unsigned char i = 0; i < g_multi_task_ctl->m_tasks_counts; i++) {
        task_t *task = g_multi_task_ctl->m_tasks[i];

        // 减少睡眠任务的睡眠时间片
        if (task->m_flags == TASK_STATUS_SLEEP) {
            if (task->m_sleep_time_slice == 0) {
                task->m_flags = TASK_STATUS_RUNNING; // 等待下一次调度
                g_multi_task_ctl->m_statistics.m_running_task_counts++;
                g_multi_task_ctl->m_statistics.m_sleep_task_counts--;
            } else {
                task->m_sleep_time_slice--;
            }
        }

        // 找到当前正在运行的任务
        if (task == g_multi_task_ctl->m_current_task) {
            // 时间片用完，进行任务切换
            if (task->m_remain_time_slice == 0 ||
                task->m_flags == TASK_STATUS_SLEEP ||
                task->m_flags == TASK_STATUS_SUSPEND) {
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

                    multi_task_switch(next_task);
                }
            } else {
                task->m_remain_time_slice--;
            }
        }
    }
}

void multi_task_switch(task_t *task) {
    if (g_multi_task_ctl->m_current_task == task)
        return;

    g_multi_task_ctl->m_current_task = task;

    g_switch_task_counts++;
    farjmp(0, g_multi_task_ctl->m_current_task->m_tr << 3);
}

bool multi_task_priority_task_add(task_t *task) {
    int eflags = io_load_eflags();
    io_cli(); // 暂时停止接收中断信号

    // 已经在优先队列中则不必重复添加
    if (task->m_is_priority_task)
        goto ok;

    if (ring_put(g_multi_task_ctl->m_priority_tasks, task)) {
        task->m_flags = TASK_STATUS_RUNNING;
        task->m_is_priority_task = true;
        goto ok;
    }

    io_store_eflags(eflags); // 恢复接收中断信号
    return false;

ok:
    io_store_eflags(eflags); // 恢复接收中断信号
    return true;
}

// 增加1次引用
void multi_task_ref_inc(task_t *task) { task->m_ref++; }

// 减少1次引用
void multi_task_ref_dec(task_t *task) {
    if (task->m_ref == 0)
        return;

    task->m_ref--;
}

void multi_task_statistics_display(void) {
    multi_task_statistics_t *stats = &g_multi_task_ctl->m_statistics;

    const char *headers[] = {
        "total tasks counts:",  "used tasks counts:",
        "unused tasks counts:", "running tasks counts:",
        "sleep tasks counts:",  "suspend tasks counts:",
        "array tasks counts:",  "priority task counts:",
        "current_tr",
    };

    unsigned int datas[] = {
        stats->m_total_task_counts,
        stats->m_used_task_counts,
        stats->m_total_task_counts - stats->m_used_task_counts,
        stats->m_running_task_counts,
        stats->m_sleep_task_counts,
        stats->m_suspend_task_counts,
        g_multi_task_ctl->m_tasks_counts,
        ring_len(g_multi_task_ctl->m_priority_tasks),
        g_multi_task_ctl->m_current_task->m_tr,
    };

    for (unsigned int i = 0, start_x = 0; i < sizeof(datas) / sizeof(datas[0]);
         i++) {
        unsigned int start_y = FONT_HEIGHT + i * FONT_HEIGHT;

        show_string_in_canvas(start_x, start_y, COL8_FFFFFF, headers[i]);
        show_string_in_canvas(start_x + FONT_WIDTH * 25, start_y, COL8_FFFFFF,
                              int2hexstr(datas[i]));
    }
}
