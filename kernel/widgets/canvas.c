#include "colo8.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"

#include "widgets/canvas.h"

void init_canvas_sheet(int z) {
    unsigned int xsize = g_boot_info.m_screen_x, ysize = g_boot_info.m_screen_y;
    unsigned char *buf = (unsigned char *)memman_alloc_4k(xsize * ysize);
    assert(buf != NULL, "show_string_in_canvas memory alloc error");

    set_buffer(buf, xsize * ysize, COLOR_INVISIBLE);

    g_canvas_sht = win_sheet_alloc();
    assert(g_canvas_sht != NULL, "show_string_in_canvas sheet alloc error");

    g_canvas_sht->m_is_transparent_layer = true;
    win_sheet_setbuf(g_canvas_sht, "canvas", buf, xsize, ysize,
                     COLOR_INVISIBLE);
    win_sheet_slide(g_canvas_sht, 0, 0);
    win_sheet_show(g_canvas_sht, z);
}

void show_string_in_canvas(unsigned int x, unsigned int y, unsigned char color,
                           const char *s) {
    if (!g_canvas_sht)
        return;

    show_string(g_canvas_sht, x, y, COLOR_INVISIBLE, color, s);
}

void clear_canvas(void) {
    if (!g_canvas_sht)
        return;

    set_buffer(g_canvas_sht->m_buf,
               g_canvas_sht->m_bxsize * g_canvas_sht->m_bysize,
               COLOR_INVISIBLE);

    win_sheet_refresh_force(g_canvas_sht, 0, 0, g_canvas_sht->m_bxsize,
                            g_background_sht->m_bysize);
}

void clear_canvas_space(unsigned int x, unsigned int y, unsigned int w,
                        unsigned int h) {
    if (!g_canvas_sht)
        return;

    set_buffer_space(g_canvas_sht->m_buf, g_canvas_sht->m_bxsize, x, y, w, h,
                     COLOR_INVISIBLE);

    win_sheet_refresh_force(g_canvas_sht, x, y, x + w, y + h);
}
