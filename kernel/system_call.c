#include "system_call.h"
#include "colo8.h"
#include "draw.h"
#include "io.h"
#include "kutil.h"
#include "string.h"
#include "timer.h"
#include "win_sheet.h"

#include "widgets/console.h"
#include "widgets/window.h"

volatile unsigned int g_rand_number = 0;

static void _sc_no_window_errmsg(console_t *p, unsigned int win) {
    console_draw_text(p, "invalid win handler: ");
    console_draw_text(p, int2hexstr(win));
    console_move_to_next_line(p);
}

static void _sc_console_draw_ch(unsigned int eax) {
    console_t *p = console_get();
    console_draw_ch(p, (char)(eax & 0xff));
}

static void _sc_console_draw_text(unsigned int ebx) {
    console_t *p = console_get();
    if (!p->m_cmd->m_data)
        return;

    const char *text = (const char *)(p->m_cmd->m_data + ebx);
    console_draw_text(p, text);
}

static void _sc_console_draw_invalid_system_call(unsigned int edx) {
    console_t *p = console_get();
    console_draw_text(p, "invalid system call: ");
    console_draw_text(p, int2hexstr(edx));
    console_move_to_next_line(p);
}

static void _sc_new_window(ptr_t *reg, unsigned int x, unsigned int y,
                           unsigned int width, unsigned int height,
                           unsigned int title) {
    console_t *p = console_get();
    const char *real_title = (char *)p->m_cmd->m_data + title;
    width = min_unsigned(width, g_boot_info.m_screen_x);
    height = min_unsigned(height, g_boot_info.m_screen_y);

    window_t *win =
        window_new(x, y, width, height, WINDOW_ID_USER, real_title, NULL);

    window_ctl_add(win);
    window_focus(win);

    // 赋值到栈上的eax寄存器，popad后会赋值给eax寄存器
    // 作为函数调用的返回值
    reg[7] = (ptr_t)win;
}

static void _sc_close_window(unsigned int win) {
    window_ctl_close_window((window_t *)win);
}

static void _sc_refresh_window(unsigned int win, unsigned int x0,
                               unsigned int y0, unsigned int x1,
                               unsigned int y1) {
    console_t *p = console_get();
    window_t *pwin = (window_t *)win;

    if (window_ctl_is_window_exist(pwin)) {
        win_sheet_t *sht = pwin->m_sheet;
        win_sheet_refresh(sht, x0, y0, x1, y1);
    } else {
        _sc_no_window_errmsg(p, win);
    }
}

void _sc_is_close_window(ptr_t *reg) {
    window_t *p = g_window_ctl.m_focus_window;
    reg[7] = (p && p->m_id == WINDOW_ID_USER && p->m_is_waiting_for_close);
}
static void _sc_draw_text_in_window(unsigned int win, unsigned int x,
                                    unsigned int y, unsigned short col,
                                    unsigned int text) {
    console_t *p = console_get();
    window_t *pwin = (window_t *)win;

    if (window_ctl_is_window_exist(pwin)) {
        const char *real_text = p->m_cmd->m_data + text;
        unsigned char bg_col = (unsigned char)(col >> 8);
        unsigned char fg_col = (unsigned char)(col & 0xff);
        show_string(pwin->m_sheet, x, y, bg_col, fg_col, real_text);
    } else {
        _sc_no_window_errmsg(p, win);
    }
}

static void _sc_draw_box_in_window(unsigned int win, unsigned int x0,
                                   unsigned int y0, unsigned int x1,
                                   unsigned int y1, unsigned char col) {
    console_t *p = console_get();
    window_t *pwin = (window_t *)win;

    if (window_ctl_is_window_exist(pwin)) {
        win_sheet_t *sht = pwin->m_sheet;
        boxfill8(sht->m_buf, sht->m_bxsize, col, x0, y0, x1, y1);
        win_sheet_refresh(sht, x0, y0, x1 + 1, y1 + 1);
    } else {
        _sc_no_window_errmsg(p, win);
    }
}

static void _sc_draw_point_in_window(unsigned int win, unsigned int x,
                                     unsigned int y, unsigned char col) {
    console_t *p = console_get();
    window_t *pwin = (window_t *)win;

    if (window_ctl_is_window_exist(pwin)) {
        win_sheet_t *sht = pwin->m_sheet;
        boxfill8(sht->m_buf, sht->m_bxsize, col, x, y, x + 1, y + 1);
    } else {
        _sc_no_window_errmsg(p, win);
    }
}

static void _sc_draw_line_in_window(unsigned int win, unsigned int x0,
                                    unsigned int y0, unsigned int x1,
                                    unsigned int y1, unsigned char col) {

    console_t *p = console_get();
    window_t *pwin = (window_t *)win;

    if (window_ctl_is_window_exist(pwin)) {
        win_sheet_t *sht = pwin->m_sheet;
        draw_line(sht, (int)x0, (int)y0, (int)x1, (int)y1, col);
    } else {
        _sc_no_window_errmsg(p, win);
    }
}

static void _sc_rand_uint(ptr_t *reg, unsigned int seed) {
    g_rand_number = g_rand_number ^ seed; // 更新随机数
    reg[7] = g_rand_number;
}

static void _sc_show_debug_uint(unsigned int x, unsigned int y,
                                unsigned int num) {
    show_debug_string(x, y, COLOR_BLACK, int2hexstr(num));
}

static void _sc_timer_alloc(ptr_t *reg) {
    timer_t *p = timer_alloc();
    reg[7] = (ptr_t)p;
}

