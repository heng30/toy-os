#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "def.h"
#include "floppy.h"
#include "fs.h"
#include "logger.h"
#include "mk_font.h"
#include "util.h"

/* 整个软盘的布局：
 *  - 0号柱面1号扇区存放启动程序
 *  - 1-10号柱面存放内核程序
 *  - 11号柱面及以后存放文件系统
 *
 * 如果内核大小超过10个柱面需要修改FS_START_CYLINDER和其他相关参数。
 */

#define BOOT_SIZE 512
#define KERNEL_START_CYLINDER 1 // 从第几个柱面号开始写入内核
#define FS_START_CYLINDER 11    // 从第几个柱面号开始写入文件系统

static unsigned char BOOT_IMAGE[BOOT_SIZE] = {
    [0 ...(BOOT_SIZE - 3)] = 0,
    [BOOT_SIZE - 2] = 0x55,
    [BOOT_SIZE - 1] = 0xaa,
};

static unsigned char *KERNEL_IMAGE = NULL;
static unsigned int KERNEL_SIZE = 0;

static void _load_boot(char *boot_file) {
    FILE *fp = fopen(boot_file, "rb");
    assert(fp);

    size_t rb = fread(BOOT_IMAGE, 1, BOOT_SIZE - 2, fp);
    assert(rb > 0);

    // make sure the boot image is small than 510 bytes
    assert(feof(fp));
    fclose(fp);

    debug("boot image size is %d bytes, and it will be written in the sector 1 "
          "of "
          "cylinder 0",
          rb);
}

static void _load_kernel(char *kernel_file) {
    long fsize = file_size(kernel_file);
    assert(fsize > 0 && fsize < UINT_MAX);

    unsigned int sector_count = (unsigned int)fsize / SECTOR_SIZE;
    if (fsize % SECTOR_SIZE != 0)
        sector_count++;

    debug("kernel image size is %ld bytes about %d sectors", fsize,
          sector_count);

    KERNEL_SIZE = sector_count * SECTOR_SIZE;
    KERNEL_IMAGE = (unsigned char *)malloc(KERNEL_SIZE);
    assert(KERNEL_IMAGE);

    FILE *fp = fopen(kernel_file, "rb");
    assert(fp);

    size_t rb = fread(KERNEL_IMAGE, 1, KERNEL_SIZE, fp);
    assert(rb == (size_t)fsize);

    fclose(fp);
}

static void _mk_disk(char *disk_file) {
    floppy_disk_init();

    // 写入引导扇区
    floppy_disk_set_pos(MAGNETIC_HEAD_0, 0, 0, 0);
    floppy_disk_write_sector(BOOT_IMAGE);

    debug("finish writing boot image in the sector 1 of cylinder 0");

    // 写入kernel到一个柱面
    unsigned int sector_count = KERNEL_SIZE / SECTOR_SIZE;

    // 从第1柱面第1个扇区开始写入
    for (unsigned int i = 0; i < sector_count; i++) {
        unsigned int cylinder_index = KERNEL_START_CYLINDER + i / SECTOR_COUNT;
        unsigned int sector_index = i % SECTOR_COUNT;

        floppy_disk_set_pos(MAGNETIC_HEAD_0, 0, cylinder_index, sector_index);
        floppy_disk_write_sector(KERNEL_IMAGE + i * SECTOR_SIZE);

        // debug("write a sector in cylinder %d and sector %d", cylinder_index,
        //       sector_index + 1);
    }

    debug("finish writing kernel in the start of sector 1 of cylinder 1, and "
          "total size is %d cylinders and %d sectors",
          sector_count / SECTOR_COUNT, sector_count % SECTOR_COUNT);

    // 写入文件系统
    buf_t fs_info = mk_fs("./res/fs-data");
    assert(fs_info.m_data && fs_info.m_size > 0);
    assert(fs_info.m_size % SECTOR_SIZE == 0);

    sector_count = fs_info.m_size / SECTOR_SIZE;

    // 从第11柱面第1个扇区开始写入
    for (unsigned int i = 0; i < sector_count; i++) {
        unsigned int cylinder_index = FS_START_CYLINDER + i / SECTOR_COUNT;
        unsigned int sector_index = i % SECTOR_COUNT;

        floppy_disk_set_pos(MAGNETIC_HEAD_0, 0, cylinder_index, sector_index);
        floppy_disk_write_sector(fs_info.m_data + i * SECTOR_SIZE);

        // debug("write a sector in cylinder %d and sector %d", cylinder_index,
        //       sector_index + 1);
    }

    debug("finish writing filesystem in the start of sector 1 of cylinder %d, and "
          "total size is %d cylinders and %d sectors",
          FS_START_CYLINDER, sector_count / SECTOR_COUNT,
          sector_count % SECTOR_COUNT);

    floppy_disk_make(disk_file);

    debug("finish making a floppy image");
}

#ifdef __TEST__
static void _test() {
    // logger_test();
    util_test();
    floppy_disk_test();
    fs_test();
}
#endif

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (!strcmp(argv[1], "--test")) {
#ifdef __TEST__
            _test();
#else
            debug("You should compile with `__TEST__` macro definition to "
                  "run "
                  "`test()` function");
#endif
        } else if (!strcmp(argv[1], "--font")) {
            mk_font("./res/system_font.txt");
        }
        return 0;
    }

    _load_boot("./kernel/build/boot.img");
    _load_kernel("./kernel/build/kernel.img");
    _mk_disk("./build/disk.img");
    return 0;
}
