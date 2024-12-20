#include "memory.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "io.h"
#include "kutil.h"

#include "widgets/canvas.h"

typedef struct {
    unsigned int m_addr, m_size;
} freeinfo_t;

typedef struct {
    unsigned int m_frees, m_lostsize, m_losts;
    freeinfo_t *m_free;
} memman_t;

memman_t g_memman = {
    .m_frees = 0,
    .m_lostsize = 0,
    .m_losts = 0,
    .m_free = (freeinfo_t *)NULL,
};

void init_memman(void) {
    unsigned int cnt = get_memory_block_count();
    unsigned int mem_size = 0, max_mem_index = 0;
    addr_range_desc_t *desc = (addr_range_desc_t *)get_memory_block_buffer();

    // 获取最大可用的内存区域
    for (unsigned int i = 0; i < cnt; i++) {
        if (desc[i].m_type == 1) {
            if (desc[i].m_length_low > mem_size) {
                mem_size = desc[i].m_length_low;
                max_mem_index = i;
            }
        }
    }

    addr_range_desc_t *mem_desc = desc + max_mem_index;
    g_memman.m_free = (freeinfo_t *)mem_desc->m_base_addr_low;

    // 将内存放入内存管理器中
    memman_free((const void *)(mem_desc->m_base_addr_low +
                               MEMMAN_FREES * sizeof(freeinfo_t)),
                mem_desc->m_length_low - MEMMAN_FREES * sizeof(freeinfo_t));
}

unsigned int memman_total(void) {
    unsigned int t = 0;
    for (unsigned int i = 0; i < g_memman.m_frees; i++) {
        t += g_memman.m_free[i].m_size;
    }

    return t;
}

void *memman_alloc(unsigned int size) {
    for (unsigned int i = 0; i < g_memman.m_frees; i++) {
        if (g_memman.m_free[i].m_size >= size) {
            unsigned int a = g_memman.m_free[i].m_addr;
            g_memman.m_free[i].m_addr += size;
            g_memman.m_free[i].m_size -= size;

            // 将成员向前推进1位来释放内存
            if (g_memman.m_free[i].m_size == 0) {
                for (unsigned int j = i; j < g_memman.m_frees - 1; j++) {
                    g_memman.m_free[i].m_addr = g_memman.m_free[i + 1].m_addr;
                    g_memman.m_free[i].m_size = g_memman.m_free[i + 1].m_size;
                }

                g_memman.m_frees--;
            }

            return (void *)a;
        }
    }

    return NULL;
}

int memman_free(const void *address, unsigned int size) {
    unsigned int addr = (unsigned int)address;
    unsigned int i = 0;

    // 查找释放点
    for (i = 0; i < g_memman.m_frees; i++) {
        if (g_memman.m_free[i].m_addr > addr) {
            break;
        }
    }

    // 内存管理器不为空
    if (i > 0) {
        // 释放点的上1个内存刚好和要释放的内存能组成1块更大的内存
        if (g_memman.m_free[i - 1].m_addr + g_memman.m_free[i - 1].m_size ==
            addr) {
            g_memman.m_free[i - 1].m_size += size;

            // 要释放的内存和释放点也能组成一块更大的内存
            if (i < g_memman.m_frees) {
                if (addr + size == g_memman.m_free[i].m_addr) {
                    g_memman.m_free[i - 1].m_size += g_memman.m_free[i].m_size;
                    assert(g_memman.m_frees > 0,
                           "g_memman.m_free count is less than 1");
                    g_memman.m_frees--;
                }
            }

            return 0;
        }
    }

    // 内存管理器不为空
    if (i < g_memman.m_frees) {
        // 要释放的内存和释放点能组成一块更大的内存
        if (addr + size == g_memman.m_free[i].m_addr) {
            g_memman.m_free[i].m_addr = addr;
            g_memman.m_free[i].m_size += size;
            return 0;
        }
    }

    if (g_memman.m_frees < MEMMAN_FREES) {
        // 向后挪动1个位置，给新的成员
        for (unsigned int j = g_memman.m_frees; j > i; j--) {
            g_memman.m_free[j].m_addr = g_memman.m_free[j - 1].m_addr;
            g_memman.m_free[j].m_size = g_memman.m_free[j - 1].m_size;
        }
        g_memman.m_frees++;

        // 添加新成员
        g_memman.m_free[i].m_addr = addr;
        g_memman.m_free[i].m_size = size;
        return 0;
    }

    // 无法放入内存管理器的内存，直接丢弃
    g_memman.m_losts++;
    g_memman.m_lostsize += size;
    return -1;
}

