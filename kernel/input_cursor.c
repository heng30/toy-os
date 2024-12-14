#include "input_cursor.h"
#include "colo8.h"
#include "draw.h"
#include "kutil.h"

win_sheet_t *g_input_cursor_sht = NULL;
timer_t *g_input_cursor_timer = NULL;

input_cursor_t g_input_cursor = {
    .m_input_cursor_abs_x = 0,
    .m_input_cursor_abs_y = 0,
    .m_input_cursor_color = COLOR_WHITE,
};

static void _init_input_cursor_timer(void) {
    g_input_cursor_timer = timer_alloc();
    set_timer(g_input_cursor_timer, INPUT_CURSOR_BLINK_INTERVAL,
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

    win_sheet_slide(g_input_cursor_sht, g_input_cursor.m_input_cursor_abs_x,
                    g_input_cursor.m_input_cursor_abs_y);
}

void init_input_cursor(void) {
    _init_input_cursor_sheet();
    _init_input_cursor_timer();
}

void input_cursor_show(int z) { win_sheet_show(g_input_cursor_sht, z); }

void input_cursor_hide(void) {
    win_sheet_show(g_input_cursor_sht, HIDE_WIN_SHEET_Z);
}

void input_cursor_move(int vx, int vy) {
    g_input_cursor.m_input_cursor_abs_x = vx,
    g_input_cursor.m_input_cursor_abs_y = vy;
    win_sheet_slide(g_input_cursor_sht, vx, vy);
}

bool input_cursor_is_visible(void) {
    return g_input_cursor_sht->m_z >= BOTTOM_WIN_SHEET_Z;
}

void input_cursor_blink(void) {
    g_input_cursor.m_input_cursor_color =
        g_input_cursor.m_input_cursor_color == COLOR_WHITE ? COLOR_BLACK
                                                           : COLOR_WHITE;

    // 重新绘制光标颜色
    boxfill8(g_input_cursor.m_input_cursor, INPUT_CURSOR_WIDTH,
             g_input_cursor.m_input_cursor_color, 0, 0, INPUT_CURSOR_WIDTH - 1,
             INPUT_CURSOR_HEIGHT - 1);

    win_sheet_refresh(g_input_cursor_sht, 0, 0, INPUT_CURSOR_WIDTH,
                      INPUT_CURSOR_HEIGHT);

    set_timer(g_input_cursor_timer, 50, INPUT_CURSOR_TIMER_DATA);
}
