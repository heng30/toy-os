#pragma once

#include "def.h"
#include "fs_reader.h"

#define FILE_DESCRIPTOR_MAX 256
#define FILE_MAX_WIRTE_LEN (1 * MB)

typedef struct {
    int m_ref;                                // 文件引用次数
    char m_filename[FS_HEADER_FILENAME_SIZE]; // 文件名
    buf_t *m_data;                            // 文件数据
} file_descriptor_t;

typedef struct {
    file_descriptor_t *m_des;
} file_ctl_t;

// 初始化文件描述表
void init_file_ctl(void);

// 销毁管理器
void destory_file_ctl(void);

// 打开文件
int file_ctl_open(const char *filename);

// 读取文件
// buf: 存放内容的缓冲区
// len: 读取数据量
// pos：开始读取的位置, 从0开始
int file_ctl_read(int fd, unsigned char *buf, unsigned int len,
                  unsigned int pos);

// 写入文件
// buf: 存放内容的缓冲区
// len: 写入数据量
// pos：开始写入的位置, 从0开始
int file_ctl_write(int fd, unsigned char *buf, unsigned int len,
                   unsigned int pos);

// 获取文件大小
int file_ctl_file_size(int fd);

// 关闭文件
void file_ctl_close(int fd);

// 测试
#ifdef __FILE_CTL_TEST__
void file_ctl_test(void);
#endif
