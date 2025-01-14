#include "system_call.h"
#include "colo8.h"
#include "draw.h"
#include "file_ctl.h"
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

static void _sc_console_draw_text(unsigned int ebx, bool is_literal) {
    console_t *p = console_get();
    if (!p->m_cmd->m_data)
        return;

    const char *text = NULL;
    if (is_literal)
        text = (const char *)(p->m_cmd->m_data + ebx);
    else
        text = (const char *)(p->m_cmd_ds + ebx);

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
                                    unsigned int text, bool is_literal) {
    console_t *p = console_get();
    window_t *pwin = (window_t *)win;

    if (window_ctl_is_window_exist(pwin)) {
        unsigned char bg_col = (unsigned char)(col >> 8);
        unsigned char fg_col = (unsigned char)(col & 0xff);
        const char *real_text = NULL;

        if (is_literal) {
            real_text = p->m_cmd->m_data + text; // 从代码段中开始偏移
        } else {
            real_text = p->m_cmd_ds + text; // 从数据段中开始偏移
        }

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

static void _sc_dump_window_sheet(ptr_t *reg, unsigned int win, unsigned int x0,
                                  unsigned int y0, unsigned int x1,
                                  unsigned int y1, unsigned int offset) {
    console_t *p = console_get();
    window_t *pwin = (window_t *)win;

    if (window_ctl_is_window_exist(pwin)) {
        win_sheet_t *sht = pwin->m_sheet;

        x1 = min_unsigned(x1, sht->m_bxsize - 1);
        y1 = min_unsigned(y1, sht->m_bysize - 1);

        if (x0 > x1 || y0 > y1) {
            reg[7] = false;
            return;
        }

        unsigned char *buf = p->m_cmd_ds + offset;

        // 复制图层数据到用户缓冲区
        for (unsigned int y_src = y0, y_dst = 0; y_src <= y1;
             y_src++, y_dst++) {
            for (unsigned int x_src = x0, x_dst = 0; x_src <= x1;
                 x_src++, x_dst++) {
                unsigned int pos_src = y_src * sht->m_bxsize + x_src;
                unsigned int pos_dst = y_dst * (x1 - x0 + 1) + x_dst;
                buf[pos_dst] = sht->m_buf[pos_src];
            }
        }
    } else {
        _sc_no_window_errmsg(p, win);
    }
}

static void _sc_cover_window_sheet(unsigned int win, unsigned int x0,
                                   unsigned int y0, unsigned int x1,
                                   unsigned int y1, unsigned int offset) {
    console_t *p = console_get();
    window_t *pwin = (window_t *)win;

    if (window_ctl_is_window_exist(pwin)) {
        win_sheet_t *sht = pwin->m_sheet;

        x1 = min_unsigned(x1, sht->m_bxsize - 1);
        y1 = min_unsigned(y1, sht->m_bysize - 1);

        if (x0 > x1 || y0 > y1) {
            return;
        }

        unsigned char *buf = p->m_cmd_ds + offset;

        // 复制用户缓冲区数据到图层
        for (unsigned int y_src = 0, y_dst = y0; y_dst <= y1;
             y_src++, y_dst++) {
            for (unsigned int x_src = 0, x_dst = x0; x_dst <= x1;
                 x_src++, x_dst++) {
                unsigned int pos_src = y_src * (x1 - x0 + 1) + x_src;
                unsigned int pos_dst = y_dst * sht->m_bxsize + x_dst;
                sht->m_buf[pos_dst] = buf[pos_src];
            }
        }
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

static void _sc_timer_is_timeout(ptr_t *reg, unsigned int timer) {
    timer_t *p = (timer_t *)timer;
    if (timer_is_valid(p)) {
        reg[7] = (ptr_t)timer_is_timeout(p);
    }
}

// NOTE: 语句太长可能会出异常
static bool _sc_handle_window_1(ptr_t *reg, unsigned int edi, unsigned int esi,
                                unsigned int ebp, unsigned int esp,
                                unsigned int ebx, unsigned int edx,
                                unsigned int ecx, unsigned int eax) {
    bool is_handle = true;
    if (edx == SYSTEM_CALL_NEW_WINDOW) {
        _sc_new_window(reg, ebx, esi, edi, eax, ecx);
    } else if (edx == SYSTEM_CALL_CLOSE_WINDOW) {
        _sc_close_window(ebx);
    } else if (edx == SYSTEM_CALL_REFRESH_WINDOW) {
        _sc_refresh_window(ebx, eax, ecx, esi, edi);
    } else if (edx == SYSTEM_CALL_IS_CLOSE_WINDOW) {
        _sc_is_close_window(reg);
    } else if (edx == SYSTEM_CALL_DRAW_TEXT_IN_WINDOW) {
        _sc_draw_text_in_window(ebx, esi, edi, (unsigned short)eax, ecx,
                                (bool)ebp);
    } else {
        is_handle = false;
    }
    return is_handle;
}

static bool _sc_handle_window_2(ptr_t *reg, unsigned int edi, unsigned int esi,
                                unsigned int ebp, unsigned int esp,
                                unsigned int ebx, unsigned int edx,
                                unsigned int ecx, unsigned int eax) {
    bool is_handle = true;
    if (edx == SYSTEM_CALL_DRAW_BOX_IN_WINDOW) {
        _sc_draw_box_in_window(ebx, eax, ecx, esi, edi, (unsigned char)ebp);
    } else if (edx == SYSTEM_CALL_DRAW_POINT_IN_WINDOW) {
        _sc_draw_point_in_window(ebx, esi, edi, (unsigned char)eax);
    } else if (edx == SYSTEM_CALL_DRAW_LINE_IN_WINDOW) {
        _sc_draw_line_in_window(ebx, eax, ecx, esi, edi, (unsigned char)ebp);
    } else if (edx == SYSTEM_CALL_DUMP_WINDOW_SHEET) {
        _sc_dump_window_sheet(reg, ebx, eax, ecx, esi, edi, ebp);
    } else if (edx == SYSTEM_CALL_COVER_WINDOW_SHEET) {
        _sc_cover_window_sheet(ebx, eax, ecx, esi, edi, ebp);
    } else {
        is_handle = false;
    }
    return is_handle;
}

static bool _sc_handle_window(ptr_t *reg, unsigned int edi, unsigned int esi,
                              unsigned int ebp, unsigned int esp,
                              unsigned int ebx, unsigned int edx,
                              unsigned int ecx, unsigned int eax) {
    if (_sc_handle_window_1(reg, edi, esi, ebp, esp, ebx, edx, ecx, eax)) {
        return true;
    } else if (_sc_handle_window_2(reg, edi, esi, ebp, esp, ebx, edx, ecx,
                                   eax)) {
        return true;
    }

    return false;
}

static bool _sc_handle_console(ptr_t *reg, unsigned int edi, unsigned int esi,
                               unsigned int ebp, unsigned int esp,
                               unsigned int ebx, unsigned int edx,
                               unsigned int ecx, unsigned int eax) {
    bool is_handle = true;
    if (edx == SYSTEM_CALL_CONSOLE_DRAW_CH) {
        _sc_console_draw_ch(eax);
    } else if (edx == SYSTEM_CALL_CONSOLE_DRAW_TEXT) {
        _sc_console_draw_text(ebx, (bool)eax);
    } else {
        is_handle = false;
    }
    return is_handle;
}

static bool _sc_handle_timer(ptr_t *reg, unsigned int edi, unsigned int esi,
                             unsigned int ebp, unsigned int esp,
                             unsigned int ebx, unsigned int edx,
                             unsigned int ecx, unsigned int eax) {
    bool is_handle = true;
    if (edx == SYSTEM_CALL_TIMER_ALLOC) {
        _sc_timer_alloc(reg);
    } else if (edx == SYSTEM_CALL_TIMER_FREE) {
        _sc_timer_free(ebx);
    } else if (edx == SYSTEM_CALL_TIMER_SET) {
        _sc_timer_set(ebx, eax, ecx);
    } else if (edx == SYSTEM_CALL_TIMER_IS_TIMEOUT) {
        _sc_timer_is_timeout(reg, ebx);
    } else {
        is_handle = false;
    }
    return is_handle;
}

static void _sc_strlen_cs(ptr_t *reg, unsigned int offset) {
    console_t *p = console_get();
    unsigned char *s = p->m_cmd->m_data + offset;
    unsigned int len = strlen((const char *)s);
    reg[7] = len;
}

static void _sc_memcpy_cs2ds(ptr_t *reg, unsigned int ds_offset,
                             unsigned int cs_offset, unsigned int len) {
    console_t *p = console_get();
    unsigned char *ds_s = p->m_cmd_ds + ds_offset;
    unsigned char *cs_s = p->m_cmd->m_data + cs_offset;
    memcpy(ds_s, cs_s, len);
    reg[7] = ds_offset;
}

static bool _sc_handle_util(ptr_t *reg, unsigned int edi, unsigned int esi,
                            unsigned int ebp, unsigned int esp,
                            unsigned int ebx, unsigned int edx,
                            unsigned int ecx, unsigned int eax) {
    bool is_handle = true;
    if (edx == SYSTEM_CALL_RAND_UINT) {
        _sc_rand_uint(reg, ebx);
    } else if (edx == SYSTEM_CALL_SHOW_DEBUG_UINT) {
        _sc_show_debug_uint(ebx, eax, ecx);
    } else if (edx == SYSTEM_CALL_STRLEN_CS) {
        _sc_strlen_cs(reg, ebx);
    } else if (edx == SYSTEM_CALL_MEMCPY_CS2DS) {
        _sc_memcpy_cs2ds(reg, ebx, eax, ecx);
    } else {
        is_handle = false;
    }

    return is_handle;
}

void _sc_file_open(ptr_t *reg, const char *filename, bool is_literal) {
    console_t *p = console_get();

    if (is_literal)
        filename = p->m_cmd->m_data + (unsigned int)filename;
    else
        filename = p->m_cmd_ds + (unsigned int)filename;

    int fd = file_ctl_open(filename);
    if (fd >= 0 && fd < FILE_DESCRIPTOR_MAX) {
        p->m_file_des[fd] = true;
    }

    reg[7] = (ptr_t)fd;
}

void _sc_file_read(ptr_t *reg, int fd, unsigned int offset, unsigned int len,
                   unsigned int pos) {
    console_t *p = console_get();
    unsigned char *buf = p->m_cmd_ds + offset;
    int rlen = file_ctl_read(fd, buf, len, pos);
    reg[7] = (ptr_t)rlen;
}

void _sc_file_write(ptr_t *reg, int fd, unsigned int offset, unsigned int len,
                    unsigned int pos) {
    console_t *p = console_get();
    unsigned char *buf = p->m_cmd_ds + offset;
    int wlen = file_ctl_write(fd, buf, len, pos);
    reg[7] = (ptr_t)wlen;
}

void _sc_file_close(int fd) {
    if (fd >= 0 && fd < FILE_DESCRIPTOR_MAX) {
        console_t *p = console_get();
        p->m_file_des[fd] = false;
        file_ctl_close(fd);
    }
}

void _sc_file_size(ptr_t *reg, int fd) {
    int size = file_ctl_file_size(fd);
    reg[7] = (ptr_t)size;
}

static bool _sc_handle_file(ptr_t *reg, unsigned int edi, unsigned int esi,
                            unsigned int ebp, unsigned int esp,
                            unsigned int ebx, unsigned int edx,
                            unsigned int ecx, unsigned int eax) {
    bool is_handle = true;

    if (edx == SYSTEM_CALL_FILE_OPEN) {
        _sc_file_open(reg, (const char *)ebx, (bool)eax);
    } else if (edx == SYSTEM_CALL_FILE_READ) {
        _sc_file_read(reg, (int)ebx, eax, ecx, edi);
    } else if (edx == SYSTEM_CALL_FILE_WRITE) {
        _sc_file_write(reg, (int)ebx, eax, ecx, edi);
    } else if (edx == SYSTEM_CALL_FILE_CLOSE) {
        _sc_file_close((int)ebx);
    } else if (edx == SYSTEM_CALL_FILE_SIZE) {
        _sc_file_size(reg, (int)ebx);
    } else {
        is_handle = false;
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
    } else if (_sc_handle_util(reg, edi, esi, ebp, esp, ebx, edx, ecx, eax)) {
        return NULL;
    } else if (_sc_handle_window(reg, edi, esi, ebp, esp, ebx, edx, ecx, eax)) {
        return NULL;
    } else if (_sc_handle_console(reg, edi, esi, ebp, esp, ebx, edx, ecx,
                                  eax)) {
        return NULL;
    } else if (_sc_handle_timer(reg, edi, esi, ebp, esp, ebx, edx, ecx, eax)) {
        return NULL;
    } else if (_sc_handle_file(reg, edi, esi, ebp, esp, ebx, edx, ecx, eax)) {
        return NULL;
    } else {
        _sc_console_draw_invalid_system_call(edx);
    }

    return NULL;
}
