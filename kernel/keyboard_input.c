#include "write_vga_desktop_background.inc"

void keyboard_input(void) {
    init_palette();
    init_mouse_cursor(g_mcursor, COL8_008484);

    write_vga_desktop_background();

    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;

    // 显示鼠标
    put_block(vram, xsize, 16, 16, 80, 80, g_mcursor, 16);

    io_sti(); // 开中断

    int data = 0;
    for (;;) {
        io_cli();
        if (g_keybuf.m_len == 0) {
            io_stihlt();
        } else {
            data = g_keybuf.m_keybuf[g_keybuf.m_next_r];
            g_keybuf.m_next_r = (g_keybuf.m_next_r + 1) % 32;
            io_sti();
            g_keybuf.m_len--;

            char *pstr = char2hexstr(data);
            static int show_pos = 0;
            show_string(vram, xsize, show_pos, 0, COL8_FFFFFF, pstr);
            show_pos += 32;
        }
    }
}

void int_handler_from_c(char *esp) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int ysize = g_boot_info.m_screen_y;

    io_out8(PIC_OCW2, 0x21);
    unsigned char data = 0;
    data = io_in8(PORT_KEYDAT);

    if (g_keybuf.m_len < 32) {
        g_keybuf.m_keybuf[g_keybuf.m_next_w] = data;
        g_keybuf.m_len++;
        g_keybuf.m_next_w = (g_keybuf.m_next_w + 1) % 32;
    }
}
