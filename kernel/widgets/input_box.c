#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "input_cursor.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"
#include "win_sheet.h"

#include "widgets/input_box.h"

static void _make_textbox8(win_sheet_t *sht, int x0, int y0, int sx, int sy,
                           int c) {
    int x1 = x0 + sx, y1 = y0 + sy;
    boxfill8(sht->m_buf, sht->m_bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1,
             y0 - 3);
    boxfill8(sht->m_buf, sht->m_bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3,
             y1 + 1);
    boxfill8(sht->m_buf, sht->m_bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1,
             y1 + 2);
    boxfill8(sht->m_buf, sht->m_bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2,
             y1 + 2);
    boxfill8(sht->m_buf, sht->m_bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0,
             y0 - 2);
    boxfill8(sht->m_buf, sht->m_bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2,
             y1 + 0);
    boxfill8(sht->m_buf, sht->m_bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0,
             y1 + 1);
    boxfill8(sht->m_buf, sht->m_bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1,
             y1 + 1);
    boxfill8(sht->m_buf, sht->m_bxsize, c, x0 - 1, y0 - 1, x1 + 0, y1 + 0);
}

void input_box_moving(void *p) {
    input_box_t *box = (input_box_t *)p;

    if (!win_sheet_is_moving(box->m_sheet))
        return;

    unsigned int vx = box->m_sheet->m_vx0, vy = box->m_sheet->m_vy0;
    int dx = g_mdec.m_rel_x, dy = g_mdec.m_rel_y;

    vx = (unsigned int)bound(
        vx + dx, 0, (int)g_boot_info.m_screen_x - box->m_sheet->m_bxsize);
    vy = (unsigned int)bound(
        vy + dy, 0, (int)g_boot_info.m_screen_y - box->m_sheet->m_bysize);

    win_sheet_slide(box->m_sheet, vx, vy);
    input_box_focus(box);
}

void input_box_focus(input_box_t *box) {
    int max_text_len =
        (box->m_sheet->m_bxsize - FONT_WIDTH * 2 - INPUT_CURSOR_WIDTH) /
        FONT_WIDTH;
    int text_len = min(max_text_len, strlen(box->m_text));

    char *dst = NULL;
    int vx = box->m_sheet->m_vx0, vy = box->m_sheet->m_vy0;

    win_sheet_set_focus(box->m_sheet);

    if (text_len > 0) {
        dst = memman_alloc_4k(text_len + 1);
        assert(dst != NULL, "input_box_focus alloc 4k error");

        strncpy_tail(dst, text_len + 1, box->m_text);
    }

    input_cursor_move(vx + text_len * FONT_WIDTH + FONT_WIDTH,
                      vy + MESSAGE_BOX_TITLE_HEIGHT + FONT_HEIGHT);

    if (text_len > 0) {
        // 重新绘制背景
        _make_textbox8(
            box->m_sheet, FONT_WIDTH, MESSAGE_BOX_TITLE_HEIGHT + FONT_HEIGHT,
            box->m_sheet->m_bxsize - FONT_WIDTH * 2, FONT_HEIGHT, COLOR_WHITE);

        show_string(box->m_sheet, FONT_WIDTH,
                    MESSAGE_BOX_TITLE_HEIGHT + FONT_HEIGHT, COLOR_WHITE,
                    COLOR_BLACK, dst);
    }

    if (dst != NULL)
        memman_free_4k(dst, text_len + 1);
}

void input_box_draw_text(input_box_t *box, const char *text) {
    strncpy(box->m_text, INPUT_BOX_TEXT_MAX_LEN + 1, text);
    input_box_focus(box);
}

void input_box_push(input_box_t *box, char c) {
    if (strlen(box->m_text) < INPUT_BOX_TEXT_MAX_LEN) {
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

input_box_t *input_box_new(int x, int y, int width, int height,
                           const char *title) {
    input_box_t *box = (input_box_t *)memman_alloc_4k(sizeof(input_box_t));
    assert(box != NULL, "input_box_new alloc 4k error");

    box->m_sheet = message_box_new(x, y, width, height, title);
    box->m_text[0] = '\0';

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

    // 绘制光标
    _make_textbox8(
        box->m_sheet, FONT_WIDTH, MESSAGE_BOX_TITLE_HEIGHT + FONT_HEIGHT,
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
    win_sheet_show(g_input_cursor_sht, z);
    input_box_focus(p);
}

void input_box_hide(input_box_t *p) {
    win_sheet_hide(p->m_sheet);
    win_sheet_hide(g_input_cursor_sht);
}