void *memman_alloc_4k(unsigned int size) {
    size = (size + 0xfff) & 0xfffff000;
    return memman_alloc(size);
}

int memman_free_4k(const void *addr, unsigned int size) {
    size = (size + 0xfff) & 0xfffff000;
    return memman_free(addr, size);
}

void show_memory_block_counts_and_addr(void) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    unsigned int xsize = g_boot_info.m_screen_x;
    unsigned int gap = 13 * 8;

    unsigned int cnt = get_memory_block_count();
    show_debug_string(0, 0, COL8_FFFFFF, "pages: ");
    show_debug_string(gap, 0, COL8_FFFFFF, int2hexstr(cnt));

    addr_range_desc_t *desc = (addr_range_desc_t *)get_memory_block_buffer();
    show_debug_string(0, 16, COL8_FFFFFF, "address: ");
    show_debug_string(gap, 16, COL8_FFFFFF, int2hexstr((unsigned int)desc));
}

void show_memory_block_info(addr_range_desc_t *desc, unsigned int page,
                            unsigned char color) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    unsigned int xsize = g_boot_info.m_screen_x;
    unsigned int x = g_boot_info.m_screen_x / 2, y = FONT_HEIGHT, gap = 13 * 8;

    const char *title[] = {
        "page: ",       "base_addr_L: ", "base_addr_H: ",
        "length_low: ", "length_high: ", "type: ",
    };

    const unsigned int ele[] = {
        page,
        desc->m_base_addr_low,
        desc->m_base_addr_high,
        desc->m_length_low,
        desc->m_length_high,
        desc->m_type,
    };

    for (unsigned int i = 0; i < sizeof(title) / sizeof(title[0]);
         i++, y += 16) {
        show_string_in_canvas(x, y, color, title[i]);
        show_string_in_canvas(x + gap, y, color,
                              int2hexstr((unsigned int)ele[i]));
    }
}

void show_all_memory_block_info(void) {
    static unsigned int memory_block_info_counts = 0;
    unsigned int mem_count = get_memory_block_count();
    addr_range_desc_t *mem_desc =
        (addr_range_desc_t *)get_memory_block_buffer();

    show_memory_block_info(mem_desc + memory_block_info_counts,
                           memory_block_info_counts, COL8_FFFFFF);

    if (++memory_block_info_counts >= mem_count) {
        memory_block_info_counts = 0;
    }
}

void show_memman_info(void) {
    unsigned char *vram = g_boot_info.m_vga_ram;
    unsigned int xsize = g_boot_info.m_screen_x;
    unsigned int total = memman_total() / (1024 * 1024);
    const char *p = int2hexstr(total);

    show_string_in_canvas(FONT_WIDTH * 12, 0, COL8_FFFFFF, "total memory is:");
    show_string_in_canvas(FONT_WIDTH * 30, 0, COL8_FFFFFF, p);
    show_string_in_canvas(FONT_WIDTH * 40, 0, COL8_FFFFFF, "MB");
}

void memman_test(void) {
    unsigned int total = memman_total();
    show_debug_int(total);

    void *buf[5];

    for (unsigned int i = 0; i < 5; i++) {
        buf[i] = memman_alloc(32 * (i + 1));
        total = memman_total();
        show_debug_int(total);
    }

    show_debug_int(0xFFFFFFFF);

    memman_free(buf[0], 32);
    total = memman_total();
    show_debug_int(total);

    buf[0] = memman_alloc(32);
    total = memman_total();
    show_debug_int(total);

    show_debug_int(0xFFFFFFFF);

    for (unsigned int i = 0; i < 5; i++) {
        memman_free(buf[i], 32 * (i + 1));
    }

    total = memman_total();
    show_debug_int(total);

    show_debug_int(0xFFFFFFFF);

    show_debug_int(g_memman.m_losts);
    show_debug_int(g_memman.m_lostsize);
}
