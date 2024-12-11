#include "message_box.h"
#include "closebtn_icon.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"

static void _draw_background(const message_box_t *msg_box) {
    win_sheet_t *sht = msg_box->m_sheet;
    int bxsize = sht->m_bxsize;
    int bysize = sht->m_bysize;

    boxfill8(sht->m_buf, bxsize, COL8_C6C6C6, 0, 0, bxsize - 1, 0);
    boxfill8(sht->m_buf, bxsize, COL8_FFFFFF, 1, 1, bxsize - 2, 1);
    boxfill8(sht->m_buf, bxsize, COL8_C6C6C6, 0, 0, 0, bysize - 1);
    boxfill8(sht->m_buf, bxsize, COL8_FFFFFF, 1, 1, 1, bysize - 1);
    boxfill8(sht->m_buf, bxsize, COL8_848484, bxsize - 2, 1, bxsize - 2,
             bysize - 2);
    boxfill8(sht->m_buf, bxsize, COL8_000000, bxsize - 1, 0, bxsize - 1,
             bysize - 1);
    boxfill8(sht->m_buf, bxsize, COL8_C6C6C6, 2, 2, bxsize - 3, bysize - 3);
    boxfill8(sht->m_buf, bxsize, COL8_000084, 3, 3, bxsize - 4, 20);
    boxfill8(sht->m_buf, bxsize, COL8_848484, 1, bysize - 2, bxsize - 2,
             bysize - 2);
    boxfill8(sht->m_buf, bxsize, COL8_000000, 0, bysize - 1, bxsize - 1,
             bysize - 1);
}

static void _draw_closebtn(const message_box_t *msg_box) {
    win_sheet_t *sht = msg_box->m_sheet;
    const char *icon =
        closebtn_icon_buf(COL8_000000, COL8_FFFFFF, COL8_848484, COL8_C6C6C6);

    for (int y = 0; y < CLOSEBTN_ICON_HEIGHT; y++) {
        for (int x = 0; x < CLOSEBTN_ICON_WIDTH; x++) {
            unsigned char c = *(icon + y * CLOSEBTN_ICON_WIDTH + x);
            sht->m_buf[(5 + y) * sht->m_bxsize + (sht->m_bxsize - 21 + x)] = c;
        }
    }
}

static void _draw_title(const message_box_t *msg_box, const char *title) {
    show_string(msg_box->m_sheet, 8, 4, COL8_000084, COL8_FFFFFF, title);
}

message_box_t *message_box_new(int x, int y, int width, int height, int sheet_z,
                               const char *title) {
    win_sheet_t *sht = win_sheet_alloc();
    assert(sht != NULL, "message_box_new sheet alloc error");

    message_box_t *msg_box =
        (message_box_t *)memman_alloc_4k(sizeof(message_box_t));
    assert(msg_box != NULL, "message_box_new alloc 4k error");

    unsigned char *buf = (unsigned char *)memman_alloc_4k(width * height);
    assert(buf != NULL, "message_box_new alloc 4k buf error");

    win_sheet_setbuf(sht, buf, width, height, COLOR_INVISIBLE);
    msg_box->m_sheet = sht;

    _draw_background(msg_box);
    _draw_closebtn(msg_box);
    _draw_title(msg_box, title);

    win_sheet_slide(sht, x, y);
    win_sheet_updown(sht, sheet_z);

    return msg_box;
}

void message_box_free(const message_box_t *p) {
    if (p->m_sheet) {
        if (p->m_sheet->m_buf) {
            memman_free_4k(p->m_sheet->m_buf,
                           p->m_sheet->m_bxsize * p->m_sheet->m_bysize);
        }

        win_sheet_free(p->m_sheet);
    }

    memman_free_4k(p, sizeof(message_box_t));
}

void message_box_show(message_box_t *p, int sheet_z) {
    win_sheet_updown(p->m_sheet, sheet_z);
}

void message_box_hide(message_box_t *p) {
    win_sheet_updown(p->m_sheet, HIDE_WIN_SHEET_Z);
}

bool message_box_is_visible(message_box_t *p) {
    return p->m_sheet->m_z >= BOTTOM_WIN_SHEET_Z;
}
