#include "cmd.h"
#include "def.h"
#include "fs_reader.h"
#include "kutil.h"
#include "memory.h"

void cmd_cls(console_t *p) { console_input_area_clear_all(p); }

void cmd_mem(console_t *console) {}

void cmd_free(console_t *console) {}

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

void cmd_cat(console_t *console) {
    const char *filename = str_trim_space(console->m_text + strlen("cat "));

    buf_t *buf = fs_read(filename);

    if (!buf)
        goto err;

    // 移除文件最后的'.'号
    if (buf->m_size >= 2 && buf->m_data[buf->m_size - 2] == 0x0a) {
        buf->m_data[buf->m_size - 2] = '\0';
    }

    console_draw_text(console, (const char *)buf->m_data);
    fs_free_buf(buf);

err:
    console_move_to_next_line(console);
}
