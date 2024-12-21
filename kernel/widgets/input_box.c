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
    input_box_t *box = (input_box_t *)p;

    if (!win_sheet_is_moving_sheet(box->m_sheet))
        return;

    unsigned int vx = box->m_sheet->m_vx0, vy = box->m_sheet->m_vy0;
    int dx = g_mdec.m_rel_x, dy = g_mdec.m_rel_y;

    vx = (unsigned int)bound((int)vx + dx, 0,
                             (int)g_boot_info.m_screen_x -
                                 (int)box->m_sheet->m_bxsize);
    vy = (unsigned int)bound((int)vy + dy, 0,
                             (int)g_boot_info.m_screen_y -
                                 (int)box->m_sheet->m_bysize);

    win_sheet_slide(box->m_sheet, vx, vy);
    input_box_focus(box);
}

void input_box_focus(input_box_t *box) {
    int max_text_len =
        ((int)box->m_sheet->m_bxsize - FONT_WIDTH * 2 - INPUT_CURSOR_WIDTH) /
        FONT_WIDTH;
    unsigned int text_len =
        (unsigned int)min(max(0, max_text_len), (int)strlen(box->m_text));

    char *dst = NULL;
    unsigned int vx = box->m_sheet->m_vx0, vy = box->m_sheet->m_vy0;

    win_sheet_set_focus(box->m_sheet);
    win_sheet_show(g_input_cursor_sht, box->m_sheet->m_z);

    if (text_len > 0) {
        dst = memman_alloc_4k(text_len + 1);
        assert(dst != NULL, "input_box_focus alloc 4k error");

        strncpy_tail(dst, text_len + 1, box->m_text);
    }

    input_cursor_move(vx + text_len * FONT_WIDTH + FONT_WIDTH,
                      vy + TITLE_BAR_HEIGHT + FONT_HEIGHT / 2);

    if (text_len > 0) {
        // 重新绘制背景
        make_textbox8(box->m_sheet, FONT_WIDTH,
                      TITLE_BAR_HEIGHT + FONT_HEIGHT / 2,
                      box->m_sheet->m_bxsize - FONT_WIDTH * 2, FONT_HEIGHT,
                      COLOR_WHITE);

        show_string(box->m_sheet, FONT_WIDTH,
                    TITLE_BAR_HEIGHT + FONT_HEIGHT / 2, COLOR_WHITE,
                    COLOR_BLACK, dst);
    }

    if (dst != NULL)
        memman_free_4k(dst, text_len + 1);
}

void input_box_draw_text(input_box_t *box, const char *text) {
    strncpy(box->m_text, INPUT_BOX_TEXT_MAX_LEN, text);
    input_box_focus(box);
}

void input_box_push(input_box_t *box, char c) {
    if (strlen(box->m_text) < INPUT_BOX_TEXT_MAX_LEN - 1) {
        strpush(box->m_text, c);
        input_box_focus(box);
    }
}

void input_box_pop(input_box_t *box) {
    if (strlen(box->m_text) == 0)
        return;

    strpop(box->m_text);
    input_box_focus(box);
}

input_box_t *input_box_new(unsigned int x, unsigned int y, unsigned int width,
                           unsigned int height, const char *title) {
    input_box_t *box = (input_box_t *)memman_alloc_4k(sizeof(input_box_t));
    assert(box != NULL, "input_box_new alloc 4k error");

    box->m_sheet = message_box_new(x, y, width, height, title);
    box->m_text[0] = '\0';

    // 设置移动回调函数
    sheet_userdata_type_moving_t *userdata = sheet_userdata_type_moving_alloc();
    sheet_userdata_type_moving_set(userdata, input_box_moving, box,
                                   "input_box_moving");

    sheet_userdata_set(&box->m_sheet->m_userdata, SHEET_USERDATA_TYPE_MOVING,
                       userdata);

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
        box->m_sheet, FONT_WIDTH, TITLE_BAR_HEIGHT + FONT_HEIGHT / 2,
        box->m_sheet->m_bxsize - FONT_WIDTH * 2, FONT_HEIGHT, COLOR_WHITE);

    return box;
}

void input_box_free(const input_box_t *p) {
    message_box_free(p->m_sheet);
    sheet_userdata_type_moving_free(p->m_sheet->m_userdata.m_data);
    memman_free_4k(p, sizeof(input_box_t));
}

void input_box_show(input_box_t *p, int z) {
    win_sheet_show(p->m_sheet, z);
    input_box_focus(p);
}

void input_box_hide(input_box_t *p) {
    win_sheet_hide(p->m_sheet);
    win_sheet_hide(g_input_cursor_sht);
}
