#include "closebtn_icon.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"

#include "widgets/common_widget.h"
#include "widgets/message_box.h"

win_sheet_t *message_box_new(unsigned int x, unsigned int y, unsigned int width,
                             unsigned int height, const char *title) {
    win_sheet_t *sht = win_sheet_alloc();
    assert(sht != NULL, "message_box_new sheet alloc error");

    unsigned char *buf = (unsigned char *)memman_alloc_4k(width * height);
    assert(buf != NULL, "message_box_new alloc 4k buf error");

    win_sheet_setbuf(sht, "message_box", buf, width, height, COLOR_INVISIBLE);

    draw_title_bar(sht, title, COLOR_TITLE_BAR_FOCUS);

    win_sheet_slide(sht, x, y);

    return sht;
}

void message_box_free(win_sheet_t *p) {
    memman_free_4k(p->m_buf, p->m_bxsize * p->m_bysize);
    win_sheet_free(p);
}
