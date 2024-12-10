#include "mouse.h"
#include "colo8.h"
#include "draw.h"
#include "fifo8.h"
#include "io.h"
#include "keyboard_mouse.h"
#include "kutil.h"

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

void init_cursor(void) {
    char *mouse = g_mdec.m_cursor;

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

void int_handler_for_mouse(char *esp) {
    io_out8(PIC1_OCW2, 0x20);
    io_out8(PIC_OCW2, 0x20);

    unsigned char data = io_in8(PORT_KEYDAT);
    fifo8_put(&g_mouseinfo, data);
}
