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
    unsigned int m_disk;                // 第几个磁盘
    floppy_disk_magnetic_head_t m_head; // 磁头位置
    unsigned int m_cylinder;            // 第几个柱面
    unsigned int m_sector;              // 第几个扇区
} floppy_disk_t;

// 初始化软盘
void floppy_disk_init();

// 设置要读取的位置
floppy_disk_error_t floppy_disk_set_disk(unsigned int disk);
void floppy_disk_set_head(enum FLOPPY_DISK_MAGNETIC_HEAD head);
floppy_disk_error_t floppy_disk_set_cylinder(unsigned int cylinder);
floppy_disk_error_t floppy_disk_set_sector(unsigned int sector);

floppy_disk_error_t floppy_disk_set_pos(unsigned int disk,
                                        floppy_disk_magnetic_head_t head,
                                        unsigned int cylinder,
                                        unsigned int sector);

// 读取扇区
void floppy_read_sector(unsigned char buf[SECTOR_SIZE]);

// 写入扇区
void floppy_write_sector(unsigned char buf[SECTOR_SIZE]);
