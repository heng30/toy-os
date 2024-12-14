#include "fifo8.h"
#include "kutil.h"
#include "memory.h"

fifo8_t *fifo8_alloc(unsigned int queue_size) {
    fifo8_t *f = (fifo8_t *)memman_alloc(sizeof(fifo8_t));
    assert(f != NULL, "fifo8_alloc alloc error");

    unsigned char *buf = (unsigned char *)memman_alloc(queue_size);
    assert(buf != NULL, "fifo8_alloc alloc buffer error");

    f->m_buf = buf;
    f->m_size = queue_size, f->m_free = queue_size;
    f->m_flags = 0, f->m_p = 0, f->m_q = 0;

    return f;
}

void fifo8_reset(fifo8_t *f) {
    f->m_free = f->m_size;
    f->m_flags = 0;
    f->m_p = 0;
    f->m_q = 0;
}

int fifo8_put(fifo8_t *fifo, unsigned char data) {
    if (fifo->m_free == 0) {
        fifo->m_flags |= FIFO8_FLAGS_OVERFLOW;
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

unsigned char fifo8_get(fifo8_t *fifo) {
    if (fifo->m_free == fifo->m_size) {
        return -1;
    }

    unsigned char data = fifo->m_buf[fifo->m_q];
    fifo->m_q++;
    if (fifo->m_q == fifo->m_size) {
        fifo->m_q = 0;
    }

    fifo->m_free++;
    return data;
}

int fifo8_status(fifo8_t *fifo) { return fifo->m_size - fifo->m_free; }
