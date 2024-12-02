#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "floppy.h"
#include "logger.h"

#define SECTOR_COUNT 18   // 一个柱面的扇区数量
#define CYLINDER_COUNT 80 // 一个盘面的柱面数量

#define CYLINDER_SIZE (SECTOR_COUNT * SECTOR_SIZE)      // 一个柱面的大小
#define HALF_DISK_SIZE (CYLINDER_COUNT * CYLINDER_SIZE) // 一个盘面的大小

floppy_disk_t g_floppy_disk;

void floppy_disk_init(void) {
    memset(&g_floppy_disk, 0, sizeof(g_floppy_disk));

    for (int i = 0; i < FLOPPY_DISK_COUNT; i++) {
        g_floppy_disk.m_data[i][0] = (unsigned char *)malloc(HALF_DISK_SIZE);
        g_floppy_disk.m_data[i][1] = (unsigned char *)malloc(HALF_DISK_SIZE);
        assert(g_floppy_disk.m_data[i][0]);
        assert(g_floppy_disk.m_data[i][1]);
    }
}

floppy_disk_error_t floppy_disk_set_disk(unsigned int disk) {
    if (disk >= FLOPPY_DISK_COUNT)
        return FLOPPY_DISK_ERROR_DISK;

    g_floppy_disk.m_disk = disk;
    return FLOPPY_DISK_ERROR_NONE;
}

void floppy_disk_set_head(floppy_disk_magnetic_head_t head) {
    g_floppy_disk.m_head = head;
}

floppy_disk_error_t floppy_disk_set_cylinder(unsigned int cylinder) {
    if (cylinder >= CYLINDER_COUNT)
        return FLOPPY_DISK_ERROR_CYLINDER;

    g_floppy_disk.m_cylinder = cylinder;
    return FLOPPY_DISK_ERROR_NONE;
}

floppy_disk_error_t floppy_disk_set_sector(unsigned int sector) {
    if (sector >= SECTOR_COUNT)
        return FLOPPY_DISK_ERROR_SECTOR;

    g_floppy_disk.m_sector = sector;
    return FLOPPY_DISK_ERROR_NONE;
}

floppy_disk_error_t floppy_disk_set_pos(unsigned int disk,
                                        floppy_disk_magnetic_head_t head,
                                        unsigned int cylinder,
                                        unsigned int sector) {
    floppy_disk_error_t error = FLOPPY_DISK_ERROR_NONE;

    error = floppy_disk_set_disk(disk);
    if (error != FLOPPY_DISK_ERROR_NONE)
        return error;

    error = floppy_disk_set_cylinder(cylinder);
    if (error != FLOPPY_DISK_ERROR_NONE)
        return error;

    error = floppy_disk_set_sector(sector);
    if (error != FLOPPY_DISK_ERROR_NONE)
        return error;

    floppy_disk_set_head(head);
    return FLOPPY_DISK_ERROR_NONE;
}

void floppy_disk_read_sector(unsigned char buf[SECTOR_SIZE]) {
    unsigned int pos = g_floppy_disk.m_cylinder * CYLINDER_SIZE +
                       g_floppy_disk.m_sector * SECTOR_SIZE;

    unsigned char *sector_bytes_offset =
        &g_floppy_disk.m_data[g_floppy_disk.m_disk][g_floppy_disk.m_head][pos];

    memcpy(buf, sector_bytes_offset, SECTOR_SIZE);
}

void floppy_disk_write_sector(const unsigned char buf[SECTOR_SIZE]) {
    unsigned int pos = g_floppy_disk.m_cylinder * CYLINDER_SIZE +
                       g_floppy_disk.m_sector * SECTOR_SIZE;

    unsigned char *sector_bytes_offset =
        &g_floppy_disk.m_data[g_floppy_disk.m_disk][g_floppy_disk.m_head][pos];

    memcpy(sector_bytes_offset, buf, SECTOR_SIZE);
}

void floppy_disk_error_display(floppy_disk_error_t error) {
    switch (error) {
    FLOPPY_DISK_ERROR_DISK:
        debug("Invalid disk number");
        break;
    FLOPPY_DISK_ERROR_SECTOR:
        debug("Invalid sector number");
        break;
    FLOPPY_DISK_ERROR_CYLINDER:
        debug("Invalid cylinder number");
        break;
    default:
        debug("No error");
    }
}

void floppy_disk_display() {
    debug("disk cout: %d, disk: %d, head: %d, cylinder: %d, sector: %d",
          FLOPPY_DISK_COUNT, g_floppy_disk.m_disk, g_floppy_disk.m_head,
          g_floppy_disk.m_cylinder, g_floppy_disk.m_sector);
}

#ifdef __TEST__
#include <unistd.h>

#include "util.h"

void floppy_disk_test(void) {
    const unsigned char src[SECTOR_SIZE] = "hello world!";
    unsigned char dst[SECTOR_SIZE] = {0};
    floppy_disk_init();

    for (int i = 0; i < 1000; i++) {
        unsigned int disk = 0;
        floppy_disk_magnetic_head_t head = i % 2;
        unsigned int cylinder = rand_num(0, CYLINDER_COUNT - 1, i);
        unsigned int sector = rand_num(0, SECTOR_COUNT - 1, i);

        assert(floppy_disk_set_pos(disk, head, cylinder, sector) ==
               FLOPPY_DISK_ERROR_NONE);

        // floppy_disk_display();

        floppy_disk_write_sector(src);
        memset(dst, 0, sizeof(dst));
        floppy_disk_read_sector(dst);

        assert(!strcmp(src, dst));
        usleep(1 * 1000);
    }

    debug("floppy_disk_test() Ok");
}
#endif
