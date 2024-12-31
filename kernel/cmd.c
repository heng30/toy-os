#include "cmd.h"
#include "def.h"
#include "fs_reader.h"
#include "kutil.h"
#include "memory.h"

void cmd_ls(console_t *console) {
    bool have_file = false;
    fs_header_t *p = (fs_header_t *)FS_START_ADDR;
    char *fname = (char *)memman_alloc_4k(FS_HEADER_FILENAME_SIZE);
    assert(fname != NULL, "fs_read alloc fname buffer error");

    for (; p->m_type != FS_HEADER_TYPE_END; p++) {
        get_filename(p, fname, FS_HEADER_FILENAME_SIZE);
        console_draw_text(console, fname);
        console_draw_ch(console, ' ');
        have_file = true;
    }

    if (have_file)
        console_move_to_next_line(console);

    memman_free_4k(fname, FS_HEADER_FILENAME_SIZE);
}
