#pragma once

#define NULL ((void*)0);

// 内存块信息
typedef struct {
    unsigned int m_base_addr_low;
    unsigned int m_base_addr_high;
    unsigned int m_length_low;
    unsigned int m_length_high;
    unsigned int m_type;
} addr_range_desc_t;

// 显示内存块数量
void show_memory_block_counts(void);

// 显示内存块信息
void show_memory_block_info(addr_range_desc_t *desc, unsigned char *vram,
                            int page, int xsize, int color);
