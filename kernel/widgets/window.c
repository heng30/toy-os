#include "closebtn_icon.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"

#include "widgets/common_widget.h"
#include "widgets/window.h"

window_t *window_new(unsigned int x, unsigned int y, unsigned int width,
                     unsigned int height, const char *title) {
    window_t *win = (window_t *)memman_alloc_4k(sizeof(window_t));
    assert(win != NULL, "window_new alloc error");

    win_sheet_t *sht = win_sheet_alloc();
    assert(sht != NULL, "window_new sheet alloc error");

    unsigned char *buf = (unsigned char *)memman_alloc_4k(width * height);
    assert(buf != NULL, "window_new alloc 4k buf error");

    win_sheet_setbuf(sht, "window", buf, width, height, COLOR_INVISIBLE);
    draw_title_bar(sht, title, COLOR_TITLE_BAR_FOCUS);
    win_sheet_slide(sht, x, y);

    win->m_title = title;
    win->m_sheet = sht;

    return win;
}

void window_free(window_t *p) {
    memman_free_4k(p->m_sheet->m_buf,
                   p->m_sheet->m_bxsize * p->m_sheet->m_bysize);
    win_sheet_free(p->m_sheet);
    memman_free_4k(p, sizeof(window_t));
}

void window_show(window_t *p, int z) {
    win_sheet_show(p->m_sheet, z);
}

void window_hide(window_t *p) {
    win_sheet_hide(p->m_sheet);
}
