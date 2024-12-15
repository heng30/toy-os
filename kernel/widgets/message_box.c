#include "widgets/message_box.h"
#include "closebtn_icon.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"

static void _draw_background(win_sheet_t *sht) {
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
    boxfill8(sht->m_buf, bxsize, COL8_000084, 3, 3, bxsize - 4,
             MESSAGE_BOX_TITLE_HEIGHT);
    boxfill8(sht->m_buf, bxsize, COL8_848484, 1, bysize - 2, bxsize - 2,
             bysize - 2);
    boxfill8(sht->m_buf, bxsize, COL8_000000, 0, bysize - 1, bxsize - 1,
             bysize - 1);
}

static void _draw_closebtn(win_sheet_t *sht) {
    const unsigned char *icon =
        closebtn_icon_buf(COL8_000000, COL8_FFFFFF, COL8_848484, COL8_C6C6C6);

    for (unsigned int y = 0; y < CLOSEBTN_ICON_HEIGHT; y++) {
        for (unsigned int x = 0; x < CLOSEBTN_ICON_WIDTH; x++) {
            unsigned char c = *(icon + y * CLOSEBTN_ICON_WIDTH + x);
            sht->m_buf[(5 + y) * sht->m_bxsize + (sht->m_bxsize - 21 + x)] = c;
        }
    }
}

static void _draw_title(win_sheet_t *sht, const char *title) {
    show_string(sht, 8, 4, COL8_000084, COL8_FFFFFF, title);
}

win_sheet_t *message_box_new(unsigned int x, unsigned int y, unsigned int width,
                             unsigned int height, const char *title) {
    win_sheet_t *sht = win_sheet_alloc();
    assert(sht != NULL, "message_box_new sheet alloc error");

    unsigned char *buf = (unsigned char *)memman_alloc_4k(width * height);
    assert(buf != NULL, "message_box_new alloc 4k buf error");

    win_sheet_setbuf(sht, "message_box", buf, width, height, COLOR_INVISIBLE);

    _draw_background(sht);
    _draw_closebtn(sht);
    _draw_title(sht, title);

    win_sheet_slide(sht, x, y);

    return sht;
}

void message_box_free(win_sheet_t *p) {
    memman_free_4k(p->m_buf, p->m_bxsize * p->m_bysize);
    win_sheet_free(p);
}
