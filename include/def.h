#pragma once

#define SECTOR_COUNT 18   // 一个柱面的扇区数量
#define CYLINDER_COUNT 80 // 一个盘面的柱面数量
#define FLOPPY_DISK_COUNT 1 // 磁盘数量，一个磁盘两个面，都可以存数据

#define SECTOR_SIZE 512 // 扇区大小
#define CYLINDER_SIZE (SECTOR_COUNT * SECTOR_SIZE)      // 一个柱面的大小



typedef struct {
    unsigned char *m_data; // 数据
    unsigned int m_size;  // 大小
} buf_t;

