#include "cmd.h"
#include "def.h"
#include "fs_reader.h"
#include "input_cursor.h"
#include "io.h"
#include "kutil.h"
#include "memory.h"
#include "multi_task.h"
#include "string.h"

void cmd_cls(console_t *p) { console_input_area_clear_all(p); }

void cmd_mem(console_t *console) {
    char buf[32];
    unsigned int total = memman_total() / MB;
    uint_to_string(total, buf);
    console_draw_text(console, "total memory: ");
    console_draw_text(console, buf);
    console_draw_text(console, "MB");
    console_move_to_next_line(console);
}

void cmd_free(console_t *console) {
    char buf[32];
    unsigned int total = memman_available() / MB;
    uint_to_string(total, buf);
    console_draw_text(console, "free memory: ");
    console_draw_text(console, buf);
    console_draw_text(console, "MB");
    console_move_to_next_line(console);
}

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
        goto end;

    // 移除文件最后的'.'号
    if (buf->m_size >= 2 && buf->m_data[buf->m_size - 2] == 0x0a) {
        buf->m_data[buf->m_size - 2] = '\0';
    }

    console_draw_text(console, (const char *)buf->m_data);
    fs_free_buf(buf);

end:
    console_move_to_next_line(console);
}

static void _after_cmd_exe(console_t *console, const char *filename) {
    if (!strcmp(filename, "rhlt.exe")) {
        const char *text = (const char *)EXTERNAL_BIN_AND_KERNEL_SHARED_MEMORY;
        unsigned int len = strlen(text);

        if (len > 32) {
            console_draw_text(console, "error from rhlt.exe");
        } else {
            console_draw_text(console, text);
        }
        console_move_to_next_line(console);
    }
}

void cmd_exe(console_t *console) {
    const char *clean_filename = str_trim_space(console->m_text);
    unsigned int fsize = strlen(clean_filename) + 1;

    char *filename = (char *)memman_alloc_4k(fsize);
    assert(filename != NULL, "cmd_exe alloc for filename error");
    strdup(filename, clean_filename);

    unsigned short func_tr = 6; // 要和kernel.asm中的调用外部程序描述符一致

    buf_t *buf = fs_read(filename);
    if (!buf)
        return;

    console_disable(console);

    segment_descriptor_t *gdt = (segment_descriptor_t *)get_addr_gdt();
    set_segmdesc(gdt + func_tr, 0xfffff, (ptr_t)buf->m_data, AR_FUNCTION);
    farjmp(0, func_tr << 3); // 跳转到外部程序代码并执行

    console_enable(console);

    _after_cmd_exe(console, filename);

    memman_free_4k(filename, fsize);
    fs_free_buf(buf);
}