#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "input_cursor.h"
#include "io.h"
#include "keyboard.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"
#include "multi_task.h"
#include "win_sheet.h"

#include "widgets/canvas.h"
#include "widgets/common_widget.h"
#include "widgets/console.h"

#define CONSOLE_WIDTH 240
#define CONSOLE_HEIGHT 200
#define CONSOLE_INPUT_AREA_MARGIN 8
#define CONSOLE_INPUT_AREA_PADDING 0

#define CONSOLE_PROMPT_WIDTH (FONT_WIDTH * 2)

// 黑框的大小
#define INPUT_AREA_WIDTH (CONSOLE_WIDTH - CONSOLE_INPUT_AREA_MARGIN * 2)
#define INPUT_AREA_HEIGHT                                                      \
    (CONSOLE_HEIGHT - CONSOLE_INPUT_AREA_MARGIN * 2 - TITLE_BAR_HEIGHT)

// 黑框开始坐标
#define INPUT_AREA_START_X_OFFSET CONSOLE_INPUT_AREA_MARGIN
#define INPUT_AREA_START_Y_OFFSET (TITLE_BAR_HEIGHT + CONSOLE_INPUT_AREA_MARGIN)

// 输入光标开始坐标
#define INPUT_CURSOR_START_X_OFFSET                                            \
    (INPUT_AREA_START_X_OFFSET + CONSOLE_INPUT_AREA_PADDING)
#define INPUT_CURSOR_START_Y_OFFSET                                            \
    (INPUT_AREA_START_Y_OFFSET + CONSOLE_INPUT_AREA_PADDING)

// 输入光标结束位置
#define INPUT_CURSOR_END_X_OFFSET                                              \
    (INPUT_CURSOR_START_X_OFFSET +                                             \
     ((INPUT_AREA_WIDTH - CONSOLE_INPUT_AREA_PADDING * 2) / FONT_WIDTH) *      \
         FONT_WIDTH)
#define INPUT_CURSOR_END_Y_OFFSET                                              \
    (INPUT_CURSOR_START_Y_OFFSET +                                             \
     ((INPUT_AREA_HEIGHT - CONSOLE_INPUT_AREA_PADDING * 2) / FONT_HEIGHT) *    \
         FONT_HEIGHT)

// 绘制黑框
static void _console_draw_input_area(win_sheet_t *sht) {
    make_textbox8(sht, INPUT_AREA_START_X_OFFSET, INPUT_AREA_START_Y_OFFSET,
                  INPUT_AREA_WIDTH, INPUT_AREA_HEIGHT, COLOR_BLACK);
}

// 绘制终端提示符
static void _console_draw_prompt(console_t *p, unsigned int y) {
    show_string(p->m_win->m_sheet, INPUT_CURSOR_START_X_OFFSET, y, COLOR_BLACK,
                COLOR_WHITE, "> ");
    p->m_cursor_pos.m_x = INPUT_CURSOR_START_X_OFFSET + CONSOLE_PROMPT_WIDTH;
}

// 是否需要换行
static bool _console_is_text_need_wrap(console_t *p) {
    return p->m_cursor_pos.m_x + FONT_WIDTH >= INPUT_CURSOR_END_X_OFFSET;
}

// 是否需要回到上一行的末尾
static bool _console_is_text_need_back_to_preview_row(console_t *p) {
    return p->m_cursor_pos.m_x <= INPUT_CURSOR_START_X_OFFSET;
}

// 是否需要向上滚动一行
static bool _console_is_need_scroll_up_one_line(console_t *p) {
    return p->m_cursor_pos.m_y + FONT_HEIGHT >= INPUT_AREA_START_Y_OFFSET +
                                                    INPUT_AREA_HEIGHT -
                                                    CONSOLE_INPUT_AREA_PADDING;
}

static void _console_draw_ch(console_t *p, const char ch) {
    unsigned int x = p->m_cursor_pos.m_x, y = p->m_cursor_pos.m_y;

    strpush(p->m_text, ch);
    show_char(p->m_win->m_sheet, x, y, COLOR_BLACK, COLOR_WHITE, ch);

    if (_console_is_text_need_wrap(p)) {
        p->m_cursor_pos.m_x = INPUT_CURSOR_START_X_OFFSET;
        p->m_cursor_pos.m_y += FONT_HEIGHT;
    } else {
        p->m_cursor_pos.m_x += FONT_WIDTH;
    }

    console_focus(p);
}

static void _console_remove_ch(console_t *p) {
    if (strlen(p->m_text) == 0)
        return;

    strpop(p->m_text);

    if (_console_is_text_need_back_to_preview_row(p)) {
        p->m_cursor_pos.m_x = INPUT_CURSOR_END_X_OFFSET - FONT_WIDTH;
        p->m_cursor_pos.m_y -= FONT_HEIGHT;
    } else {
        p->m_cursor_pos.m_x -= FONT_WIDTH;
    }

    unsigned int x = p->m_cursor_pos.m_x, y = p->m_cursor_pos.m_y;
    show_char(p->m_win->m_sheet, x, y, COLOR_BLACK, COLOR_BLACK, ' ');
    console_focus(p);
}

