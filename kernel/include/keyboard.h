#pragma once

#include "fifo8.h"

// 从中断函数里，保存键盘输入
// extern unsigned char g_keybuf[64];

// 从中断函数里，保存键盘输入
extern fifo8_t g_keyinfo;

// 初始化键盘
void init_keyboard(void);

// 键盘中断函数
void int_handler_from_c(char *esp);
