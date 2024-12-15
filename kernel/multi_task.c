#include "multi_task.h"
#include "io.h"
#include "draw.h"

unsigned char g_multi_task_current_tr = 0;
unsigned char g_multi_task_next_tr = 0;
static volatile unsigned int g_switch_task_counts = 0; // 任务切换次数

void init_multi_task(unsigned char tr) {
    g_multi_task_current_tr = tr;
    g_multi_task_next_tr = tr;
}

void multi_task_switch(unsigned char tr) {
    if (g_multi_task_current_tr == tr)
        return;

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

    sd->m_limit_low = limit & 0xffff;
    sd->m_base_low = base & 0xffff;
    sd->m_base_mid = (base >> 16) & 0xff;
    sd->m_access_right = ar & 0xff;
    sd->m_limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->m_base_high = (unsigned char)((base >> 24) & 0xff);
}

