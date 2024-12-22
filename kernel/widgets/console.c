#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "input_cursor.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"
#include "win_sheet.h"

#include "widgets/common_widget.h"
#include "widgets/console.h"

#define INPUT_CURSOR_Y_OFFSET (TITLE_BAR_HEIGHT + FONT_HEIGHT / 2)

void console_moving(void *p) {
    window_t *win = ((console_t *)p)->m_win;

    unsigned int vx = win->m_sheet->m_vx0, vy = win->m_sheet->m_vy0;
    int dx = g_mdec.m_rel_x, dy = g_mdec.m_rel_y;

    vx = (unsigned int)bound((int)vx + dx, 0,
                             (int)g_boot_info.m_screen_x -
                                 (int)win->m_sheet->m_bxsize);
    vy = (unsigned int)bound((int)vy + dy, 0,
                             (int)g_boot_info.m_screen_y -
                                 (int)win->m_sheet->m_bysize);

    win_sheet_slide(win->m_sheet, vx, vy);
    input_cursor_move(vx + FONT_WIDTH, vy + INPUT_CURSOR_Y_OFFSET);
}

void console_focus(console_t *p) {
    window_t *win = p->m_win;

    int max_text_len =
        ((int)win->m_sheet->m_bxsize - FONT_WIDTH * 2 - INPUT_CURSOR_WIDTH) /
        FONT_WIDTH;
    unsigned int text_len =
        (unsigned int)min(max(0, max_text_len), (int)strlen(p->m_text));

    char *dst = NULL;
    unsigned int vx = win->m_sheet->m_vx0, vy = win->m_sheet->m_vy0;

    window_ctl_set_focus_window(win);
    win_sheet_show(g_input_cursor_sht, win->m_sheet->m_z);

    if (text_len > 0) {
        dst = memman_alloc_4k(text_len + 1);
        assert(dst != NULL, "console_focus alloc 4k error");

        strncpy_tail(dst, text_len + 1, p->m_text);
    }

    input_cursor_move(vx + text_len * FONT_WIDTH + FONT_WIDTH,
                      vy + INPUT_CURSOR_Y_OFFSET);

    if (text_len > 0) {
        make_textbox8(win->m_sheet, FONT_WIDTH, TITLE_BAR_HEIGHT + FONT_HEIGHT,
                      win->m_sheet->m_bxsize - FONT_WIDTH * 2, FONT_HEIGHT,
                      COLOR_WHITE);

        show_string(win->m_sheet, FONT_WIDTH, TITLE_BAR_HEIGHT + FONT_HEIGHT,
                    COLOR_WHITE, COLOR_BLACK, dst);
    }

    if (dst != NULL)
        memman_free_4k(dst, text_len + 1);

    // win_sheet_refresh(win->m_sheet, 0, 0, win->m_sheet->m_bxsize,
    //                   win->m_sheet->m_bysize);
}

void console_draw_text(console_t *p, const char *text) {
    strncpy(p->m_text, CONSOLE_TEXT_MAX_LEN, text);
    console_focus(p);
}

void console_push(console_t *p, char c) {
    if (strlen(p->m_text) < CONSOLE_TEXT_MAX_LEN - 1) {
        strpush(p->m_text, c);
        console_focus(p);
    }
}

void console_pop(console_t *p) {
    if (strlen(p->m_text) == 0)
        return;

    strpop(p->m_text);
    console_focus(p);
}

console_t *console_new(unsigned int x, unsigned int y, unsigned int width,
                       unsigned int height, const char *title) {
    console_t *p = (console_t *)memman_alloc_4k(sizeof(console_t));
    assert(p != NULL, "console_new alloc 4k error");

    p->m_win = window_new(x, y, width, height, WINDOW_ID_CONSOLE, title, p);
    p->m_text[0] = '\0';

    make_textbox8(p->m_win->m_sheet, FONT_WIDTH,
                  TITLE_BAR_HEIGHT + FONT_HEIGHT / 2,
                  p->m_win->m_sheet->m_bxsize - FONT_WIDTH * 2,
                  p->m_win->m_sheet->m_bysize - TITLE_BAR_HEIGHT - FONT_HEIGHT,
                  COLOR_BLACK);

    return p;
}

void console_free(const console_t *p) {
    window_free(p->m_win);
    memman_free_4k(p, sizeof(console_t));
}

void console_show(console_t *p, int z) {
    window_show(p->m_win, z);
    console_focus(p);
}

void console_hide(console_t *p) {
    if (window_ctl_is_focus_window(p->m_win)) {
        win_sheet_hide(g_input_cursor_sht);
    }
    window_hide(p->m_win);
}