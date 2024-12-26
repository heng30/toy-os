#pragma once

#include "def.h"
#include "multi_task.h"
#include "window.h"
#include "wuitl.h"

#define CONSOLE_TEXT_MAX_LEN 1024

typedef struct {
    window_t *m_win;
    char m_text[CONSOLE_TEXT_MAX_LEN]; // 输入的字符串
    pos_t m_cursor_pos;                // 输入光标位置
} console_t;

// 分配对象
console_t *console_new(unsigned int x, unsigned int y, unsigned int width,
                       unsigned int height, const char *title);

// 释放对象
void console_free(const console_t *p);

// 获取关注点
void console_focus(console_t *box);

// 显示对象
void console_show(console_t *p, int z);

// 隐藏对象
void console_hide(console_t *p);

// 绘制字符串
void console_draw_text(console_t *box, const char *text);

// 添加一个字符
void console_push(console_t *box, char c);

// 移除一个字符
void console_pop(console_t *box);

// 移动窗口回调函数
void console_moving(void *p);

// 终端任务
task_t *init_console_task(void);
