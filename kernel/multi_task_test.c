
#include "multi_task_test.h"
#include "colo8.h"
#include "draw.h"
#include "multi_task.h"

#ifdef __MULTI_TASK_TEST_WITHOUT_SCHEDUL__
#include "fifo8.h"
#include "input_cursor.h"
#include "io.h"
#include "kutil.h"

/*************************************单次任务切换测试****************************/
// 测试任务B
static void _multi_task_test_task_b_main(void) {
    show_string_in_canvas(0, 160, COL8_FFFFFF, "enter task b");

    unsigned int counter = 1;
    timer_t *timer = timer_alloc();
    set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE * 3, 1,
              MULTI_TASK_TEST_B_MAIN_TIMER_DATA);

    for (;;) {
        io_cli();
        if (fifo8_is_empty(&g_timerctl.m_fifo)) {
            io_sti();
        } else {
            unsigned char data = (unsigned char)fifo8_get(&g_timerctl.m_fifo);
            io_sti();

            switch (data) {
            case MULTI_TASK_TEST_B_MAIN_TIMER_DATA:
                show_string_in_canvas(0, 176, COL8_FFFFFF,
                                      "switch back to main task");

                show_string_in_canvas(0, 176 + 32, COL8_FFFFFF,
                                      int2hexstr(counter++));
                taskswitch7(); // 切换回到主任务
                break;
            default:
                break;
            }
        }
    }
}

// 切换到任务B
static void _multi_task_test_switch_to_task_b(void) { taskswitch9(); }

void multi_task_test_in_main_timer_callback(void) {
    show_string_in_canvas(0, 144, COL8_FFFFFF, "switch to task b");
    _multi_task_test_switch_to_task_b();
    show_string_in_canvas(0, 176 + 16, COL8_FFFFFF, "enter task main");

    // 重新启动光标定时器
    set_timer(g_input_cursor_timer, TIMER_INPUT_CURSOR_TIME_SLICE, 1,
              INPUT_CURSOR_TIMER_DATA);
}

void multi_task_test(void) {
    unsigned int addr_code32 = get_code32_addr();
    segment_descriptor_t *gdt = (segment_descriptor_t *)get_addr_gdt();

    static TSS32_t tss_a, tss_b;
    tss_a.m_ldtr = 0, tss_a.m_iomap = 0x40000000;
    tss_b.m_ldtr = 0, tss_b.m_iomap = 0x40000000;

    // 设置TSS32对象地址到段描述符中
    set_segmdesc(gdt + 7, 103, (ptr_t)&tss_a,
                 AR_TSS32); // 需要和TASK_GDT0保持一致
    set_segmdesc(gdt + 8, 103, (ptr_t)&tss_a, AR_TSS32);

    set_segmdesc(gdt + 9, 103, (ptr_t)&tss_b, AR_TSS32); // 任务B的TSS32对象

    // 设置函数地址到段描述符
    set_segmdesc(gdt + 6, 0xffff, (ptr_t)_multi_task_test_task_b_main,
                 AR_FUNCTION);

    // 告诉cpu第gdt+7位置是一个TSS32对象，任务切换是保存当前的TSS32对象到这个位置
    load_tr(7 << 3);

    // 保存当前的TSS32到gdt+7的位置,并切换到gdt+8位置的TSS32.
    // 因为`gdt+7`和`gdt+8`对应同一个TSS32对象,
    // 而且7处的大部分寄存器值都为0.
    // 所以这是一个自身任务的切换操作，不过可以获取到当前的TSS32值
    taskswitch8();

    // 打印当前的TSS32对象信息
    char *p = int2hexstr(tss_a.m_eflags);
    show_string_in_canvas(0, 0, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.m_esp);
    show_string_in_canvas(0, 16, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.m_es / 8);
    show_string_in_canvas(0, 32, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.m_cs / 8);
    show_string_in_canvas(0, 48, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.m_ss / 8);
    show_string_in_canvas(0, 64, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.m_ds / 8);
    show_string_in_canvas(0, 80, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.m_gs / 8);
    show_string_in_canvas(0, 96, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.m_fs / 8);
    show_string_in_canvas(0, 112, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.m_cr3);
    show_string_in_canvas(0, 128, COL8_FFFFFF, p);

    // 设置任务B的TSS32对象
    tss_b.m_eip = (unsigned int)_multi_task_test_task_b_main - addr_code32;
    tss_b.m_eflags = 0x00000202;
    tss_b.m_eax = 0;
    tss_b.m_ecx = 0;
    tss_b.m_edx = 0;
    tss_b.m_ebx = 0;

    // 因为有俩个堆栈，每个1024字节，可以查看kernel.asm代码
    tss_b.m_esp = 1024 * 2;

    tss_b.m_ebp = 0;
    tss_b.m_esi = 0;
    tss_b.m_edi = 0;
    tss_b.m_es = tss_a.m_es;
    tss_b.m_cs = tss_a.m_cs;
    tss_b.m_ss = tss_a.m_ss;
    tss_b.m_ds = tss_a.m_ds;
    tss_b.m_fs = tss_a.m_fs;
    tss_b.m_gs = tss_a.m_gs;

    // 启动定器
    timer_t *timer = timer_alloc();
    set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE * 3, 1,
              MULTI_TASK_TEST_B_MAIN_TIMER_DATA);
}

