#include "write_vga_desktop_background.inc"

#define PORT_KEYDAT 0x0060
#define PIC_OCW2 0x20
#define PIC1_OCW2 0xA0

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KBC_MODE 0x47

#define KEYCMD_WRITE_MODE 0x60
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

#define FLAGS_OVERRUN 0x0001

#define MOUSE_PHASE_UNINIT 0
#define MOUSE_PHASE_ONE 1
#define MOUSE_PHASE_TWO 2
#define MOUSE_PHASE_THREE 3

typedef struct {
    unsigned char *m_buf;
    int m_p, m_q, m_size, m_free, m_flags;
} fifo8_t;

typedef struct {
    unsigned char m_buf[3];
    unsigned char m_phase;
    int m_rel_x, m_rel_y;
    int m_abs_x, m_abs_y;
} mouse_dec_t;

static unsigned char g_keybuf[64];
static unsigned char g_mousebuf[512];

static fifo8_t g_keyinfo = {
    .m_buf = g_keybuf,
    .m_size = sizeof(g_keybuf),
    .m_free = sizeof(g_keybuf),
    .m_flags = 0,
    .m_p = 0,
    .m_q = 0,
};

static fifo8_t g_mouseinfo = {
    .m_buf = g_mousebuf,
    .m_size = sizeof(g_mousebuf),
    .m_free = sizeof(g_mousebuf),
    .m_flags = 0,
    .m_p = 0,
    .m_q = 0,
};

static mouse_dec_t g_mdec = {
    .m_buf = {0, 0, 0},
    .m_phase = MOUSE_PHASE_UNINIT,
    .m_rel_x = 0,
    .m_rel_y = 0,
    .m_abs_x = 80,
    .m_abs_y = 80,
};

int fifo8_put(fifo8_t *fifo, unsigned char data) {
    if (fifo->m_free == 0) {
        fifo->m_flags |= FLAGS_OVERRUN;
        return -1;
    }

    fifo->m_buf[fifo->m_p] = data;
    fifo->m_p++;
    if (fifo->m_p == fifo->m_size) {
        fifo->m_p = 0;
    }

    fifo->m_free--;
    return 0;
}

int fifo8_get(fifo8_t *fifo) {
    if (fifo->m_free == fifo->m_size) {
        return -1;
    }

    int data = fifo->m_buf[fifo->m_q];
    fifo->m_q++;
    if (fifo->m_q == fifo->m_size) {
        fifo->m_q = 0;
    }

    fifo->m_free++;
    return data;
}

int fifo8_status(fifo8_t *fifo) { return fifo->m_size - fifo->m_free; }

void wait_KBC_sendready() {
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
}

void init_keyboard(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}

void enable_mouse(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
}

void erase_mouse(unsigned char *vram) {
    boxfill8(vram, g_xsize, COL8_008484, g_mdec.m_abs_x, g_mdec.m_abs_y,
             g_mdec.m_abs_x + 15, g_mdec.m_abs_y + 15);
}

void draw_mouse(unsigned char *vram) {
    put_block(vram, g_xsize, 16, 16, g_mdec.m_abs_x, g_mdec.m_abs_y, g_mcursor,
              16);
}

void compute_mouse_position(void) {
    g_mdec.m_abs_x += g_mdec.m_rel_x;
    g_mdec.m_abs_y += g_mdec.m_rel_y;

    if (g_mdec.m_abs_x < 0) {
        g_mdec.m_abs_x = 0;
    }

    if (g_mdec.m_abs_y < 0) {
        g_mdec.m_abs_y = 0;
    }

    if (g_mdec.m_abs_x > g_xsize - 16) {
        g_mdec.m_abs_x = g_xsize - 16;
    }

    if (g_mdec.m_abs_y > g_ysize - 16) {
        g_mdec.m_abs_y = g_ysize - 16;
    }
}

void show_mouse_error(unsigned char data) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    char *pstr = char2hexstr(data);
    show_string(vram, g_xsize, 32, 64, COL8_FFFFFF, pstr);
}

// FIXME: 无法判断鼠标左右移动，应为都是返回0x00
int mouse_decode(unsigned char data) {
    // 初始化鼠标成功后会收到一个`0xfa`
    if (g_mdec.m_phase == MOUSE_PHASE_UNINIT) {
        if (data == 0xfa) {
            g_mdec.m_phase = MOUSE_PHASE_ONE;
        }
        return 0;
    }

    if (g_mdec.m_phase == MOUSE_PHASE_ONE) {
        g_mdec.m_rel_x = 0;
        g_mdec.m_rel_y = 0;

        if (data == 0x00) {
            g_mdec.m_rel_x = 1;
            return 1;
        }

        char det = data & 0x0f;
        if ((data >> 4) == 0x00) { // 向上移动
            g_mdec.m_rel_y = -det - 1;
        } else { // 向下移动
            g_mdec.m_rel_y = 0x0f - det + 1;
        }
        return 1;
    }

    return -1;
}

void show_mouse_info(void) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    unsigned char data = fifo8_get(&g_mouseinfo);

    io_sti();

    if (mouse_decode(data) == 1) {
        erase_mouse(vram);
        compute_mouse_position();
        draw_mouse(vram);
    }
}

void show_keyboard_input(addr_range_desc_t *desc, int mem_count) {
    static int count = 0;
    unsigned int data = fifo8_get(&g_keyinfo);

    io_sti();

    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;

    // 回车键
    if (data == 0x1C) {
        // FIXME: 代码没问题，应该是反汇编的代码问题，导致程序异常退出
        show_memory_block_info(desc + count, vram, count, xsize, COL8_FFFFFF);
        count = (count + 1);
        if (count > mem_count) {
            count = 0;
        }
    }
}

void memory_block_info(void) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    init_palette();
    init_mouse_cursor(g_mcursor, COL8_008484);

    init_keyboard();

    write_vga_desktop_background();

    int mem_count = get_memory_block_count();
    addr_range_desc_t *mem_desc =
        (addr_range_desc_t *)get_memory_block_buffer();

    // 显示鼠标
    put_block(vram, g_xsize, 16, 16, g_mdec.m_abs_x, g_mdec.m_abs_y, g_mcursor,
              16);

    show_memory_block_counts();
    char *p_page_cnt = int2hexstr((int)mem_desc);
    show_string(vram, g_xsize, 0, 32, COL8_FFFFFF, p_page_cnt);

    io_sti(); // 开中断
    enable_mouse();

    for (;;) {
        io_cli();
        if (fifo8_status(&g_keyinfo) + fifo8_status(&g_mouseinfo) == 0) {
            io_stihlt();
        } else if (fifo8_status(&g_keyinfo) != 0) {
            show_keyboard_input(mem_desc, mem_count);
        } else if (fifo8_status(&g_mouseinfo) != 0) {
            show_mouse_info();
        }
    }
}

void int_handler_from_c(char *esp) {
    io_out8(PIC_OCW2, 0x20);
    unsigned char data = io_in8(PORT_KEYDAT);
    fifo8_put(&g_keyinfo, data);
}

void int_handler_for_mouse(char *esp) {
    io_out8(PIC1_OCW2, 0x20);
    io_out8(PIC_OCW2, 0x20);

    unsigned char data = io_in8(PORT_KEYDAT);
    fifo8_put(&g_mouseinfo, data);
}
