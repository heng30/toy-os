#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "floppy.h"
#include "logger.h"
#include "util.h"

#define BOOT_SIZE 512
#define DISK_SIZE (1.5 * 1024 * 1024)

static unsigned char BOOT_IMAGE[BOOT_SIZE] = {
    [0 ...(BOOT_SIZE - 3)] = 0,
    [BOOT_SIZE - 2] = 0x55,
    [BOOT_SIZE - 1] = 0xaa,
};

#ifdef __TEST__
static void _test();
#endif

static void _load_boot(char *boot_file) {
    FILE *fp = fopen(boot_file, "rb");
    assert(fp);

    int rb = fread(BOOT_IMAGE, 1, BOOT_SIZE - 2, fp);
    assert(rb > 0);

    // make sure the boot image is small than 510 bytes
    assert(feof(fp));
    fclose(fp);

    debug("boot image: %d bytes", rb);
}

static void _mk_disk(char *disk_file) {
    floppy_disk_init();

    // 写入引导扇区
    floppy_disk_set_pos(MAGNETIC_HEAD_0, 0, 0, 0);
    floppy_disk_write_sector(BOOT_IMAGE);

    // 写入数据到第0号磁盘，第1号柱面，第2个扇区
    unsigned char msg[BOOT_SIZE] = "It's a text from cylinder 1 and sector 2.";
    floppy_disk_set_pos(MAGNETIC_HEAD_0, 0, 1, 1);
    floppy_disk_write_sector(msg);

    floppy_disk_make(disk_file);

    debug("finish making a floppy image");
}

int main(int argc, char *argv[]) {
    if (argc > 1 && !strcmp(argv[1], "--test")) {
#ifdef __TEST__
        _test();
#else
        debug("You should compile with `__TEST__` macro definition to run "
              "`test()` function");
#endif
        return 0;
    }

    _load_boot("./build/boot.img");
    _mk_disk("./build/disk.img");
    return 0;
}

#ifdef __TEST__
static void _test() {
    // logger_test();
    util_test();
    floppy_disk_test();
}
#endif
