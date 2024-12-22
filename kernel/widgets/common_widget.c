#include "closebtn_icon.h"
#include "colo8.h"
#include "draw.h"

#include "widgets/common_widget.h"
#include "widgets/window.h"

void make_textbox8(win_sheet_t *sht, unsigned int x0, unsigned int y0,
                   unsigned int sx, unsigned int sy, unsigned char c) {
    unsigned int x1 = x0 + sx, y1 = y0 + sy;
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

static void _draw_title_bar_closebtn(win_sheet_t *sht) {
    const unsigned char *icon =
        closebtn_icon_buf(COL8_000000, COL8_FFFFFF, COL8_848484, COL8_C6C6C6);

    for (unsigned int y = 0; y < CLOSEBTN_ICON_HEIGHT; y++) {
        for (unsigned int x = 0; x < CLOSEBTN_ICON_WIDTH; x++) {
            unsigned char c = *(icon + y * CLOSEBTN_ICON_WIDTH + x);
            sht->m_buf[(5 + y) * sht->m_bxsize + (sht->m_bxsize - 21 + x)] = c;
        }
    }
}

static void _draw_title_bar_title(win_sheet_t *sht, const char *title, unsigned char background_color) {
    unsigned int bxsize = sht->m_bxsize, bysize = sht->m_bysize;
    boxfill8(sht->m_buf, bxsize, background_color, 3, 3, bxsize - 4,
             TITLE_BAR_HEIGHT);
    show_string(sht, 8, 4, background_color, COL8_FFFFFF, title);
}

void draw_title_bar(win_sheet_t *sht, const char *title, unsigned char background_color) {
    _draw_title_bar_title(sht, title, background_color);
    _draw_title_bar_closebtn(sht);
}

void draw_window_background(win_sheet_t *sht) {
    unsigned int bxsize = sht->m_bxsize, bysize = sht->m_bysize;

    boxfill8(sht->m_buf, bxsize, COL8_C6C6C6, 0, 0, bxsize - 1, 0);
    boxfill8(sht->m_buf, bxsize, COL8_FFFFFF, 1, 1, bxsize - 2, 1);
    boxfill8(sht->m_buf, bxsize, COL8_C6C6C6, 0, 0, 0, bysize - 1);
    boxfill8(sht->m_buf, bxsize, COL8_FFFFFF, 1, 1, 1, bysize - 1);
    boxfill8(sht->m_buf, bxsize, COL8_848484, bxsize - 2, 1, bxsize - 2,
             bysize - 2);
    boxfill8(sht->m_buf, bxsize, COL8_000000, bxsize - 1, 0, bxsize - 1,
             bysize - 1);
    boxfill8(sht->m_buf, bxsize, COL8_C6C6C6, 2, 2, bxsize - 3, bysize - 3);
    // boxfill8(sht->m_buf, bxsize, background_color, 3, 3, bxsize - 4,
    //          TITLE_BAR_HEIGHT);
    boxfill8(sht->m_buf, bxsize, COL8_848484, 1, bysize - 2, bxsize - 2,
             bysize - 2);
    boxfill8(sht->m_buf, bxsize, COL8_000000, 0, bysize - 1, bxsize - 1,
             bysize - 1);
}
