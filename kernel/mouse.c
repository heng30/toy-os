#include "mouse.h"
#include "colo8.h"
#include "draw.h"
#include "fifo8.h"
#include "io.h"
#include "keyboard_mouse.h"
#include "kutil.h"

#include "widgets/canvas.h"

win_sheet_t *g_mouse_sht = NULL;

task_t *g_mouse_task = NULL;

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
};

void init_mouse(void) {
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
    g_mdec.m_abs_x =
        (unsigned int)bound((int)g_mdec.m_abs_x + g_mdec.m_rel_x, 0,
                            (int)g_boot_info.m_screen_x - CURSOR_ICON_SIZE);
    g_mdec.m_abs_y =
        (unsigned int)bound((int)g_mdec.m_abs_y + g_mdec.m_rel_y, 0,
                            (int)g_boot_info.m_screen_y - CURSOR_ICON_SIZE);
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
            g_mdec.m_rel_x |= (int)0xffffff00;
        }

        if ((g_mdec.m_buf[0] & 0x20) != 0) {
            g_mdec.m_rel_y |= (int)0xffffff00;
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

    win_sheet_setbuf(g_mouse_sht, WIN_SHEET_ID_MOUSE, g_mdec.m_cursor, CURSOR_ICON_SIZE,
                     CURSOR_ICON_SIZE, COLOR_INVISIBLE);

    win_sheet_slide(g_mouse_sht, g_mdec.m_abs_x, g_mdec.m_abs_y);
    win_sheet_show(g_mouse_sht, MOUSE_WIN_SHEET_Z);
}

void draw_mouse(void) {
    compute_mouse_position();
    win_sheet_slide(g_mouse_sht, g_mdec.m_abs_x, g_mdec.m_abs_y);
}

void keep_mouse_sheet_on_top(void) {
    win_sheet_show(g_mouse_sht, MOUSE_WIN_SHEET_Z);
}

// 鼠标中断函数
void int_handler_for_mouse(char *esp) {
    io_out8(PIC1_OCW2, 0x20);
    io_out8(PIC_OCW2, 0x20);

    unsigned char data = io_in8(PORT_KEYDAT);
    fifo8_put(&g_mouseinfo, data);
    multi_task_priority_task_add(g_mouse_task);
}

bool is_mouse_left_btn_pressed(void) { return (g_mdec.m_btn & 0x01) != 0; }
bool is_mouse_right_btn_pressed(void) { return (g_mdec.m_btn & 0x2) != 0; }
bool is_mouse_middle_btn_pressed(void) { return (g_mdec.m_btn & 0x4) != 0; }

static void _mouse_task_main(void) {
    for (;;) {
        io_cli();
        if (fifo8_is_empty(&g_mouseinfo)) {
            io_sti(); // 开中断，保证循环不会被挂起
        } else {
            unsigned char code = (unsigned char)fifo8_get(&g_mouseinfo);

            io_sti();

            if (mouse_decode(code) == 1) {
                draw_mouse();

                if (is_mouse_left_btn_pressed()) {
                    // TODO: 捕获鼠标左键按下
                    moving_sheet();
                }
            }
        }
    }
}

void init_mouse_task(void) {
    g_mouse_task = multi_task_alloc((ptr_t)_mouse_task_main, 1);
}
