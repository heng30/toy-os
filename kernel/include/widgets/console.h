#pragma once

#include "def.h"
#include "fs_reader.h"
#include "multi_task.h"
#include "window.h"
#include "wuitl.h"

#define CONSOLE_TEXT_MAX_LEN 1024
#define CONSOLE_CMD_DS_SIZE (128 * KB)

typedef struct {
    window_t *m_win;
    char m_text[CONSOLE_TEXT_MAX_LEN]; // 输入的字符串
    pos_t m_cursor_pos;                // 输入光标位置
    buf_t *m_cmd;                      // 保存外部命令的代码缓冲区

    // 外部命令的ds寄存器执行的内存，为了隔离内核和外部命令之间的内存
    unsigned char *m_cmd_ds;
} console_t;

// 分配对象
console_t *console_new(unsigned int x, unsigned int y, unsigned int width,
                       unsigned int height, const char *title);

// 释放对象
void console_free(const console_t *p);

// 获取关注点
void console_focus(console_t *p);

// 窗口可用, 从外部命令返回后需要调用该函数
void console_enable(console_t *p);

// 窗口不可用, 调用外部命令时终端处于不可用状态
void console_disable(console_t *p);

// 获取唯一的终端对象
console_t *console_get(void);

// 显示对象
void console_show(console_t *p, int z);

// 隐藏对象
void console_hide(console_t *p);

// 绘制字符串,结尾不要有回车符
void console_draw_text(console_t *p, const char *text);

// 绘制一个字符
void console_draw_ch(console_t *p, const char ch);

// 移动到下一行
void console_move_to_next_line(console_t *p);

// 添加一个字符
void console_push(console_t *p, char c);

// 移除一个字符
void console_pop(console_t *p);

// 移动窗口回调函数
void console_moving(void *p);

// 终端任务
task_t *init_console_task(void);

// 清空屏幕
void console_input_area_clear_all(console_t *p);