static void _console_input_area_clear_all(console_t *p) {
    p->m_text[0] = '\0';
    p->m_cursor_pos.m_y = INPUT_CURSOR_START_Y_OFFSET;
    _console_draw_input_area(p->m_win->m_sheet);
    _console_draw_prompt(p, p->m_cursor_pos.m_y);
    win_sheet_refresh(p->m_win->m_sheet, INPUT_AREA_START_X_OFFSET,
                      INPUT_AREA_START_Y_OFFSET, INPUT_CURSOR_END_X_OFFSET,
                      INPUT_CURSOR_END_Y_OFFSET);
}

static void _console_handle_command(console_t *p) {
    const char *cmd = p->m_text;

    if (!strcmp(cmd, "clear")) {
        _console_input_area_clear_all(p);
    }

    p->m_text[0] = '\0';
}

static void _console_enter_pressed(console_t *p) {
    if (_console_is_need_scroll_up_one_line(p)) {
        // TODO: reach the end of input area of Y
    } else {
        p->m_cursor_pos.m_y += FONT_HEIGHT;
    }

    _console_draw_prompt(p, p->m_cursor_pos.m_y);
    _console_handle_command(p);

    console_focus(p);
}

static void _console_push(console_t *p, char c) {
    if (strlen(p->m_text) < CONSOLE_TEXT_MAX_LEN - 1) {
        _console_draw_ch(p, c);
    }
}

static void _console_pop(console_t *p) {
    if (strlen(p->m_text) == 0)
        return;

    _console_remove_ch(p);
}

void console_moving(void *p) {
    console_t *con = (console_t *)p;
    window_t *win = con->m_win;

    unsigned int vx = win->m_sheet->m_vx0, vy = win->m_sheet->m_vy0;
    int dx = g_mdec.m_rel_x, dy = g_mdec.m_rel_y;

    vx = (unsigned int)bound((int)vx + dx, 0,
                             (int)g_boot_info.m_screen_x -
                                 (int)win->m_sheet->m_bxsize);
    vy = (unsigned int)bound((int)vy + dy, 0,
                             (int)g_boot_info.m_screen_y -
                                 (int)win->m_sheet->m_bysize);

    win_sheet_slide(win->m_sheet, vx, vy);
    input_cursor_move(vx + con->m_cursor_pos.m_x, vy + con->m_cursor_pos.m_y);
}

void console_focus(console_t *p) {
    window_t *win = p->m_win;
    unsigned int vx = win->m_sheet->m_vx0, vy = win->m_sheet->m_vy0;
    window_ctl_set_focus_window(win);
    win_sheet_show(g_input_cursor_sht, win->m_sheet->m_z);
    input_cursor_move(vx + p->m_cursor_pos.m_x, vy + p->m_cursor_pos.m_y);
}

console_t *console_new(unsigned int x, unsigned int y, unsigned int width,
                       unsigned int height, const char *title) {
    console_t *p = (console_t *)memman_alloc_4k(sizeof(console_t));
    assert(p != NULL, "console_new alloc 4k error");

    p->m_win = window_new(x, y, width, height, WINDOW_ID_CONSOLE, title, p);
    _console_input_area_clear_all(p);
    return p;
}

void console_free(const console_t *p) {
    window_free(p->m_win);
    memman_free_4k(p, sizeof(console_t));
}

void console_show(console_t *p, int z) {
    window_show(p->m_win, z);
    console_focus(p);
}

void console_hide(console_t *p) {
    if (window_ctl_is_focus_window(p->m_win)) {
        win_sheet_hide(g_input_cursor_sht);
    }
    window_hide(p->m_win);
}

static void _console_task_main(task_t *task, const char *title) {
    console_t *console =
        console_new(300, 50, CONSOLE_WIDTH, CONSOLE_HEIGHT, title);
    console->m_win->m_task = task;
    window_ctl_add(console->m_win);

    for (;;) {
        if (fifo8_is_empty(&g_keyinfo) ||
            console->m_win != g_window_ctl.m_focus_window)
            continue;

        io_cli();
        int c = fifo8_get(&g_keyinfo);
        io_sti();

        if (c < 0)
            continue;

        unsigned char code = (unsigned char)c;
        show_keyboard_input(code);
        set_modkey_status(code);

        // alt+tab切换焦点窗口
        if (is_alt_key_pressed() && is_tab_down(code)) {
            window_ctl_focus_next_window();
            continue;
        }

        if (is_backspace_down(code)) {
            _console_pop(console);
        } else if (is_enter_down(code)) {
            _console_enter_pressed(console);
        } else {
            char ch = get_pressed_char(code);
            if (ch == 0)
                continue;

            _console_push(console, ch);
        }
    }
}

task_t *init_console_task(void) {
    static void *_console_task_argv[] = {"Console"};
    task_t *t = multi_task_alloc((ptr_t)_console_task_main, 1,
                                 _console_task_argv, ONE_RUNNING_TIME_SLICE);
    multi_task_run(t);
    return t;
}
