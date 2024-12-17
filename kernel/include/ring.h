#pragma once

#include "def.h"

#define RING_FLAGS_INIT 0x0
#define RING_FLAGS_OVERFLOW 0x1

// 循环缓冲区
typedef struct {
    void **m_buf;         // 循环缓冲区, 成员类型为`void*`
    unsigned int m_h;     // 头下标
    unsigned int m_e;     // 尾下标
    unsigned int m_size;  //  缓冲区成员数量
    unsigned int m_free;  // 空闲成员数量
    unsigned int m_flags; // 标志位
} ring_t;

// 分配一个大小为size的缓冲区
// size: 成员数量
ring_t *ring_alloc(unsigned int size);

// 销毁这个循环缓冲区
void ring_free(ring_t *r);

// 恢复默认设置
void ring_reset(ring_t *r);

// 获取数据, 没有数据返回-1
void *ring_get(ring_t *r);

// 添加数据
bool ring_put(ring_t *r, void *data);

// 获取可用数据数量
bool ring_is_empty(ring_t *r);

// 测试
void ring_test(void);
