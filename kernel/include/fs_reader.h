#pragma once

#define ORG_ADDR 0x8000 // 这个地址要和kernel.asm中定义的保持一致
#define SECTOR_SIZE 512 // 扇区大小
#define SECTOR_COUNT_PER_CYLINDER 18 // 一个柱面的扇区数量

// 文件系统开始的柱面号，这个值要和制作镜像文件时保存一致
#define FS_START_CYLINDER 11

// 文件系统在内存中的位置
// `FS_START_CYLINDER - 1` 的原因是第0柱面是启动扇区柱面
// 内核代码从第1柱面开始写入镜像文件
// 启动代码加载内核和文件系统到ORG_ADDR处
#define FS_START_ADDR                                                          \
    (ORG_ADDR +                                                                \
     (FS_START_CYLINDER - 1) * SECTOR_COUNT_PER_CYLINDER * SECTOR_SIZE)

#define FS_HEADER_RESERVE_SIZE 10
#define FS_HEADER_NAME_SIZE 8
#define FS_HEADER_EXT_SIZE 3
#define FS_HEADER_FILENAME_SIZE (FS_HEADER_NAME_SIZE + FS_HEADER_EXT_SIZE + 2)

#define FS_HEADER_TYPE_FILE 0 // 文件
#define FS_HEADER_TYPE_DIR 1  // 目录
#define FS_HEADER_TYPE_END 2  // 头结果结束标志

typedef struct {
    unsigned char *m_data; // 数据
    unsigned int m_size;   // 大小
} buf_t;

/*
 * 这时一个类似于fat32的文件系统，不过为了方便会简化很多实现。
 * 整个文件系统的结构是:
 *  - fs_header_t_1...fs_header_t_n|fs_data
 *  -
 * 文件系统头表必须占满1个扇区的大小，即数据的起始位置必须是文件系统头表的下1个扇区.
 *  - 每个文件必须独占一整个扇区，即一个扇区不能同时存储超过1个文件
 */
typedef struct {
    char m_name[FS_HEADER_NAME_SIZE];                // 文件名
    char m_ext[FS_HEADER_EXT_SIZE];                  // 文件扩展名
    unsigned char m_type;                            // 文件类型
    unsigned char m_reserve[FS_HEADER_RESERVE_SIZE]; // 保留
    unsigned short m_time;                           // 时间
    unsigned short m_date;                           // 日期
    unsigned short m_clustno; // 簇号，从文件系统开始位置偏移的扇区数
    unsigned int m_size; // 文件大小
} fs_header_t;

// 读取文件内容
buf_t *fs_read(const char *filename);

// 释放内存
void fs_free_buf(buf_t *p);

// 获取文件名
void get_filename(fs_header_t *p, char *buf, unsigned int size);
