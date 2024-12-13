#include "mouse.h"
#include "colo8.h"
#include "draw.h"
#include "fifo8.h"
#include "io.h"
#include "keyboard_mouse.h"
#include "kutil.h"

win_sheet_t *g_mouse_sht = NULL;
win_sheet_t *g_input_block_sht = NULL;
timer_t *g_input_block_timer = NULL;

// 鼠标图标
extern char cursor_icon[CURSOR_ICON_SIZE][CURSOR_ICON_SIZE];

static unsigned char g_mousebuf[128];
fifo8_t g_mouseinfo = {
    .m_buf = g_mousebuf,
    .m_size = sizeof(g_mousebuf),
    .m_free = sizeof(g_mousebuf),
    .m_flags = 0,
    .m_p = 0,
    .m_q = 0,
};

mouse_dec_t g_mdec = {
    .m_buf = {0, 0, 0},
    .m_phase = MOUSE_PHASE_UNINIT,
    .m_btn = 0,
    .m_rel_x = 0,
    .m_rel_y = 0,
    .m_abs_x = 80,
    .m_abs_y = 80,
    .m_input_block_abs_x = 0,
    .m_input_block_abs_y = 0,
    // .m_input_block_abs_x = 0x58,
    // .m_input_block_abs_y = 0x0,
    .m_input_block_color = COLOR_WHITE,
    .m_focus_sheet = NULL,
};

void init_cursor(void) {
    unsigned char *mouse = g_mdec.m_cursor;

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (cursor_icon[y][x] == '*') {
                mouse[y * 16 + x] = COL8_000000;
            }
            if (cursor_icon[y][x] == 'O') {
                mouse[y * 16 + x] = COL8_FFFFFF;
            }
            if (cursor_icon[y][x] == '.') {
                mouse[y * 16 + x] = COLOR_INVISIBLE;
            }
        }
    }
}

void enable_mouse(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
}

void compute_mouse_position(void) {
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;

    g_mdec.m_abs_x += g_mdec.m_rel_x;
    g_mdec.m_abs_y += g_mdec.m_rel_y;

    if (g_mdec.m_abs_x < 0) {
        g_mdec.m_abs_x = 0;
    }

    if (g_mdec.m_abs_y < 0) {
        g_mdec.m_abs_y = 0;
    }

    if (g_mdec.m_abs_x > xsize - 16) {
        g_mdec.m_abs_x = xsize - 16;
    }

    if (g_mdec.m_abs_y > ysize - 16) {
        g_mdec.m_abs_y = ysize - 16;
    }
}

int mouse_decode(unsigned char dat) {
    // 初始化鼠标成功后会收到一个`0xfa`
    if (g_mdec.m_phase == MOUSE_PHASE_UNINIT) {
        if (dat == 0xfa) {
            g_mdec.m_phase = MOUSE_PHASE_ONE;
        }
        return 0;
    }

    if (g_mdec.m_phase == MOUSE_PHASE_ONE) {
        if ((dat & 0xc8) == 0x08) {
            g_mdec.m_buf[0] = dat;
            g_mdec.m_phase = MOUSE_PHASE_TWO;
        }
        return 0;
    }

    if (g_mdec.m_phase == MOUSE_PHASE_TWO) {
        g_mdec.m_buf[1] = dat;
        g_mdec.m_phase = MOUSE_PHASE_THREE;
        return 0;
    }

    if (g_mdec.m_phase == MOUSE_PHASE_THREE) {
        g_mdec.m_buf[2] = dat;
        g_mdec.m_phase = MOUSE_PHASE_ONE;
        g_mdec.m_btn = g_mdec.m_buf[0] & 0x07;
        g_mdec.m_rel_x = g_mdec.m_buf[1];
        g_mdec.m_rel_y = g_mdec.m_buf[2];

        if ((g_mdec.m_buf[0] & 0x10) != 0) {
            g_mdec.m_rel_x |= 0xffffff00;
        }

        if ((g_mdec.m_buf[0] & 0x20) != 0) {
            g_mdec.m_rel_y |= 0xffffff00;
        }

        g_mdec.m_rel_y = -g_mdec.m_rel_y;
        return 1;
    }

    return -1;
}