/*************************************交替打印AB测试****************************/
// 测试任务B
static void _multi_task_test_task_b_main_auto(void) {
    show_string_in_canvas(0, 160, COL8_FFFFFF, "enter task b auto");

    timer_t *timer = timer_alloc();
    set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE, 1,
              MULTI_TASK_TEST_B_MAIN_TIMER_AUTO_DATA);

    unsigned int pos = 0;
    unsigned int counter = 0;
    for (;;) {
        show_string_in_canvas(FONT_WIDTH * 15, 0, COL8_FFFFFF,
                              int2hexstr(counter++));
        io_cli();
        if (fifo8_is_empty(&g_timerctl.m_fifo)) {
            io_sti();
        } else {
            unsigned char data = (unsigned char)fifo8_get(&g_timerctl.m_fifo);
            io_sti();

            switch (data) {
            case MULTI_TASK_TEST_B_MAIN_TIMER_AUTO_DATA:
                show_string_in_canvas(0, 176, COL8_FFFFFF,
                                      "switch back to main task auto");

                show_string_in_canvas(pos, 250 + FONT_HEIGHT, COL8_FFFFFF, "B");
                pos += 8;

                set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE, 1,
                          MULTI_TASK_TEST_B_MAIN_TIMER_AUTO_DATA);

                multi_task_yeild(7); // gdt+7跳回到主线程

                set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE, 1,
                          MULTI_TASK_TEST_B_MAIN_TIMER_AUTO_DATA);
                break;
            default:
                break;
            }
        }
    }
}

void multi_task_test_in_main_timer_callback_auto(void) {
    static unsigned int _main_timer_callback_auto_counter = 1;
    static unsigned int _main_timer_callback_auto_pos = 0;

    show_string_in_canvas(_main_timer_callback_auto_pos, 250, COL8_FFFFFF, "A");
    _main_timer_callback_auto_pos += 8;

    show_string_in_canvas(0, 176 + 32, COL8_FFFFFF,
                          int2hexstr(_main_timer_callback_auto_counter++));

    show_string_in_canvas(0, 144, COL8_FFFFFF, "switch to task b auto");
    multi_task_yeild(9); // 切换到任务B
    show_string_in_canvas(0, 176 + 16, COL8_FFFFFF, "enter task main auto");

    // 重新启动光标定时器
    set_timer(g_input_cursor_timer, TIMER_INPUT_CURSOR_TIME_SLICE, 1,
              INPUT_CURSOR_TIMER_DATA);
}

void multi_task_test_auto(void) {
    unsigned int addr_code32 = get_code32_addr();
    segment_descriptor_t *gdt = (segment_descriptor_t *)get_addr_gdt();

    static TSS32_t tss_a, tss_b;
    tss_a.m_ldtr = 0, tss_a.m_iomap = 0x40000000;
    tss_b.m_ldtr = 0, tss_b.m_iomap = 0x40000000;

    // 设置TSS32对象地址到段描述符中
    set_segmdesc(gdt + 7, 103, (ptr_t)&tss_a,
                 AR_TSS32); // 需要和TASK_GDT0保持一致
    set_segmdesc(gdt + 8, 103, (ptr_t)&tss_a, AR_TSS32);

    set_segmdesc(gdt + 9, 103, (ptr_t)&tss_b, AR_TSS32); // 任务B的TSS32对象

    // 告诉cpu第gdt+7位置是一个TSS32对象，任务切换是保存当前的TSS32对象到这个位置
    load_tr(7 << 3);

    // 保存当前的TSS32到gdt+7的位置,并切换到gdt+8位置的TSS32.
    // 因为`gdt+7`和`gdt+8`对应同一个TSS32对象,
    // 而且7处的大部分寄存器值都为0.
    // 所以这是一个自身任务的切换操作，不过可以获取到当前的TSS32值
    taskswitch8();

    // 设置任务B的TSS32对象
    tss_b.m_eip = (unsigned int)_multi_task_test_task_b_main_auto - addr_code32;
    tss_b.m_eflags = 0x00000202;
    tss_b.m_eax = 0;
    tss_b.m_ecx = 0;
    tss_b.m_edx = 0;
    tss_b.m_ebx = 0;

    // 因为有俩个堆栈，每个1024字节，可以查看kernel.asm代码
    tss_b.m_esp = 1024 * 2;

    tss_b.m_ebp = 0;
    tss_b.m_esi = 0;
    tss_b.m_edi = 0;
    tss_b.m_es = tss_a.m_es;
    tss_b.m_cs = tss_a.m_cs;
    tss_b.m_ss = tss_a.m_ss;
    tss_b.m_ds = tss_a.m_ds;
    tss_b.m_fs = tss_a.m_fs;
    tss_b.m_gs = tss_a.m_gs;

    timer_t *timer = timer_alloc();
    set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE, 1,
              MULTI_TASK_TEST_B_MAIN_TIMER_AUTO_DATA);
}

#else

void print_A(task_t *task) {
    static unsigned int print_A_pos = 0;

    for (;;) {
        show_string_in_canvas(print_A_pos, 250, COL8_FFFFFF, "A");
        print_A_pos += 8;

        multi_task_sleep(task, TIMER_ONE_SECOND_TIME_SLICE);
    }
}

void print_B(task_t *task) {
    static unsigned int print_B_pos = 0;
    for (;;) {
        show_string_in_canvas(print_B_pos, 250 + FONT_HEIGHT, COL8_FFFFFF, "B");
        print_B_pos += 8;

        multi_task_sleep(task, TIMER_ONE_SECOND_TIME_SLICE);
    }
}

void multi_task_test_schedul(void) {
    unsigned int addr_code32 = get_code32_addr();
    task_t *task = multi_task_alloc(TIMER_ONE_SECOND_TIME_SLICE);

    task->m_tss.m_eip = print_B - addr_code32;
    task->m_tss.m_es = 0;
    task->m_tss.m_cs = 1 * 8; // 6 * 8;
    task->m_tss.m_ss = 4 * 8;
    task->m_tss.m_ds = 3 * 8;
    task->m_tss.m_fs = 0;
    task->m_tss.m_gs = 2 * 8;
}
#endif
