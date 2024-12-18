#include "input_cursor.h"
#include "colo8.h"
#include "draw.h"
#include "kutil.h"

win_sheet_t *g_input_cursor_sht = NULL;
timer_t *g_input_cursor_timer = NULL;

input_cursor_t g_input_cursor = {
    .m_input_cursor_color = COLOR_WHITE,
    .m_input_cursor_color_show = COLOR_WHITE,
    .m_input_cursor_color_hide = COLOR_BLACK,
};

static void _init_input_cursor_timer(void) {
    g_input_cursor_timer = timer_alloc();
    set_timer(g_input_cursor_timer, 1, TIMER_MAX_RUN_COUNTS,
              INPUT_CURSOR_TIMER_DATA);
}

static void _init_input_cursor_sheet(void) {
    g_input_cursor_sht = win_sheet_alloc();
    assert(g_input_cursor_sht != NULL,
           "init_input_cursor_sheet alloc sheet error");

    // 绘制光标颜色
    boxfill8(g_input_cursor.m_input_cursor, INPUT_CURSOR_WIDTH,
             g_input_cursor.m_input_cursor_color, 0, 0, INPUT_CURSOR_WIDTH - 1,
             INPUT_CURSOR_HEIGHT - 1);

    win_sheet_setbuf(g_input_cursor_sht, "input-cursor",
                     g_input_cursor.m_input_cursor, INPUT_CURSOR_WIDTH,
                     INPUT_CURSOR_HEIGHT, COLOR_INVISIBLE);

    win_sheet_slide(g_input_cursor_sht, 0, 0);
}

void init_input_cursor(void) {
    _init_input_cursor_sheet();
    _init_input_cursor_timer();
}

void input_cursor_show(int z) { win_sheet_show(g_input_cursor_sht, z); }

void input_cursor_set_color(unsigned char show_color,
                            unsigned char hide_color) {
    g_input_cursor.m_input_cursor_color_show = show_color;
    g_input_cursor.m_input_cursor_color_hide = hide_color;
}

void input_cursor_hide(void) {
    win_sheet_show(g_input_cursor_sht, HIDE_WIN_SHEET_Z);
}

void input_cursor_move(unsigned int vx, unsigned int vy) {
    win_sheet_slide(g_input_cursor_sht, vx, vy);
}

bool input_cursor_is_visible(void) {
    return g_input_cursor_sht->m_z >= BOTTOM_WIN_SHEET_Z;
}

void input_cursor_blink(void) {
    g_input_cursor.m_input_cursor_color =
        g_input_cursor.m_input_cursor_color ==
                g_input_cursor.m_input_cursor_color_show
            ? g_input_cursor.m_input_cursor_color_hide
            : g_input_cursor.m_input_cursor_color_show;

    // 重新绘制光标颜色
    boxfill8(g_input_cursor.m_input_cursor, INPUT_CURSOR_WIDTH,
             g_input_cursor.m_input_cursor_color, 0, 0, INPUT_CURSOR_WIDTH - 1,
             INPUT_CURSOR_HEIGHT - 1);

    win_sheet_refresh(g_input_cursor_sht, 0, 0, INPUT_CURSOR_WIDTH,
                      INPUT_CURSOR_HEIGHT);

    set_timer(g_input_cursor_timer, TIMER_INPUT_CURSOR_TIME_SLICE, 1,
              INPUT_CURSOR_TIMER_DATA);
}
