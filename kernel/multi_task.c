#include "multi_task.h"
#include "colo8.h"
#include "draw.h"
#include "fifo8.h"
#include "input_cursor.h"
#include "io.h"
#include "kutil.h"

unsigned char g_multi_task_prev_tr = 0;
unsigned char g_multi_task_current_tr = 0;
unsigned char g_multi_task_next_tr = 0;
static volatile unsigned int g_switch_task_counts = 0; // 任务切换次数

void init_multi_task(unsigned char tr) {
    g_multi_task_prev_tr = tr;
    g_multi_task_current_tr = tr;
    g_multi_task_next_tr = tr;
}

void multi_task_switch(unsigned char tr) {
    if (g_multi_task_current_tr == tr)
        return;

    g_multi_task_prev_tr = g_multi_task_current_tr;
    g_multi_task_current_tr = tr;

    if (g_multi_task_current_tr > 0) {
        g_switch_task_counts++;
        farjmp(0, g_multi_task_current_tr << 3);
    }
}

void yeild(unsigned char tr) {
    unsigned int counts = g_switch_task_counts;
    g_multi_task_next_tr = tr;

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

    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (unsigned char)((base >> 24) & 0xff);
}

/*************************************单次任务切换测试****************************/
// 测试任务B
static void _multi_task_test_task_b_main(void) {
    show_string_in_canvas(0, 160, COL8_FFFFFF, "enter task b");

    unsigned int counter = 1;
    timer_t *timer = timer_alloc();
    set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE * 3,
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
    set_timer(g_input_cursor_timer, TIMER_INPUT_CURSOR_TIME_SLICE,
              INPUT_CURSOR_TIMER_DATA);
}

void multi_task_test(void) {
    unsigned int addr_code32 = get_code32_addr();
    segment_descriptor_t *gdt = (segment_descriptor_t *)get_addr_gdt();

    static TSS32_t tss_a, tss_b;
    tss_a.ldtr = 0, tss_a.iomap = 0x40000000;
    tss_b.ldtr = 0, tss_b.iomap = 0x40000000;

    // 设置TSS32对象地址到段描述符中
    set_segmdesc(gdt + 7, 103, (ptr_t)&tss_a, AR_TSS32);
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
    char *p = int2hexstr(tss_a.eflags);
    show_string_in_canvas(0, 0, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.esp);
    show_string_in_canvas(0, 16, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.es / 8);
    show_string_in_canvas(0, 32, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.cs / 8);
    show_string_in_canvas(0, 48, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.ss / 8);
    show_string_in_canvas(0, 64, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.ds / 8);
    show_string_in_canvas(0, 80, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.gs / 8);
    show_string_in_canvas(0, 96, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.fs / 8);
    show_string_in_canvas(0, 112, COL8_FFFFFF, p);

    p = int2hexstr(tss_a.cr3);
    show_string_in_canvas(0, 128, COL8_FFFFFF, p);

    // 设置任务B的TSS32对象
    tss_b.eip = (unsigned int)_multi_task_test_task_b_main - addr_code32;
    tss_b.eflags = 0x00000202;
    tss_b.eax = 0;
    tss_b.ecx = 0;
    tss_b.edx = 0;
    tss_b.ebx = 0;

    // 因为有俩个堆栈，每个1024字节，可以查看kernel.asm代码
    tss_b.esp = 1024 * 2;

    tss_b.ebp = 0;
    tss_b.esi = 0;
    tss_b.edi = 0;
    tss_b.es = tss_a.es;
    tss_b.cs = tss_a.cs;
    tss_b.ss = tss_a.ss;
    tss_b.ds = tss_a.ds;
    tss_b.fs = tss_a.fs;
    tss_b.gs = tss_a.gs;

    // 启动定器
    timer_t *timer = timer_alloc();
    set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE * 3,
              MULTI_TASK_TEST_B_MAIN_TIMER_DATA);
}

/*************************************交替打印AB测试****************************/
// 测试任务B
static void _multi_task_test_task_b_main_auto(void) {
    show_string_in_canvas(0, 160, COL8_FFFFFF, "enter task b auto");

    timer_t *timer = timer_alloc();
    set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE,
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

                set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE,
                          MULTI_TASK_TEST_B_MAIN_TIMER_AUTO_DATA);

                yeild(7); // gdt+7跳回到主线程

                set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE,
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
    yeild(9); // 切换到任务B
    show_string_in_canvas(0, 176 + 16, COL8_FFFFFF, "enter task main auto");

    // 重新启动光标定时器
    set_timer(g_input_cursor_timer, TIMER_INPUT_CURSOR_TIME_SLICE,
              INPUT_CURSOR_TIMER_DATA);
}

void multi_task_test_auto(void) {
    unsigned int addr_code32 = get_code32_addr();
    segment_descriptor_t *gdt = (segment_descriptor_t *)get_addr_gdt();

    static TSS32_t tss_a, tss_b;
    tss_a.ldtr = 0, tss_a.iomap = 0x40000000;
    tss_b.ldtr = 0, tss_b.iomap = 0x40000000;

    // 设置TSS32对象地址到段描述符中
    set_segmdesc(gdt + 7, 103, (ptr_t)&tss_a, AR_TSS32);
    set_segmdesc(gdt + 8, 103, (ptr_t)&tss_a, AR_TSS32);

    set_segmdesc(gdt + 9, 103, (ptr_t)&tss_b, AR_TSS32); // 任务B的TSS32对象

    // 设置函数地址到段描述符
    set_segmdesc(gdt + 6, 0xffff, (ptr_t)_multi_task_test_task_b_main_auto,
                 AR_FUNCTION);

    // 告诉cpu第gdt+7位置是一个TSS32对象，任务切换是保存当前的TSS32对象到这个位置
    load_tr(7 << 3);

    // 保存当前的TSS32到gdt+7的位置,并切换到gdt+8位置的TSS32.
    // 因为`gdt+7`和`gdt+8`对应同一个TSS32对象,
    // 而且7处的大部分寄存器值都为0.
    // 所以这是一个自身任务的切换操作，不过可以获取到当前的TSS32值
    taskswitch8();

    // 设置任务B的TSS32对象
    tss_b.eip = (unsigned int)_multi_task_test_task_b_main_auto - addr_code32;
    tss_b.eflags = 0x00000202;
    tss_b.eax = 0;
    tss_b.ecx = 0;
    tss_b.edx = 0;
    tss_b.ebx = 0;

    // 因为有俩个堆栈，每个1024字节，可以查看kernel.asm代码
    tss_b.esp = 1024 * 2;

    tss_b.ebp = 0;
    tss_b.esi = 0;
    tss_b.edi = 0;
    tss_b.es = tss_a.es;
    tss_b.cs = tss_a.cs;
    tss_b.ss = tss_a.ss;
    tss_b.ds = tss_a.ds;
    tss_b.fs = tss_a.fs;
    tss_b.gs = tss_a.gs;

    timer_t *timer = timer_alloc();
    set_timer(timer, TIMER_ONE_SECOND_TIME_SLICE,
              MULTI_TASK_TEST_B_MAIN_TIMER_AUTO_DATA);
}