// 初始化鼠标图层
void init_mouse_sheet(void) {
    g_mouse_sht = win_sheet_alloc();
    assert(g_mouse_sht != NULL, "init_mouse_sheet alloc sheet error");

    win_sheet_setbuf(g_mouse_sht, "mouse", g_mdec.m_cursor, CURSOR_ICON_SIZE,
                     CURSOR_ICON_SIZE, COLOR_INVISIBLE);

    win_sheet_slide(g_mouse_sht, g_mdec.m_abs_x, g_mdec.m_abs_y);
    win_sheet_updown(g_mouse_sht, MOUSE_WIN_SHEET_Z);
}

void draw_mouse(void) {
    compute_mouse_position();
    win_sheet_slide(g_mouse_sht, g_mdec.m_abs_x, g_mdec.m_abs_y);
}

void keep_mouse_sheet_on_top(void) {
    win_sheet_updown(g_mouse_sht, MOUSE_WIN_SHEET_Z);
}

void int_handler_for_mouse(char *esp) {
    io_out8(PIC1_OCW2, 0x20);
    io_out8(PIC_OCW2, 0x20);

    unsigned char data = io_in8(PORT_KEYDAT);
    fifo8_put(&g_mouseinfo, data);
}

void init_input_block_timer(void) {
    g_input_block_timer = timer_alloc();
    set_timer(g_input_block_timer, 50, INPUT_BLOCK_TIMER_DATA);
}

void init_input_block_sheet(void) {
    g_input_block_sht = win_sheet_alloc();
    assert(g_input_block_sht != NULL,
           "init_input_block_sheet alloc sheet error");

    // 绘制光标颜色
    boxfill8(g_mdec.m_input_block, INPUT_BLOCK_WIDTH,
             g_mdec.m_input_block_color, 0, 0, INPUT_BLOCK_WIDTH - 1,
             INPUT_BLOCK_HEIGHT - 1);

    win_sheet_setbuf(g_input_block_sht, "input-block", g_mdec.m_input_block,
                     INPUT_BLOCK_WIDTH, INPUT_BLOCK_HEIGHT, COLOR_INVISIBLE);

    win_sheet_slide(g_input_block_sht, g_mdec.m_input_block_abs_x,
                    g_mdec.m_input_block_abs_y);
}

void input_block_show(int z) { win_sheet_updown(g_input_block_sht, z); }

void input_block_hide(void) {
    win_sheet_updown(g_input_block_sht, HIDE_WIN_SHEET_Z);
}

void input_block_move(int vx, int vy) {
    g_mdec.m_input_block_abs_x = vx, g_mdec.m_input_block_abs_y = vy;
    win_sheet_slide(g_input_block_sht, vx, vy);
}

bool input_block_is_visible(void) {
    return g_input_block_sht->m_z >= BOTTOM_WIN_SHEET_Z;
}

void input_block_blink(void) {
    g_mdec.m_input_block_color =
        g_mdec.m_input_block_color == COLOR_WHITE ? COLOR_BLACK : COLOR_WHITE;

    // 重新绘制光标颜色
    boxfill8(g_mdec.m_input_block, INPUT_BLOCK_WIDTH,
             g_mdec.m_input_block_color, 0, 0, INPUT_BLOCK_WIDTH - 1,
             INPUT_BLOCK_HEIGHT - 1);

    win_sheet_refresh(g_input_block_sht, 0, 0, INPUT_BLOCK_WIDTH,
                      INPUT_BLOCK_HEIGHT);

    set_timer(g_input_block_timer, 50, INPUT_BLOCK_TIMER_DATA);
}

void set_focus_sheet(win_sheet_t *p) { g_mdec.m_focus_sheet = p; }

bool is_focus_sheet(win_sheet_t *p) { return g_mdec.m_focus_sheet == p; }
