#pragma once

#define MEMMAN_FREES 4096

// 内存块信息
typedef struct {
    unsigned int m_base_addr_low;
    unsigned int m_base_addr_high;
    unsigned int m_length_low;
    unsigned int m_length_high;
    unsigned int m_type;
} addr_range_desc_t;

// 初始化内存管理器
void init_memman(void);

// 总内存量
unsigned int memman_total(void);

// 分配内存
void* memman_alloc(unsigned int size);

// 归还内存
int memman_free(const void* address, unsigned int size);

// 对齐到4k进行分配
void *memman_alloc_4k(unsigned int size);

// 对齐到4k进行释放
int memman_free_4k(const void *addr, unsigned int size);

// 显示内存块数量和内存块起始地址
void show_memory_block_counts_and_addr(void);

// 显示内存块信息
void show_memory_block_info(addr_range_desc_t *desc, int page, int color);

// 显示内存管理器信息
void show_memman_info(void);

void memman_test(void);