static void _sc_timer_free(unsigned int timer) {
    timer_t *p = (timer_t *)timer;
    if (timer_is_valid(p)) {
        timer_free(p);
    }
}

static void _sc_timer_set(unsigned int timer, unsigned int timeout,
                          unsigned int run_count) {
    timer_t *p = (timer_t *)timer;
    if (timer_is_valid(p)) {
        set_timer(p, timeout, run_count);
    }
}

static void _sc_timer_is_timeout(unsigned int timer) {
    timer_t *p = (timer_t *)timer;
    if (timer_is_valid(p)) {

        timer_is_timeout(p);
    }
}

// NOTE: swtch语句太长可能会出异常
static bool _sc_handle_window(ptr_t *reg, unsigned int edi, unsigned int esi,
                              unsigned int ebp, unsigned int esp,
                              unsigned int ebx, unsigned int edx,
                              unsigned int ecx, unsigned int eax) {
    bool is_handle = false;
    switch (edx) {
    case SYSTEM_CALL_NEW_WINDOW:
        _sc_new_window(reg, ebx, esi, edi, eax, ecx);
        is_handle = true;
        break;
    case SYSTEM_CALL_CLOSE_WINDOW:
        _sc_close_window(ebx);
        is_handle = true;
        break;
    case SYSTEM_CALL_REFRESH_WINDOW:
        _sc_refresh_window(ebx, eax, ecx, esi, edi);
        is_handle = true;
        break;
    case SYSTEM_CALL_IS_CLOSE_WINDOW:
        _sc_is_close_window(reg);
        is_handle = true;
        break;
    case SYSTEM_CALL_DRAW_TEXT_IN_WINDOW:
        _sc_draw_text_in_window(ebx, esi, edi, (unsigned short)eax, ecx);
        is_handle = true;
        break;
    case SYSTEM_CALL_DRAW_BOX_IN_WINDOW:
        _sc_draw_box_in_window(ebx, eax, ecx, esi, edi, (unsigned char)ebp);
        is_handle = true;
        break;
    case SYSTEM_CALL_DRAW_POINT_IN_WINDOW:
        _sc_draw_point_in_window(ebx, esi, edi, (unsigned char)eax);
        is_handle = true;
        break;
    case SYSTEM_CALL_DRAW_LINE_IN_WINDOW:
        _sc_draw_line_in_window(ebx, eax, ecx, esi, edi, (unsigned char)ebp);
        is_handle = true;
        break;
    case SYSTEM_CALL_CONSOLE_DRAW_CH:
        _sc_console_draw_ch(eax);
        is_handle = true;
        break;
    case SYSTEM_CALL_CONSOLE_DRAW_TEXT:
        _sc_console_draw_text(ebx);
        is_handle = true;
        break;
    default:
        break;
    }
    return is_handle;
}

static bool _sc_handle_timer(ptr_t *reg, unsigned int edi, unsigned int esi,
                             unsigned int ebp, unsigned int esp,
                             unsigned int ebx, unsigned int edx,
                             unsigned int ecx, unsigned int eax) {
    bool is_handle = false;
    switch (edx) {
    case SYSTEM_CALL_TIMER_ALLOC:
        _sc_timer_alloc(reg);
        is_handle = true;
        break;
    case SYSTEM_CALL_TIMER_FREE:
        _sc_timer_free(ebx);
        is_handle = true;
        break;
    case SYSTEM_CALL_TIMER_SET:
        _sc_timer_set(ebx, eax, ecx);
        is_handle = true;
        break;
    case SYSTEM_CALL_TIMER_IS_TIMEOUT:
        _sc_timer_is_timeout(ebx);
        is_handle = true;
        break;
    default:
        break;
    }
    return is_handle;
}

ptr_t *system_call_api(unsigned int edi, unsigned int esi, unsigned int ebp,
                       unsigned int esp, unsigned int ebx, unsigned int edx,
                       unsigned int ecx, unsigned int eax) {
    // - SYSTEM_CALL_HANDLER中断函数开头会执行两次pushad指令，
    // 一共压入了16各寄存器到堆栈中，第二次是为system_call_api函数传递参数。
    // 第一次是在system_call_api函数返回后恢复用户态寄存器值的。
    // 此处reg保存的是执行第一次pushad后的esp值, 即指向edi的指针
    // 而且最右侧参数先入栈.

    // - 这里需要加上addr_stack_start，是因为`&eax`获取的是相对于ss段基址，
    // 而通过`*eax`来访问内存是要相对于ds段基址的。
    // addr_stack_start刚好就是ss段基址与ds段基址之间的距离
    unsigned int addr_stack_start = get_stack_start_addr();
    unsigned int *reg =
        (unsigned int *)(addr_stack_start + (unsigned char *)(&eax + 1));

    if (edx == SYSTEM_CALL_END_CMD) {
        return &g_multi_task_ctl->m_current_task->m_tss.m_esp0;
    } else if (edx == SYSTEM_CALL_RAND_UINT) {
        _sc_rand_uint(reg, ebx);
    } else if (edx == SYSTEM_CALL_SHOW_DEBUG_UINT) {
        _sc_show_debug_uint(ebx, eax, ecx);
    } else if (_sc_handle_window(reg, edi, esi, ebp, esp, ebx, edx, ecx, eax)) {
        return NULL;
    } else if (_sc_handle_timer(reg, edi, esi, ebp, esp, ebx, edx, ecx, eax)) {
        return NULL;
    } else {
        _sc_console_draw_invalid_system_call(edx);
    }

    return NULL;
}
