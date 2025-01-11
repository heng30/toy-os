#include "closebtn_icon.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"
#include "win_sheet.h"

#include "widgets/common_widget.h"
#include "widgets/console.h"
#include "widgets/input_box.h"
#include "widgets/window.h"

window_ctl_t g_window_ctl = {
    .m_focus_window = NULL,
    .m_moving_window = NULL,
    .m_top = 0,
    .m_windows = {NULL},
};

window_t *window_new(unsigned int x, unsigned int y, unsigned int width,
                     unsigned int height, unsigned char id, const char *title,
                     void *instance) {
    window_t *win = (window_t *)memman_alloc_4k(sizeof(window_t));
    assert(win != NULL, "window_new alloc error");

    win_sheet_t *sht = win_sheet_alloc();
    assert(sht != NULL, "window_new sheet alloc error");

    unsigned char *buf = (unsigned char *)memman_alloc_4k(width * height);
    assert(buf != NULL, "window_new alloc 4k buf error");

    win_sheet_setbuf(sht, WIN_SHEET_ID_WINDOW, buf, width, height,
                     COLOR_INVISIBLE);
    draw_window_background(sht);
    draw_title_bar(sht, title, COLOR_TITLE_BAR_FOCUS);
    win_sheet_slide(sht, x, y);

    win->m_enabled = true;
    win->m_have_input_cursor = false;
    win->m_is_waiting_for_close = false;
    win->m_id = id;
    win->m_title = title;
    win->m_sheet = sht;
    win->m_instance = instance;
    win->m_task = NULL;

    return win;
}

void window_free(window_t *p) {
    memman_free_4k(p->m_sheet->m_buf,
                   p->m_sheet->m_bxsize * p->m_sheet->m_bysize);
    win_sheet_free(p->m_sheet);
    memman_free_4k(p, sizeof(window_t));
}

void window_show(window_t *p, int z) { win_sheet_show(p->m_sheet, z); }

void window_hide(window_t *p) { win_sheet_hide(p->m_sheet); }

void window_moving(window_t *p) {
    unsigned int vx = p->m_sheet->m_vx0, vy = p->m_sheet->m_vy0;
    int dx = g_mdec.m_rel_x, dy = g_mdec.m_rel_y;

    vx = (unsigned int)bound((int)vx + dx, 0,
                             (int)g_boot_info.m_screen_x -
                                 (int)p->m_sheet->m_bxsize);
    vy = (unsigned int)bound((int)vy + dy, 0,
                             (int)g_boot_info.m_screen_y -
                                 (int)p->m_sheet->m_bysize);

    win_sheet_slide(p->m_sheet, vx, vy);
}

void window_focus(window_t *p) {
    if (!p->m_enabled)
        return;

    window_ctl_set_focus_window(p);
    win_sheet_show(p->m_sheet, p->m_sheet->m_z);
}

void window_ctl_add(window_t *p) {
    for (unsigned int i = 0; i < g_window_ctl.m_top; i++) {
        if (p == g_window_ctl.m_windows[i])
            return;
    }

    assert(g_window_ctl.m_top < MAX_SHEETS, "window_ctl_add too many windows");

    // 调整窗口图层的高度，保证每个图层高度只有一个图层
    int z = WINDOW_WIN_SHEET_MIN_Z + (int)g_window_ctl.m_top;
    assert(z <= WINDOW_WIN_SHEET_MAX_Z,
           "window_ctl_add overflow the WINDOW_WIN_SHEET_MAX_Z");

    g_window_ctl.m_windows[g_window_ctl.m_top] = p;
    g_window_ctl.m_top++;

    if (p->m_instance) {
        if (p->m_id == WINDOW_ID_INPUT_BOX) {
            input_box_show(p->m_instance, z);
        } else if (p->m_id == WINDOW_ID_CONSOLE) {
            console_show(p->m_instance, z);
        }
    } else {
        window_show(p, z);
    }
}

void window_ctl_remove(window_t *p) {
    for (unsigned int i = 0; i < g_window_ctl.m_top; i++) {
        if (p == g_window_ctl.m_windows[i]) {
            // 后面的成员向前移动一位
            for (unsigned int j = i; j < g_window_ctl.m_top - 1; j++) {
                g_window_ctl.m_windows[j] = g_window_ctl.m_windows[j + 1];
            }

            g_window_ctl.m_top--;
            return;
        }
    }
}

void window_ctl_set_focus_window(window_t *p) {
    window_t *old = g_window_ctl.m_focus_window;
    if (old == p)
        return;

    if (old) {
        draw_title_bar(old->m_sheet, old->m_title, COLOR_TITLE_BAR_UNFOCUS);
        win_sheet_refresh(old->m_sheet, 0, 0, old->m_sheet->m_bxsize,
                          TITLE_BAR_HEIGHT + 1);
    }

    g_window_ctl.m_focus_window = p;

    if (p) {
        draw_title_bar(p->m_sheet, p->m_title, COLOR_TITLE_BAR_FOCUS);
        win_sheet_refresh(p->m_sheet, 0, 0, p->m_sheet->m_bxsize,
                          TITLE_BAR_HEIGHT + 1);
    }
}

bool window_ctl_is_focus_window(window_t *p) {
    return g_window_ctl.m_focus_window == p;
}

void window_ctl_set_moving_window(window_t *p) {
    g_window_ctl.m_moving_window = p;
}

bool window_ctl_is_moving_window(window_t *p) {
    return g_window_ctl.m_moving_window == p;
}

window_t *window_ctl_get_moving_window(void) {
    return g_window_ctl.m_moving_window;
}

