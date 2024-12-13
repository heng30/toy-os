#include "widgets/input_box.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"
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

void input_box_focus(input_box_t *box) {
    int max_text_len =
        (box->m_sheet->m_bxsize - FONT_WIDTH * 2 - INPUT_BLOCK_WIDTH) /
        FONT_WIDTH;
    int text_len = min(max_text_len, strlen(box->m_text));

    char *dst = NULL;
    int vx = box->m_sheet->m_vx0, vy = box->m_sheet->m_vy0;

    if (text_len > 0) {
        dst = memman_alloc_4k(text_len + 1);
        assert(dst != NULL, "input_box_focus alloc 4k error");

        strncpy(dst, text_len + 1, box->m_text);
    }

    input_block_move(vx + text_len * FONT_WIDTH + FONT_WIDTH,
                     vy + MESSAGE_BOX_TITLE_HEIGHT + FONT_HEIGHT);

    if (text_len > 0) {

        // 重新绘制背景
        make_textbox8(
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

    make_textbox8(
        box->m_sheet, FONT_WIDTH, MESSAGE_BOX_TITLE_HEIGHT + FONT_HEIGHT,
        box->m_sheet->m_bxsize - FONT_WIDTH * 2, FONT_HEIGHT, COLOR_WHITE);

    return box;
}

void input_box_free(const input_box_t *p) {
    message_box_free(p->m_sheet);
    memman_free_4k(p, sizeof(input_box_t));
}
