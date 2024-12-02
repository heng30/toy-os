#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "util.h"
#include "floppy.h"

#define BOOT_SIZE 512
#define DISK_SIZE (1.5 * 1024 * 1024)

static char BOOT_IMAGE[BOOT_SIZE] = {
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
    char *disk = malloc(DISK_SIZE);
    assert(disk);

    FILE *fp = fopen(disk_file, "wb");
    assert(fp);

    // copy boot.img to disk.img
    memcpy(disk, BOOT_IMAGE, BOOT_SIZE);

    disk[BOOT_SIZE] = 0xfa;
    disk[BOOT_SIZE + 1] = 0xff;
    disk[BOOT_SIZE + 2] = 0xff;

    int wb = fwrite(disk, 1, DISK_SIZE, fp);
    assert(wb == DISK_SIZE);
    fclose(fp);
    free(disk);

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
    logger_test();
    util_test();
    floppy_disk_test();
}
#endif
