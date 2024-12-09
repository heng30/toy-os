#include "memory.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "io.h"
#include "kutil.h"

typedef struct {
    unsigned int m_addr, m_size;
} freeinfo_t;

typedef struct {
    int m_frees, m_lostsize, m_losts;
    freeinfo_t *m_free;
} memman_t;

memman_t g_memman = {
    .m_frees = 0,
    .m_lostsize = 0,
    .m_losts = 0,
    .m_free = (freeinfo_t *)NULL,
};

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

void memman_init(void) {
    int cnt = get_memory_block_count();
    unsigned int mem_size = 0, max_mem_index = 0;
    addr_range_desc_t *desc = (addr_range_desc_t *)get_memory_block_buffer();

    // 获取最大可用的内存区域
    for (int i = 0; i < cnt; i++) {
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
    for (int i = 0; i < g_memman.m_frees; i++) {
        t += g_memman.m_free[i].m_size;
    }

    return t;
}

void *memman_alloc(unsigned int size) {
    for (int i = 0; i < g_memman.m_frees; i++) {
        if (g_memman.m_free[i].m_size >= size) {
            unsigned int a = g_memman.m_free[i].m_addr;
            g_memman.m_free[i].m_addr += size;
            g_memman.m_free[i].m_size -= size;

            // 将成员向前推进1位来释放内存
            if (g_memman.m_free[i].m_size == 0) {
                for (int j = i; j < g_memman.m_frees - 1; j++) {
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
    int i = 0;

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
        for (int j = g_memman.m_frees; j > i; j--) {
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

void show_memman_info(void) {
    int total = memman_total() / (1024 * 1024);
    char *p = int2hexstr(total);
    show_string(0, 0, COL8_FFFFFF, "Total memory is:");
    show_string(17 * 8, 0, COL8_FFFFFF, p);
    show_string(28 * 8, 0, COL8_FFFFFF, "MB");
}

void memman_test(void) {
    int total = memman_total();
    show_debug_int(total);

    void *buf[5];

    for (int i = 0; i < 5; i++) {
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

    for (int i = 0; i < 5; i++) {
        memman_free(buf[i], 32 * (i + 1));
    }

    total = memman_total();
    show_debug_int(total);

    show_debug_int(0xFFFFFFFF);

    show_debug_int(g_memman.m_losts);
    show_debug_int(g_memman.m_lostsize);
}
