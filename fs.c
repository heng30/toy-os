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

#define FS_HEADER_TYPE_FILE 0 // 文件
#define FS_HEADER_TYPE_DIR 1  // 目录
#define FS_HEADER_TYPE_END 2  // 头结果结束标志

// 将一个文件夹里的所有一级目录文件制作成一个文件系统
buf_t mk_fs(const char *data_dir) {
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

    unsigned int header_size =
        file_count * (unsigned int)sizeof(fs_header_t) +
        (unsigned int)sizeof(fs_header_t); // 加上一个结束头
    unsigned int header_sector =
        header_size / SECTOR_SIZE + (header_size % SECTOR_SIZE == 0 ? 0 : 1);
    total_sector_count = header_sector + data_sector_count;

    buf.m_size = total_sector_count * SECTOR_SIZE;
    buf.m_data = (unsigned char *)malloc(buf.m_size);
    assert(buf.m_data);

    fs_header_t *fh = (fs_header_t *)buf.m_data;
    unsigned char *ds = buf.m_data + header_size;
    unsigned short clustno = (unsigned short)header_sector;

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
        size_t rb = fread(file_buf, 1, (size_t)finfo.st_size, fp);
        assert(rb == (size_t)finfo.st_size);
        fclose(fp);

        // 计算文件占用的扇区数量
        unsigned int one_file_data_sector_count =
            (unsigned int)(finfo.st_size / SECTOR_SIZE +
                           (finfo.st_size % SECTOR_SIZE == 0 ? 0 : 1));
        unsigned int one_file_data_size =
            one_file_data_sector_count * SECTOR_SIZE;
        clustno += (unsigned short)one_file_data_sector_count;

        // 设置头结构
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

        fh++, ds +=
              one_file_data_size; // 数据指针移动到下一个文件写入的起始扇区位置

        assert(data_sector_count >= one_file_data_sector_count);
        file_count--, data_sector_count -= one_file_data_sector_count;

        // 写入最后1个结束头
        if (file_count == 0)
            fh->m_type = FS_HEADER_TYPE_END;

        debug("filename: %s, size: %lld bytes", entry->d_name, finfo.st_size);
    }
    closedir(dir);

    assert(file_count == 0), assert(data_sector_count == 0);

    return buf;
}

unsigned char *fs_read(unsigned char *start_addr, const char *filename) {
    fs_header_t *p = (fs_header_t *)start_addr;
    char fname[sizeof(p->m_name) + sizeof(p->m_ext) + 2] = {0};

    for (; p->m_type != FS_HEADER_TYPE_END; p++) {
        unsigned int i = 0;
        for (; i < sizeof(p->m_name); i++)
            fname[i] = p->m_name[i];

        fname[i] = '.';
        i++;

        for (unsigned int j = 0; j < sizeof(p->m_ext); j++)
            fname[i + j] = p->m_ext[j];

        if (strcmp(filename, fname))
            continue;

        unsigned char *data = (unsigned char *)malloc(p->m_size);
        if (!data)
            return NULL;

        unsigned char *data_ptr = start_addr + p->m_clustno * SECTOR_SIZE;
        memcpy(data, data_ptr, p->m_size);
        return data;
    }

    return NULL;
}

// TODO
void fs_test(void) {

}
