#include "fifo8.h"

int fifo8_put(fifo8_t *fifo, unsigned char data) {
    if (fifo->m_free == 0) {
        fifo->m_flags |= FLAGS_OVERRUN;
        return -1;
    }

    fifo->m_buf[fifo->m_p] = data;
    fifo->m_p++;
    if (fifo->m_p == fifo->m_size) {
        fifo->m_p = 0;
    }

    fifo->m_free--;
    return 0;
}

int fifo8_get(fifo8_t *fifo) {
    if (fifo->m_free == fifo->m_size) {
        return -1;
    }

    int data = fifo->m_buf[fifo->m_q];
    fifo->m_q++;
    if (fifo->m_q == fifo->m_size) {
        fifo->m_q = 0;
    }

    fifo->m_free++;
    return data;
}

int fifo8_status(fifo8_t *fifo) { return fifo->m_size - fifo->m_free; }
