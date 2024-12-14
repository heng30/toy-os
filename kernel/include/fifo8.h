#pragma once

#include "fifo8.h"

#define FIFO8_FLAGS_OVERFLOW 0x0001
#define FIFO8_EMPTY_STATUS 0    // 没有数据

// 保存键盘和鼠标数据
typedef struct {
    unsigned char *m_buf;
    int m_p, m_q, m_size, m_free, m_flags;
} fifo8_t;

// 分配一个大小为queue_size的队列
fifo8_t *fifo8_alloc(unsigned int queue_size);

// 恢复默认设置
void fifo8_reset(fifo8_t *f);

// 获取数据
int fifo8_get(fifo8_t *fifo);

// 添加数据
int fifo8_put(fifo8_t *fifo, unsigned char data);

// 获取可用数据数量
int fifo8_status(fifo8_t *fifo);
