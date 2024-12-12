#include "input_box.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"
#include "win_sheet.h"

static void make_textbox8(win_sheet_t *sht, int x0, int y0, int sx, int sy,
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

input_box_t *input_box_new(int x, int y, int width, int height,
                           const char *title) {

    input_box_t *box = (input_box_t *)memman_alloc_4k(sizeof(input_box_t));
    assert(box != NULL, "input_box_new alloc 4k error");

    box->m_box = message_box_new(x, y, width, height, title);
    assert(box->m_box != NULL, "input_box_new alloc box error");

    make_textbox8(box->m_box->m_sheet, FONT_WIDTH,
                  MESSAGE_BOX_TITLE_HEIGHT + FONT_HEIGHT,
                  box->m_box->m_sheet->m_bxsize - FONT_WIDTH * 2, FONT_HEIGHT,
                  COLOR_WHITE);

    return box;
}

void input_box_free(const input_box_t *p) {
    message_box_free(p->m_box);
    memman_free_4k(p, sizeof(input_box_t));
}

void input_box_show(input_box_t *p, int sheet_z) {
    win_sheet_updown(p->m_box->m_sheet, sheet_z);
}

void input_box_hide(input_box_t *p) {
    win_sheet_updown(p->m_box->m_sheet, HIDE_WIN_SHEET_Z);
}

bool input_box_is_visible(input_box_t *p) {
    return p->m_box->m_sheet->m_z >= BOTTOM_WIN_SHEET_Z;
}

const char *input_box_get_name(input_box_t *p) {
    return message_box_get_name(p->m_box);
}

void input_box_set_name(input_box_t *p, const char *name) {
    message_box_set_name(p->m_box, name);
}
