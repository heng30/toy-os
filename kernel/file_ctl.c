#include "file_ctl.h"
#include "def.h"
#include "kutil.h"
#include "memory.h"
#include "string.h"

file_ctl_t g_file_ctl;

static bool _file_ctl_is_open_file(int fd) {
    return g_file_ctl.m_des[fd].m_ref > 0;
}

void init_file_ctl(void) {
    unsigned int size = sizeof(file_descriptor_t) * FILE_DESCRIPTOR_MAX;
    g_file_ctl.m_des = (file_descriptor_t *)memman_alloc_4k(size);
    assert(g_file_ctl.m_des != NULL, "file_ctl_init alloc failed");
    memset((unsigned char *)g_file_ctl.m_des, 0, size);
}

void destory_file_ctl(void) {
    memman_free_4k(g_file_ctl.m_des,
                   sizeof(file_descriptor_t) * FILE_DESCRIPTOR_MAX);
}

int file_ctl_open(const char *filename) {
    buf_t *buf = fs_read(filename);

    if (buf) {
        // 已经被打开了，不需要重复打开
        for (int i = 0; i < FILE_DESCRIPTOR_MAX; i++) {
            file_descriptor_t *fp = &g_file_ctl.m_des[i];
            if (fp->m_ref > 0 && !strcmp(filename, fp->m_filename)) {
                fp->m_ref++;
                return i;
            }
        }
    }

    // 查找一个新位置
    for (int i = 0; i < FILE_DESCRIPTOR_MAX; i++) {
        file_descriptor_t *fp = &g_file_ctl.m_des[i];
        if (fp->m_ref > 0)
            continue;

        fp->m_ref = 1;

        if (buf) {
            // 文件在文件系统中
            fp->m_data = buf;
        } else {
            // 文件不再文件系统中
            fp->m_data = memman_alloc_4k(sizeof(buf_t));
            assert(fp->m_data != NULL, "file_ctl_open alloc fp->m_data failed");
            memset((unsigned char *)fp->m_data, 0, sizeof(buf_t));
        }

        unsigned int len =
            min_unsigned(FS_HEADER_FILENAME_SIZE - 1, strlen(filename));
        memcpy((unsigned char *)fp->m_filename, (unsigned char *)filename, len);
        fp->m_filename[len] = '\0';
        return i;
    }

    return -1;
}

int file_ctl_read(int fd, unsigned char *buf, unsigned int len,
                  unsigned int pos) {
    if (fd < 0 || fd >= FILE_DESCRIPTOR_MAX)
        return -1;

    if (!_file_ctl_is_open_file(fd))
        return -1;

    file_descriptor_t *fp = &g_file_ctl.m_des[fd];

    if (fp->m_data->m_data == NULL || fp->m_data->m_size == 0 ||
        pos >= fp->m_data->m_size)
        return 0;

    unsigned int rlen = min_unsigned(fp->m_data->m_size - pos, len);
    memcpy(buf, fp->m_data->m_data + pos, rlen);
    return (int)rlen;
}

int file_ctl_write(int fd, unsigned char *buf, unsigned int len,
                   unsigned int pos) {
    if (fd < 0 || fd >= FILE_DESCRIPTOR_MAX)
        return -1;

    if (!_file_ctl_is_open_file(fd) || pos + len > FILE_MAX_WIRTE_LEN)
        return -1;

    file_descriptor_t *fp = &g_file_ctl.m_des[fd];

    if (fp->m_data->m_data == NULL) {
        fp->m_data->m_data = memman_alloc_4k(pos + len);
        assert(fp->m_data->m_data != NULL, "file_ctl_write alloc len failed");
        fp->m_data->m_size = pos + len;
    } else {
        if (pos >= fp->m_data->m_size)
            pos = fp->m_data->m_size;

        unsigned int wlen = pos + len;
        if (wlen > fp->m_data->m_size) {
            fs_free_buf(fp->m_data);

            fp->m_data->m_data = memman_alloc_4k(wlen);
            assert(fp->m_data->m_data != NULL,
                   "file_ctl_write alloc epos failed");
            fp->m_data->m_size = wlen;
        }
    }

    mempcpy(fp->m_data->m_data + pos, buf, len);
    return (int)len;
}

int file_ctl_file_size(int fd) {
    if (fd < 0 || fd >= FILE_DESCRIPTOR_MAX)
        return -1;

    if (!_file_ctl_is_open_file(fd))
        return -1;

    return (int)g_file_ctl.m_des[fd].m_data->m_size;
}

void file_ctl_close(int fd) {
    if (fd < 0 || fd >= FILE_DESCRIPTOR_MAX)
        return;

    if (!_file_ctl_is_open_file(fd))
        return;

    file_descriptor_t *fp = &g_file_ctl.m_des[fd];
    if (fp->m_ref <= 0)
        return;

    fp->m_ref--;

    if (fp->m_ref == 0 && fp->m_data) {
        fs_free_buf(fp->m_data);
        fp->m_data = NULL;
        fp->m_filename[0] = '\0';
    }
}

#ifdef __FILE_CTL_TEST__
void file_ctl_test(void) {
    unsigned char buf[16];
    const char *test_str = "1234567890";
    const char *filename = "foo.bar";

    // 测试创建一个文件
    int rlen_1 = file_ctl_read(0, buf, 10, 0);
    assert(rlen_1 == -1, "rlen_1 != -1");

    int fd = file_ctl_open(filename);
    assert(fd >= 0, "fd < 0");

    int fsize = file_ctl_file_size(fd);
    assert(fsize == 0, "fsize != 0");

    int rlen_2 = file_ctl_read(fd, buf, 16, 0);
    assert(rlen_2 == 0, "rlen_2 != 0");

    int wlen = file_ctl_write(fd, (unsigned char *)test_str, 10, 0);
    assert(wlen == 10, "wlen != 10");

    memset(buf, 0, sizeof(buf));
    int rlen_3 = file_ctl_read(fd, buf, 10, 0);
    assert(rlen_3 == 10, "rlen_3 != 10");
    assert(strcmp((char *)buf, test_str) == 0, "buf != test_str");

    file_ctl_close(fd);

    int rlen_4 = file_ctl_read(fd, buf, 10, 0);
    assert(rlen_4 == -1, "rlen_4 != -1");

    // 测试读取文件系统中的文件
    const char *filename_1 = "a.txt";
    int fd_1 = file_ctl_open(filename_1);
    assert(fd_1 >= 0, "fd < 0");

    int fsize_1 = file_ctl_file_size(fd_1);
    assert(fsize_1 > 0, "fsize_1 <= 0");

    memset(buf, 0, sizeof(buf));
    int rlen_5 = file_ctl_read(fd_1, buf, 4, 1);
    assert(rlen_5 == 4, "rlen_5 != 4");
    assert(strcmp((char *)buf, "ello") == 0, "buf != ello");

    file_ctl_close(fd_1);
}
#endif
