#include "memory.h"
#include "colo8.h"
#include "draw.h"
#include "io.h"
#include "kutil.h"

void show_memory_block_counts(void) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    int xsize = g_boot_info.m_screen_x;
    int cnt = get_memory_block_count();
    char *p = int2hexstr(cnt);
    show_string(vram, xsize, 0, 0, COL8_FFFFFF, p);
}

void show_memory_block_info(addr_range_desc_t *desc, unsigned char *vram,
                            int page, int xsize, int color) {
    int x = 0, y = 0, gap = 13 * 8;
    char *p = NULL;

    boxfill8(vram, xsize, COL8_008484, 0, 0, xsize, 100);

    const char *title[] = {"page: ",       "base_addr_L: ", "base_addr_H: ",
                           "length_low: ", "length_high: ", "type: "};

    const int ele[] = {page,
                       desc->m_base_addr_low,
                       desc->m_base_addr_high,
                       desc->m_length_low,
                       desc->m_length_high,
                       desc->m_type};

    for (int i = 0, y = 0; i < sizeof(title) / sizeof(title[0]); i++, y += 16) {
        show_string(vram, xsize, x, y, color, title[i]);
        p = int2hexstr(ele[i]);
        show_string(vram, xsize, gap, y, color, p);
    }
}
