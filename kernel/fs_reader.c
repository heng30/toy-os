#include "fs_reader.h"
#include "def.h"
#include "kutil.h"
#include "memory.h"
#include "string.h"

buf_t *fs_read(const char *filename) {
    char fname[FS_HEADER_FILENAME_SIZE];
    fs_header_t *p = (fs_header_t *)FS_START_ADDR;

    for (; p->m_type != FS_HEADER_TYPE_END; p++) {
        get_filename(p, fname, FS_HEADER_FILENAME_SIZE);

        if (strcmp(filename, fname))
            continue;

        buf_t *buf = (buf_t *)memman_alloc_4k(sizeof(buf_t));
        assert(buf != NULL, "fs_read alloc buf_t error");

        buf->m_size = p->m_size + 1;
        buf->m_data = (unsigned char *)memman_alloc_4k(buf->m_size);
        assert(buf->m_data != NULL, "fs_read alloc file size buffer error");

        unsigned char *data_ptr =
            (unsigned char *)FS_START_ADDR + p->m_clustno * SECTOR_SIZE;
        memcpy(buf->m_data, data_ptr, p->m_size);
        buf->m_data[p->m_size] = '\0';
        return buf;
    }

    return NULL;
}

void fs_free_buf(buf_t *p) {
    if (p->m_data)
        memman_free_4k(p->m_data, p->m_size);

    memman_free_4k(p, sizeof(buf_t));
}

void get_filename(fs_header_t *p, char *buf, unsigned int size) {
    assert(size >= FS_HEADER_FILENAME_SIZE,
           "get_filename size is less than FS_HEADER_EXT_SIZE");

    unsigned int i = 0, j = 0;
    for (; i < sizeof(p->m_name); i++) {
        if (!p->m_name[i])
            break;

        buf[i] = p->m_name[i];
    }

    buf[i] = '.';
    i++;

    for (; j < sizeof(p->m_ext); j++) {
        if (!p->m_ext[j])
            break;

        buf[i + j] = p->m_ext[j];
    }

    buf[i + j] = '\0';
}
