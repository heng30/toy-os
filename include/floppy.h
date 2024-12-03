#pragma once

#define SECTOR_SIZE 512 // 扇区大小
#define FLOPPY_DISK_COUNT 1 // 磁盘数量，一个磁盘两个面，都可以存数据

typedef enum FLOPPY_DISK_MAGNETIC_HEAD {
    MAGNETIC_HEAD_0 = 0,
    MAGNETIC_HEAD_1 = 1,
} floppy_disk_magnetic_head_t;

typedef enum FLOPPY_DISK_ERROR {
    FLOPPY_DISK_ERROR_NONE,
    FLOPPY_DISK_ERROR_DISK,
    FLOPPY_DISK_ERROR_CYLINDER,
    FLOPPY_DISK_ERROR_SECTOR,
} floppy_disk_error_t;

typedef struct floppy_disk {
    // 每个成员为1个盘面，2个盘面组成一个磁盘
    unsigned char *m_data[FLOPPY_DISK_COUNT][2];

    // 设置要读取的扇区位置
    unsigned int m_disk;                // 第几个磁盘,从0开始计数。汇编从0开始计数
    floppy_disk_magnetic_head_t m_head; // 磁头位置, 0/1
    unsigned int m_cylinder;            // 第几个柱面,从0开始计数。汇编从0开始计数
    unsigned int m_sector;              // 第几个扇区,从0开始计数。汇编从1开始计数
} floppy_disk_t;

// 初始化软盘
void floppy_disk_init(void);

// 设置要读取的位置
floppy_disk_error_t floppy_disk_set_disk(unsigned int disk);
void floppy_disk_set_head(enum FLOPPY_DISK_MAGNETIC_HEAD head);
floppy_disk_error_t floppy_disk_set_cylinder(unsigned int cylinder);
floppy_disk_error_t floppy_disk_set_sector(unsigned int sector);

floppy_disk_error_t floppy_disk_set_pos(floppy_disk_magnetic_head_t head,
                                        unsigned int disk,
                                        unsigned int cylinder,
                                        unsigned int sector);

// 读取扇区
void floppy_disk_read_sector(unsigned char buf[SECTOR_SIZE]);

// 写入扇区
void floppy_disk_write_sector(const unsigned char buf[SECTOR_SIZE]);

// 创建一个软盘镜像文件
void floppy_disk_make(const char *disk_file);

// 打印出错误信息
void floppy_disk_error_display(floppy_disk_error_t error);

// 打印出当前软盘信息
void floppy_disk_display();

#ifdef __TEST__
void floppy_disk_test(void);
#endif
