#pragma once

#define FLAGS_OVERRUN 0x0001

// 保存键盘和鼠标数据
typedef struct {
    unsigned char *m_buf;
    int m_p, m_q, m_size, m_free, m_flags;
} fifo8_t;

// 获取数据
int fifo8_get(fifo8_t *fifo);

// 添加数据
int fifo8_put(fifo8_t *fifo, unsigned char data);

// 获取可用数据数量
int fifo8_status(fifo8_t *fifo);