window_t *window_ctl_get_mouse_click_window(void) {
    unsigned int mouse_x = g_mdec.m_abs_x, mouse_y = g_mdec.m_abs_y;
    g_window_ctl.m_mouse_click_flag = WINDOW_CTL_MOUSE_CLICK_FLAG_NONE;

    for (int i = (int)g_window_ctl.m_top - 1; i >= 0; i--) {
        win_sheet_t *sht = g_window_ctl.m_windows[i]->m_sheet;
        unsigned int x0 = sht->m_vx0, y0 = sht->m_vy0;
        unsigned int x1 = x0 + sht->m_bxsize, y1 = y0 + sht->m_bysize;

        // 不再图层内
        if (!(mouse_x > x0 && mouse_x < x1 && mouse_y > y0 && mouse_y < y1)) {
            continue;
        }

        g_window_ctl.m_mouse_click_flag |= WINDOW_CTL_MOUSE_CLICK_FLAG_WINDOW;

        // 关闭按钮
        unsigned int closebtn_x0 = x1 - 21, closebtn_y0 = y0;
        unsigned int closebtn_x1 = x1 - 21 + CLOSEBTN_ICON_WIDTH,
                     closebtn_y1 = y0 + TITLE_BAR_HEIGHT;
        if (mouse_x > closebtn_x0 && mouse_x < closebtn_x1 &&
            mouse_y > closebtn_y0 && mouse_y < closebtn_y1) {
            g_window_ctl.m_mouse_click_flag |=
                WINDOW_CTL_MOUSE_CLICK_FLAG_CLOSEBTN;
            return g_window_ctl.m_windows[i];
        }

        // 标题栏
        unsigned int title_y1 = y0 + TITLE_BAR_HEIGHT;
        if (mouse_x > x0 && mouse_x < x1 && mouse_y > y0 &&
            mouse_y < title_y1) {
            g_window_ctl.m_mouse_click_flag |=
                WINDOW_CTL_MOUSE_CLICK_FLAG_TITLE;
            return g_window_ctl.m_windows[i];
        }

        return g_window_ctl.m_windows[i];
    }

    return NULL;
}

bool window_ctl_is_click_closebtn(void) {
    return g_window_ctl.m_mouse_click_flag &
           WINDOW_CTL_MOUSE_CLICK_FLAG_CLOSEBTN;
}

bool window_ctl_is_click_title(void) {
    return g_window_ctl.m_mouse_click_flag & WINDOW_CTL_MOUSE_CLICK_FLAG_TITLE;
}

bool window_ctl_is_click_window(void) {
    return g_window_ctl.m_mouse_click_flag & WINDOW_CTL_MOUSE_CLICK_FLAG_WINDOW;
}

void window_ctl_move_window_to_top(window_t *p) {
    if (g_window_ctl.m_top == 0 ||
        p == g_window_ctl.m_windows[g_window_ctl.m_top - 1])
        return;

    int start_z = p->m_sheet->m_z;
    assert(start_z >= WINDOW_WIN_SHEET_MIN_Z &&
               start_z <= WINDOW_WIN_SHEET_MAX_Z,
           "window_ctl_move_window_to_top invaild start_z");

    // 窗口和图层向下移动一个位置
    for (unsigned int i = (unsigned int)start_z - WINDOW_WIN_SHEET_MIN_Z;
         i < g_window_ctl.m_top - 1; i++) {
        g_window_ctl.m_windows[i] = g_window_ctl.m_windows[i + 1];
        win_sheet_t *sht = g_window_ctl.m_windows[i]->m_sheet;
        win_sheet_show(sht, sht->m_z - 1);
    }

    g_window_ctl.m_windows[g_window_ctl.m_top - 1] = p;
    win_sheet_show(p->m_sheet,
                   (int)(WINDOW_WIN_SHEET_MIN_Z + g_window_ctl.m_top - 1));
}

void window_ctl_focus_next_window(void) {
    // 只有一个窗口
    if (g_window_ctl.m_top == 1)
        return;

    unsigned int i = 0;

    // 查找当前获得焦点的窗口
    for (; i < g_window_ctl.m_top; i++) {
        window_t *win = g_window_ctl.m_windows[i];
        if (win == g_window_ctl.m_focus_window)
            break;
    }

    // 从当前窗口向后查找下一个窗口
    for (unsigned int p = i + 1; p < g_window_ctl.m_top; p++) {
        window_t *win = g_window_ctl.m_windows[p];
        if (!win_sheet_is_visible(win->m_sheet) || win->m_task->m_ref == 0 ||
            !win->m_enabled)
            continue;

        mouse_focus_window(win);
        return;
    }

    // 从头开始查找下一个窗口
    for (unsigned int q = 0; q < i; q++) {
        window_t *win = g_window_ctl.m_windows[q];
        if (!win_sheet_is_visible(win->m_sheet) || win->m_task->m_ref == 0 ||
            !win->m_enabled)
            continue;

        mouse_focus_window(win);
        return;
    }
}

bool window_ctl_is_window_exist(window_t *p) {
    for (unsigned int i = 0; i < g_window_ctl.m_top; i++) {
        if (p == g_window_ctl.m_windows[i]) {
            return true;
        }
    }

    return false;
}

window_t *window_ctl_find_window_by_id(unsigned char id) {
    for (unsigned int i = 0; i < g_window_ctl.m_top; i++) {
        window_t *w = g_window_ctl.m_windows[i];
        if (w->m_id == id) {
            return w;
        }
    }

    return NULL;
}

// 关闭窗口
void window_ctl_close_window_by_id(unsigned char id) {
    for (unsigned int i = 0; i < g_window_ctl.m_top; i++) {
        window_t *w = g_window_ctl.m_windows[i];
        if (w->m_id == id) {
            window_ctl_remove(w);
            window_free(w);
        }
    }
}
