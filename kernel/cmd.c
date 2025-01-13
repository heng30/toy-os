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

static void _after_cmd_exe(console_t *console, const char *filename,
                           window_t *win) {
    if (!strcmp(filename, "dch.exe") || !strcmp(filename, "dtext.exe")) {
        console_move_to_next_line(console);
    }

    // 使用Ctrl+K强制杀死窗口
    if (win && win->m_id == WINDOW_ID_USER) {
        window_ctl_close_window(win);
    }

    // 销毁所有等待被销毁的窗口
    window_ctl_close_all_waiting_window();

    console_close_all_open_files(console);
}

void cmd_exe(console_t *console) {
    const char *filename = str_trim_space(console->m_text);
    task_t *task = console->m_win->m_task;

    console->m_cmd = fs_read(filename);
    if (!console->m_cmd)
        return;

    console_disable(console);

    // 设置外部命令调用的代码段描述符
    set_segmdesc(&task->m_ldt[0], 0xfffff, (ptr_t)console->m_cmd->m_data,
                 AR_FUNCTION + AR_RING_3);

    // 设置外部命令调用的数据段描述符，为了与内核的数据段进行隔离
    set_segmdesc(&task->m_ldt[1], CONSOLE_CMD_DS_SIZE - 1,
                 (ptr_t)console->m_cmd_ds, AR_FUNCTION_DS + AR_RING_3);

    // 跳转到外部程序代码并执行. 参数: eip, cs, esp, ds, esp0
    // 加4是要将第3位置为1, 启用局部描述符表
    // 因为在局部描述符表中第1位为代码段描述符，第2位为数据段描述符
    // 所以这里的代码段描述符下标为0, 数据段描述符下标为1
    start_cmd(0, 0 * 8 + 4, CONSOLE_CMD_DS_SIZE, 1 * 8 + 4,
              &g_multi_task_ctl->m_current_task->m_tss.m_esp0);

    window_t *fouce_win = g_window_ctl.m_focus_window;
    console_enable(console);
    _after_cmd_exe(console, filename, fouce_win);

    fs_free_buf(console->m_cmd);
    console->m_cmd = NULL;
}

void cmd_kill_process(void) {
    console_t *p = console_get();
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
