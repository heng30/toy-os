#include "fs_reader.h"
#include "def.h"
#include "kutil.h"
#include "memory.h"

unsigned char *fs_read(unsigned char *start_addr, const char *filename) {
    char *fname = (char *)memman_alloc_4k(FS_HEADER_FILENAME_SIZE);
    assert(fname != NULL, "fs_read alloc fname buffer error");

    fs_header_t *p = (fs_header_t *)start_addr;

    for (; p->m_type != FS_HEADER_TYPE_END; p++) {
        get_filename(p, fname, FS_HEADER_FILENAME_SIZE);

        if (strcmp(filename, fname))
            continue;

        memman_free_4k(fname, FS_HEADER_FILENAME_SIZE);

        unsigned char *buf = (unsigned char *)memman_alloc_4k(p->m_size + 1);
        assert(buf != NULL, "fs_read alloc file size buffer error");

        unsigned char *data_ptr = start_addr + p->m_clustno * SECTOR_SIZE;
        memcpy(buf, data_ptr, p->m_size);
        buf[p->m_size] = '\0';
        return buf;
    }

    memman_free_4k(fname, FS_HEADER_FILENAME_SIZE);
    return NULL;
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

void fs_header_display(fs_header_t *p) {
    char fname[FS_HEADER_FILENAME_SIZE];
    get_filename(p, fname, FS_HEADER_FILENAME_SIZE);

    // debug("filename: %s", fname);
    // debug("clustno: %d", p->m_clustno);
    // debug("flag: %d", p->m_type);
    // debug("size: %d", p->m_size);
}
