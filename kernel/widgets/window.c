#include "closebtn_icon.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"

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

    win->m_id = id;
    win->m_title = title;
    win->m_sheet = sht;
    win->m_instance = instance;

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
    draw_title_bar(p->m_sheet, p->m_title, COLOR_TITLE_BAR_FOCUS);
    win_sheet_refresh(p->m_sheet, 0, 0, p->m_sheet->m_bxsize,
                      TITLE_BAR_HEIGHT + 1);
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

// TODO
void window_ctl_up_window_to_top(window_t *p) {
    if (g_window_ctl.m_top == 0 ||
        p == g_window_ctl.m_windows[g_window_ctl.m_top])
        return;

    int start_z = p->m_sheet->m_z;
    assert(start_z >= WINDOW_WIN_SHEET_MIN_Z &&
               start_z <= WINDOW_WIN_SHEET_MAX_Z,
           "window_ctl_up_window_to_top invaild start_z");

    // 窗口和图层向下移动一个位置
    for (unsigned int i = (unsigned int)start_z - WINDOW_WIN_SHEET_MIN_Z + 1;
         i < g_window_ctl.m_top - 1; i++) {
        g_window_ctl.m_windows[i] = g_window_ctl.m_windows[i + 1];
        win_sheet_t *sht = g_window_ctl.m_windows[i]->m_sheet;
        win_sheet_show(sht, sht->m_z - 1);
    }

    g_window_ctl.m_windows[g_window_ctl.m_top - 1] = p;
    win_sheet_show(p->m_sheet,
                   (int)(WINDOW_WIN_SHEET_MIN_Z + g_window_ctl.m_top - 1));
}
