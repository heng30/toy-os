#include "closebtn_icon.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"

#include "widgets/common_widget.h"
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

    g_window_ctl.m_windows[g_window_ctl.m_top] = p;
    g_window_ctl.m_top++;
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
    g_window_ctl.m_focus_window = p;
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
