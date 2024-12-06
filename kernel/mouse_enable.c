#include "write_vga_desktop_background.inc"

#define PORT_KEYDAT 0x0060
#define PIC_OCW2 0x20
#define PIC1_OCW2 0xA0

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

#define FLAGS_OVERRUN 0x0001

typedef struct {
    unsigned char *m_buf;
    int m_p, m_q, m_size, m_free, m_flags;
} fifo8_t;

static unsigned char g_keybuf[32];
static unsigned char g_mousebuf[128];

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

void show_mouse_info(void) {
    unsigned char data = fifo8_get(&g_mouseinfo);

    io_sti();

    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;
    char *pstr = char2hexstr(data);
    static int mouse_pos = 16;
    if (mouse_pos <= 256) {
        show_string(vram, xsize, mouse_pos, 16, COL8_FFFFFF, pstr);
        mouse_pos += 32;
    }
}

void show_keyboard_input(void) {
    unsigned int data = fifo8_get(&g_keyinfo);

    io_sti();

    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;
    char *pstr = char2hexstr(data);
    static int show_pos = 0;
    show_string(vram, xsize, show_pos, 0, COL8_FFFFFF, pstr);
    show_pos += 32;
}

void mouse_enable(void) {
    init_palette();
    init_mouse_cursor(g_mcursor, COL8_008484);

    init_keyboard();

    write_vga_desktop_background();

    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;

    // 显示鼠标
    put_block(vram, xsize, 16, 16, 80, 80, g_mcursor, 16);

    io_sti(); // 开中断
    enable_mouse();

    for (;;) {
        io_cli();
        if (fifo8_status(&g_keyinfo) + fifo8_status(&g_mouseinfo) == 0) {
            io_stihlt();
        } else if (fifo8_status(&g_keyinfo) != 0) {
            show_keyboard_input();
        } else if (fifo8_status(&g_mouseinfo) != 0) {
            show_mouse_info();
        }
    }
}

void int_handler_from_c(char *esp) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;

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
