#pragma once

#include "def.h"

#define FIFO8_FLAGS_OVERFLOW 0x0001

// 保存键盘和鼠标数据
typedef struct {
    unsigned char *m_buf;
    unsigned int m_p, m_q, m_size, m_free, m_flags;
} fifo8_t;

// 分配一个大小为queue_size的队列
fifo8_t *fifo8_alloc(unsigned int queue_size);

// 恢复默认设置
void fifo8_reset(fifo8_t *f);

// 获取数据, 没有数据返回-1
int fifo8_get(fifo8_t *fifo);

// 添加数据, 没有空间返回-1
int fifo8_put(fifo8_t *fifo, unsigned char data);

// 获取可用数据数量
bool fifo8_is_empty(fifo8_t *fifo);
