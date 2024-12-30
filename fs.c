#include <assert.h>
#include <dirent.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fs.h"
#include "logger.h"
#include "util.h"

// 将一个文件夹里的所有一级目录文件制作成一个文件系统
buf_t mk_fs(const char *data_dir) {
    assert(is_little_endian());

    char fpath[4096];
    struct stat finfo;
    struct dirent *entry = NULL;
    buf_t buf = {.m_data = NULL, .m_size = 0};
    unsigned int total_sector_count = 0, file_count = 0, data_sector_count = 0;

    DIR *dir = opendir(data_dir);
    assert(dir != NULL);

    // 获取文件数量和存储文件内容需要的扇区数
    while ((entry = readdir(dir)) != NULL) {
        // 跳过目录和隐藏文件
        if (entry->d_type == DT_DIR || entry->d_name[0] == '.')
            continue;

        snprintf(fpath, sizeof(fpath), "%s/%s", data_dir, entry->d_name);
        assert(access(fpath, F_OK) != -1); // 检查是否有权限访问文件

        assert(stat(fpath, &finfo) == 0);
        data_sector_count +=
            (unsigned int)(finfo.st_size / SECTOR_SIZE +
                           (finfo.st_size % SECTOR_SIZE == 0 ? 0 : 1));
        file_count++;
    }
    closedir(dir);

    // 计算头表大小和需要的扇区数
    unsigned int header_size =
        file_count * (unsigned int)sizeof(fs_header_t) +
        (unsigned int)sizeof(fs_header_t); // 加上一个结束头
    unsigned int header_sector_count =
        header_size / SECTOR_SIZE + (header_size % SECTOR_SIZE == 0 ? 0 : 1);
    total_sector_count = header_sector_count + data_sector_count;

    // 设置文件系统缓冲区和大小
    buf.m_size = total_sector_count * SECTOR_SIZE;
    buf.m_data = (unsigned char *)malloc(buf.m_size);
    assert(buf.m_data);

    // 为制作文件系统做准备，设置数据写入位置和变动数据
    fs_header_t *fh = (fs_header_t *)buf.m_data;
    unsigned char *ds = buf.m_data + header_sector_count * SECTOR_SIZE;
    unsigned short clustno = (unsigned short)header_sector_count;

    // 构建头结构和复制文件数据
    dir = opendir(data_dir);
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR || entry->d_name[0] == '.')
            continue;

        snprintf(fpath, sizeof(fpath), "%s/%s", data_dir, entry->d_name);
        assert(access(fpath, F_OK) != -1); // 检查是否有权限访问文件
        assert(stat(fpath, &finfo) == 0);

        // 读取文件内容
        FILE *fp = fopen(fpath, "rb");
        assert(fp);

        unsigned char *file_buf =
            (unsigned char *)malloc((size_t)finfo.st_size);
        assert(file_buf);

        size_t rb = fread(file_buf, 1, (size_t)finfo.st_size, fp);
        assert(rb == (size_t)finfo.st_size);
        fclose(fp);

        // 计算文件占用的扇区数量
        unsigned int one_file_data_sector_count =
            (unsigned int)(finfo.st_size / SECTOR_SIZE +
                           (finfo.st_size % SECTOR_SIZE == 0 ? 0 : 1));
        unsigned int one_file_data_size =
            one_file_data_sector_count * SECTOR_SIZE;

        // 初始化头结构
        memset(fh, 0, sizeof(fs_header_t));

        // 复制文件名, 不包括扩展名
        for (unsigned int i = 0; i < sizeof(fh->m_name); i++) {
            if (entry->d_name[i] == '.')
                break;
            fh->m_name[i] = entry->d_name[i];
        }

        // 复制扩展名
        bool is_find_dot = false;
        for (unsigned int i = 0; i < strlen(entry->d_name); i++) {
            if (!is_find_dot && entry->d_name[i] != '.')
                continue;

            // 跳过'.'分隔符
            i++, is_find_dot = true;
            for (unsigned int j = 0;
                 j < sizeof(fh->m_ext) && i < strlen(entry->d_name); j++, i++) {
                fh->m_ext[j] = entry->d_name[i];
            }

            break;
        }

        fh->m_size = (unsigned int)finfo.st_size;
        fh->m_type = FS_HEADER_TYPE_FILE;
        fh->m_clustno = clustno;
        memcpy(ds, file_buf, rb);
        free(file_buf);

        // 更新写入位置
        fh++, ds += one_file_data_size;
        clustno += (unsigned short)one_file_data_sector_count;

        assert(data_sector_count >= one_file_data_sector_count);
        file_count--, data_sector_count -= one_file_data_sector_count;

        // 写入最后1个结束头
        if (file_count == 0)
            fh->m_type = FS_HEADER_TYPE_END;

        // debug("filename: %s, size: %lld bytes", entry->d_name, finfo.st_size);
    }
    closedir(dir);

    assert(file_count == 0), assert(data_sector_count == 0);

    return buf;
}

unsigned char *fs_read(unsigned char *start_addr, const char *filename) {
    char fname[FS_HEADER_FILENAME_SIZE];
    fs_header_t *p = (fs_header_t *)start_addr;

    for (; p->m_type != FS_HEADER_TYPE_END; p++) {
        get_filename(p, fname, FS_HEADER_FILENAME_SIZE);

        if (strcmp(filename, fname))
            continue;

        unsigned char *buf = (unsigned char *)malloc(p->m_size + 1);
        if (!buf)
            return NULL;

        unsigned char *data_ptr = start_addr + p->m_clustno * SECTOR_SIZE;
        memcpy(buf, data_ptr, p->m_size);
        buf[p->m_size] = '\0';
        return buf;
    }

    return NULL;
}

void get_filename(fs_header_t *p, char *buf, unsigned int size) {
    assert(size >= FS_HEADER_FILENAME_SIZE);

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

    debug("filename: %s", fname);
    debug("clustno: %d", p->m_clustno);
    debug("flag: %d", p->m_type);
    debug("size: %d", p->m_size);
}

#ifdef __TEST__
void fs_test(void) {
    char fname[FS_HEADER_FILENAME_SIZE];
    buf_t buf = mk_fs("./res/fs-data");
    assert(buf.m_data && buf.m_size > 0);

    for (fs_header_t *p = (fs_header_t *)buf.m_data;
         p->m_type != FS_HEADER_TYPE_END; p++) {
        debug("========================");
        fs_header_display(p);

        get_filename(p, fname, FS_HEADER_FILENAME_SIZE);
        unsigned char *data = fs_read(buf.m_data, fname);
        debug("%s", (char *)data);
    }
}
#endif
