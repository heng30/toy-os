#include "memory.h"
#include "colo8.h"
#include "draw.h"
#include "io.h"
#include "kutil.h"

void show_memory_block_counts_and_addr(void) {
    int gap = 13 * 8;
    char *p = NULL;

    int cnt = get_memory_block_count();
    p = int2hexstr(cnt);
    show_string(0, 0, COL8_FFFFFF, "pages: ");
    show_string(gap, 0, COL8_FFFFFF, p);

    addr_range_desc_t *desc = (addr_range_desc_t *)get_memory_block_buffer();
    p = int2hexstr((int)desc);
    show_string(0, 16, COL8_FFFFFF, "address: ");
    show_string(gap, 16, COL8_FFFFFF, p);
}

void show_memory_block_info(addr_range_desc_t *desc, int page, int color) {
    int x = 0, y = 0, gap = 13 * 8;
    char *p = NULL;

    int xsize = g_boot_info.m_screen_x;
    boxfill8(COL8_008484, 0, 0, xsize, 100);

    const char *title[] = {
        "page: ",       "base_addr_L: ", "base_addr_H: ",
        "length_low: ", "length_high: ", "type: ",
    };

    const int ele[] = {
        page,
        desc->m_base_addr_low,
        desc->m_base_addr_high,
        desc->m_length_low,
        desc->m_length_high,
        desc->m_type,
    };

    for (int i = 0, y = 0; i < sizeof(title) / sizeof(title[0]); i++, y += 16) {
        show_string(x, y, color, title[i]);
        p = int2hexstr(ele[i]);
        show_string(gap, y, color, p);
    }
}
