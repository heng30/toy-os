#include "colo8.h"
#include "draw.h"
#include "io.h"
#include "keyboard.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"

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

void start_kernel(void) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    init_palette();
    // init_mouse_cursor(g_mcursor, COL8_008484);

    // init_keyboard();

    write_vga_desktop_background();

    // int mem_count = get_memory_block_count();
    // addr_range_desc_t *mem_desc =
    //     (addr_range_desc_t *)get_memory_block_buffer();

    // 显示鼠标
    // put_block(vram, xsize, 16, 16, g_mdec.m_abs_x, g_mdec.m_abs_y, g_mcursor,
    //           16);

    // show_memory_block_counts();
    // char *p_page_cnt = int2hexstr((int)mem_desc);
    // show_string(vram, xsize, 0, 32, COL8_FFFFFF, p_page_cnt);

    // io_sti(); // 开中断
    // enable_mouse();

    // for (;;) {
    //     io_cli();
    //     if (fifo8_status(&g_keyinfo) + fifo8_status(&g_mouseinfo) == 0) {
    //         io_stihlt();
    //     } else if (fifo8_status(&g_keyinfo) != 0) {
    //         show_keyboard_input(mem_desc, mem_count);
    //     } else if (fifo8_status(&g_mouseinfo) != 0) {
    //         show_mouse_info();
    //     }
    // }
}
