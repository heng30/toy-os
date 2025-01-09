#include "cmd.h"
#include "def.h"
#include "fs_reader.h"
#include "input_cursor.h"
#include "io.h"
#include "kutil.h"
#include "memory.h"
#include "multi_task.h"
#include "string.h"
#include "timer.h"

#include "widgets/console.h"

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
    if (!strcmp(filename, "dch.exe") || !strcmp(filename, "dtext.exe")) {
        console_move_to_next_line(console);
    }
}

void cmd_exe(console_t *console) {
    const char *clean_filename = str_trim_space(console->m_text);
    unsigned int fsize = strlen(clean_filename) + 1;

    char *filename = (char *)memman_alloc_4k(fsize);
    assert(filename != NULL, "cmd_exe alloc for filename error");
    strdup(filename, clean_filename);

    unsigned short cmd_tr = GDT_CONSOLE_CMD_TR;

    console->m_cmd = fs_read(filename);
    if (!console->m_cmd)
        return;

    console_disable(console);

    segment_descriptor_t *gdt = (segment_descriptor_t *)get_addr_gdt();
    set_segmdesc(gdt + cmd_tr, 0xfffff, (ptr_t)console->m_cmd->m_data,
                 AR_FUNCTION + AR_RING_3);

    // 跳转到外部程序代码并执行. 参数: eip, cs, esp, ds, esp0
    start_cmd(0, cmd_tr << 3, CONSOLE_CMD_DS_SIZE, GDT_CONSOLE_CMD_DS_TR << 3,
              &g_multi_task_ctl->m_current_task->m_tss.m_esp0);

    // 命令运行完毕，重置ss0和esp0
    g_multi_task_ctl->m_current_task->m_tss.m_ss0 = 0;
    g_multi_task_ctl->m_current_task->m_tss.m_esp0 = 0;

    console_enable(console);

    _after_cmd_exe(console, filename);

    memman_free_4k(filename, fsize);
    fs_free_buf(console->m_cmd);
    console->m_cmd = NULL;
}

void cmd_kill_process(void) {
    console_t *p = console_get();
    console_draw_text(p, "Kill Process");
    console_move_to_next_line(p);
    kill_cmd(&p->m_win->m_task->m_tss.m_esp0);
}

ptr_t *int_handler_for_exception(int *esp) {
    console_t *p = console_get();
    console_draw_text(p, "INT 0D, Protected Exception");
    console_move_to_next_line(p);

    return &g_multi_task_ctl->m_current_task->m_tss.m_esp0;
}

ptr_t *int_handler_for_stack_overflow(unsigned int *esp) {
    console_t *p = console_get();
    console_draw_text(p, "INT 0C, Stack Exception");
    console_move_to_next_line(p);

    // 出错代码的位置
    const char *eip = int2hexstr(esp[11]);
    console_draw_text(p, "eip = ");
    console_draw_text(p, eip);
    console_move_to_next_line(p);

    return &g_multi_task_ctl->m_current_task->m_tss.m_esp0;
}
