#include "sheet_userdata.h"
#include "colo8.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"
#include "widgets/input_box.h"

void sheet_userdata_set(sheet_userdata_t *p, unsigned char type, void *data) {
    p->m_type = type;
    p->m_data = data;
}

sheet_userdata_type_moving_t *sheet_userdata_type_moving_alloc(void) {
    sheet_userdata_type_moving_t *p =
        memman_alloc_4k(sizeof(sheet_userdata_type_moving_t));

    assert(p != NULL, "sheet_userdata_type_moving_alloc error");

    return p;
}

void sheet_userdata_type_moving_free(sheet_userdata_type_moving_t *p) {
    memman_free_4k(p, sizeof(sheet_userdata_type_moving_t));
}

void sheet_userdata_type_moving_set(sheet_userdata_type_moving_t *p,
                                    sheet_userdata_type_moving_cb cb,
                                    void *data, const char* func_name) {
    p->m_cb = cb;
    p->m_data = data;
    p->m_func_name = func_name;
}

void _hander_moving_sheet(sheet_userdata_type_moving_t *p) {
    if (!strcmp(p->m_func_name, "input_box_moving")) {
        input_box_moving(p->m_data);
    }
}

void moving_sheet(void) {
    win_sheet_t *sht = win_sheet_get_moving_sheet();
    if (!sht)
        return;

    sheet_userdata_t *userdata = &sht->m_userdata;

    switch (userdata->m_type) {
    case SHEET_USERDATA_TYPE_MOVING: {
        sheet_userdata_type_moving_t *p =
            (sheet_userdata_type_moving_t *)(userdata->m_data);

#ifdef __DEBUG__
        show_string_in_canvas(0, FONT_HEIGHT, COLOR_WHITE, int2hexstr((int)p));
        show_string_in_canvas(FONT_WIDTH * 10, FONT_HEIGHT, COLOR_WHITE,
                              int2hexstr((int)p->m_cb));
        show_string_in_canvas(FONT_WIDTH * 20, FONT_HEIGHT, COLOR_WHITE,
                              int2hexstr((int)p->m_data));
#endif

#ifdef __USE_CALLBAK_MOVING_FUNCTION__
        p->m_cb(p->m_data);
#else
        _hander_moving_sheet(p);
#endif
        break;
    }
    default:
        break;
    }
}
