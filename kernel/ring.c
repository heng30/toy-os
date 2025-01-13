#include "ring.h"
#include "kutil.h"
#include "memory.h"

ring_t *ring_alloc(unsigned int size) {
    ring_t *r = (ring_t *)memman_alloc_4k(sizeof(ring_t));
    assert(r != NULL, "ring_alloc alloc error");

    void **buf = (void **)memman_alloc_4k(size * sizeof(void *));
    assert(buf != NULL, "ring_alloc alloc buffer error");

    r->m_buf = buf, r->m_size = size, r->m_free = size;
    r->m_flags = RING_FLAGS_INIT, r->m_h = 0, r->m_e = 0;

    return r;
}

void ring_free(ring_t *r) {
    memman_free_4k(r->m_buf, r->m_size * sizeof(void *));
    memman_free_4k(r, sizeof(ring_t));
}

void ring_reset(ring_t *r) {
    r->m_free = r->m_size;
    r->m_flags = RING_FLAGS_INIT;
    r->m_e = 0;
    r->m_h = 0;
}

bool ring_put(ring_t *r, void *data) {
    if (r->m_free == 0) {
        r->m_flags |= RING_FLAGS_OVERFLOW;
        return false;
    }

    r->m_buf[r->m_h] = data;
    r->m_h++;
    if (r->m_h == r->m_size) {
        r->m_h = 0;
    }

    r->m_free--;
    return true;
}

void *ring_get(ring_t *r) {
    if (r->m_free == r->m_size) {
        return NULL;
    }

    void *data = r->m_buf[r->m_e];
    r->m_e++;
    if (r->m_e == r->m_size) {
        r->m_e = 0;
    }

    r->m_free++;
    return data;
}

bool ring_is_empty(ring_t *r) { return r->m_size == r->m_free; }

unsigned int ring_len(ring_t *r) { return r->m_size - r->m_free; }

#ifdef __RING_TEST__
void ring_test(void) {
    ring_t *r = ring_alloc(10);
    for (int i = 0; i < 5; i++) {
        ring_put(r, (void *)i);
    }

    for (int i = 0; i < 5; i++) {
        void *data = ring_get(r);
        assert((int)data == i, "ring_test 5 elements failed");
    }

    for (int i = 0; i < 8; i++) {
        ring_put(r, (void *)i);
    }

    for (int i = 0; i < 8; i++) {
        void *data = ring_get(r);
        assert((int)data == i, "ring_test 8 elements failed");
    }

    for (int i = 0; i < 10; i++) {
        bool ret = ring_put(r, (void *)i);
        assert(ret, "ring_put test failed, it should be true");
    }

    bool ret = ring_put(r, (void *)1);
    assert(!ret, "ring_put test failed, it should be false");

    for (int i = 0; i < 10; i++) {
        ring_get(r);
    }

    assert(ring_is_empty(r), "ring_is_empty test failed");

    ring_free(r);
}
#endif
