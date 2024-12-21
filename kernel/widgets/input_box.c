#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "input_cursor.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"
#include "win_sheet.h"

#include "widgets/common_widget.h"
#include "widgets/input_box.h"

void input_box_moving(void *p) {
    window_t *win = ((input_box_t *)p)->m_win;

    if (!win_sheet_is_moving_sheet(win->m_sheet))
        return;

    unsigned int vx = win->m_sheet->m_vx0, vy = win->m_sheet->m_vy0;
    int dx = g_mdec.m_rel_x, dy = g_mdec.m_rel_y;

    vx = (unsigned int)bound((int)vx + dx, 0,
                             (int)g_boot_info.m_screen_x -
                                 (int)win->m_sheet->m_bxsize);
    vy = (unsigned int)bound((int)vy + dy, 0,
                             (int)g_boot_info.m_screen_y -
                                 (int)win->m_sheet->m_bysize);

    win_sheet_slide(win->m_sheet, vx, vy);
    input_box_focus((input_box_t *)p);
}

void input_box_focus(input_box_t *p) {
    window_t *win = ((input_box_t *)p)->m_win;

    int max_text_len =
        ((int)win->m_sheet->m_bxsize - FONT_WIDTH * 2 - INPUT_CURSOR_WIDTH) /
        FONT_WIDTH;
    unsigned int text_len =
        (unsigned int)min(max(0, max_text_len), (int)strlen(p->m_text));

    char *dst = NULL;
    unsigned int vx = win->m_sheet->m_vx0, vy = win->m_sheet->m_vy0;

    win_sheet_set_focus(win->m_sheet);
    win_sheet_show(g_input_cursor_sht, win->m_sheet->m_z);

    if (text_len > 0) {
        dst = memman_alloc_4k(text_len + 1);
        assert(dst != NULL, "input_box_focus alloc 4k error");

        strncpy_tail(dst, text_len + 1, p->m_text);
    }

    input_cursor_move(vx + text_len * FONT_WIDTH + FONT_WIDTH,
                      vy + TITLE_BAR_HEIGHT + FONT_HEIGHT / 2);

    if (text_len > 0) {
        // 重新绘制背景
        make_textbox8(
            win->m_sheet, FONT_WIDTH, TITLE_BAR_HEIGHT + FONT_HEIGHT / 2,
            win->m_sheet->m_bxsize - FONT_WIDTH * 2, FONT_HEIGHT, COLOR_WHITE);

        show_string(win->m_sheet, FONT_WIDTH,
                    TITLE_BAR_HEIGHT + FONT_HEIGHT / 2, COLOR_WHITE,
                    COLOR_BLACK, dst);
    }

    if (dst != NULL)
        memman_free_4k(dst, text_len + 1);
}

void input_box_draw_text(input_box_t *p, const char *text) {
    strncpy(p->m_text, INPUT_BOX_TEXT_MAX_LEN, text);
    input_box_focus(p);
}

void input_box_push(input_box_t *p, char c) {
    if (strlen(p->m_text) < INPUT_BOX_TEXT_MAX_LEN - 1) {
        strpush(p->m_text, c);
        input_box_focus(p);
    }
}

void input_box_pop(input_box_t *p) {
    if (strlen(p->m_text) == 0)
        return;

    strpop(p->m_text);
    input_box_focus(p);
}

input_box_t *input_box_new(unsigned int x, unsigned int y, unsigned int width,
                           unsigned int height, const char *title) {
    input_box_t *p = (input_box_t *)memman_alloc_4k(sizeof(input_box_t));
    assert(p != NULL, "input_box_new alloc 4k error");

    p->m_win = window_new(x, y, width, height, title);
    p->m_text[0] = '\0';

    // 设置移动回调函数
    sheet_userdata_type_moving_t *userdata = sheet_userdata_type_moving_alloc();
    sheet_userdata_type_moving_set(userdata, input_box_moving, p,
                                   "input_box_moving");

    sheet_userdata_set(&p->m_win->m_sheet->m_userdata,
                       SHEET_USERDATA_TYPE_MOVING, userdata);

#ifdef __DEBUG__
    show_string_in_canvas(0, FONT_HEIGHT * 2, COLOR_WHITE,
                          int2hexstr((int)userdata));
    show_string_in_canvas(FONT_WIDTH * 10, FONT_HEIGHT * 2, COLOR_WHITE,
                          int2hexstr((int)userdata->m_cb));
    show_string_in_canvas(FONT_WIDTH * 20, FONT_HEIGHT * 2, COLOR_WHITE,
                          int2hexstr((int)userdata->m_data));
    show_string_in_canvas(FONT_WIDTH * 30, FONT_HEIGHT * 2, COLOR_WHITE,
                          int2hexstr((int)input_box_moving));
#endif

    // 绘制输入框
    make_textbox8(
        p->m_win->m_sheet, FONT_WIDTH, TITLE_BAR_HEIGHT + FONT_HEIGHT / 2,
        p->m_win->m_sheet->m_bxsize - FONT_WIDTH * 2, FONT_HEIGHT, COLOR_WHITE);

    return p;
}

void input_box_free(const input_box_t *p) {
    window_free(p->m_win);
    sheet_userdata_type_moving_free(p->m_win->m_sheet->m_userdata.m_data);
    memman_free_4k(p, sizeof(input_box_t));
}

void input_box_show(input_box_t *p, int z) {
    window_show(p->m_win, z);
    input_box_focus(p);
}

void input_box_hide(input_box_t *p) {
    if (win_sheet_is_focus(p->m_win->m_sheet)) {
        win_sheet_hide(g_input_cursor_sht);
    }

    window_hide(p->m_win);
}
