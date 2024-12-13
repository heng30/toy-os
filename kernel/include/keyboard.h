#pragma once

#include "def.h"
#include "fifo8.h"

// 键盘按下和释放码
typedef struct {
    unsigned char down;
    unsigned char up;
} key_code_t;

// 从中断函数里，保存键盘输入
extern fifo8_t g_keyinfo;

// 按下按键映射表
extern char keydown_table[0x54];

// 初始化键盘
void init_keyboard(void);

// 键盘中断函数
void int_handler_from_c(char *esp);

inline bool is_shift_key_down(unsigned char code) {
    return code == 0x2a || code == 0x36;
}

inline bool is_shift_key_up(unsigned char code) {
    return code == 0xaa || code == 0xb6;
}

// 获取按下的可见字符
char get_pressed_char(unsigned char code);

bool is_ctrl_key_down(unsigned char code);

bool is_ctrl_key_up(unsigned char code);

bool is_alt_key_down(unsigned char code);

bool is_alt_key_up(unsigned char code);

bool is_backspace_down(unsigned char code);

bool is_enter_down(unsigned char code);

bool is_enter_up(unsigned char code);
