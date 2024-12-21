#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "input_cursor.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"
#include "win_sheet.h"

#include "widgets/console.h"
#include "widgets/common_widget.h"

void console_moving(void *p) {
    console_t *box = (console_t *)p;

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
    console_focus(box);
}

void console_focus(console_t *box) {
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
        assert(dst != NULL, "console_focus alloc 4k error");

        strncpy_tail(dst, text_len + 1, box->m_text);
    }

    input_cursor_move(vx + text_len * FONT_WIDTH + FONT_WIDTH,
                      vy + TITLE_BAR_HEIGHT + FONT_HEIGHT);

    if (text_len > 0) {
        make_textbox8(
            box->m_sheet, FONT_WIDTH, TITLE_BAR_HEIGHT + FONT_HEIGHT,
            box->m_sheet->m_bxsize - FONT_WIDTH * 2, FONT_HEIGHT, COLOR_WHITE);

        show_string(box->m_sheet, FONT_WIDTH,
                    TITLE_BAR_HEIGHT + FONT_HEIGHT, COLOR_WHITE,
                    COLOR_BLACK, dst);
    }

    if (dst != NULL)
        memman_free_4k(dst, text_len + 1);
}

void console_draw_text(console_t *box, const char *text) {
    strncpy(box->m_text, CONSOLE_TEXT_MAX_LEN, text);
    console_focus(box);
}

void console_push(console_t *box, char c) {
    if (strlen(box->m_text) < CONSOLE_TEXT_MAX_LEN - 1) {
        strpush(box->m_text, c);
        console_focus(box);
    }
}

void console_pop(console_t *box) {
    if (strlen(box->m_text) == 0)
        return;

    strpop(box->m_text);
    console_focus(box);
}

console_t *console_new(unsigned int x, unsigned int y, unsigned int width,
                       unsigned int height, const char *title) {
    console_t *box = (console_t *)memman_alloc_4k(sizeof(console_t));
    assert(box != NULL, "console_new alloc 4k error");

    box->m_sheet = message_box_new(x, y, width, height, title);
    box->m_text[0] = '\0';

    sheet_userdata_type_moving_t *userdata = sheet_userdata_type_moving_alloc();
    sheet_userdata_type_moving_set(userdata, console_moving, box,
                                   "console_moving");

    sheet_userdata_set(&box->m_sheet->m_userdata, SHEET_USERDATA_TYPE_MOVING,
                       userdata);

    make_textbox8(
        box->m_sheet, FONT_WIDTH, TITLE_BAR_HEIGHT + FONT_HEIGHT / 2,
        box->m_sheet->m_bxsize - FONT_WIDTH * 2,
        box->m_sheet->m_bysize - TITLE_BAR_HEIGHT - FONT_HEIGHT,
        COLOR_BLACK);

    return box;
}

void console_free(const console_t *p) {
    message_box_free(p->m_sheet);
    sheet_userdata_type_moving_free(p->m_sheet->m_userdata.m_data);
    memman_free_4k(p, sizeof(console_t));
}

void console_show(console_t *p, int z) {
    win_sheet_show(p->m_sheet, z);
    console_focus(p);
}

void console_hide(console_t *p) {
    win_sheet_hide(p->m_sheet);
    win_sheet_hide(g_input_cursor_sht);
}
