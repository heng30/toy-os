#pragma once
#include "def.h"

/*
 * 这时一个类似于fat32的文件系统，不过为了方便会简化很多实现。
 * 整个文件系统的结构是:
 *  - fs_header_t_1...fs_header_t_n|fs_data
 *  -
 * 文件系统头表必须占满1个扇区的大小，即数据的起始位置必须是文件系统头表的下1个扇区.
 *  - 每个文件必须独占一整个扇区，即一个扇区不能同时存储超过1个文件
 */
typedef struct {
    char m_name[8];     // 文件名
    char m_ext[3];      // 文件扩展名
    unsigned char m_type;        // 文件类型
    unsigned char m_reserve[10]; // 保留
    unsigned short m_time;       // 时间
    unsigned short m_date;       // 日期
    unsigned short m_clustno; // 簇号，从文件系统开始的偏移扇区
    unsigned int m_size;      // 文件大小
} fs_header_t;

// 将一个文件夹里的所有一级目录文件制作成一个文件系统
buf_t mk_fs(const char *data_dir);

// 读取文件内容
unsigned char *fs_read(unsigned char *start_addr, const char *filename);

// 测试
void fs_test(void);
