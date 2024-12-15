#pragma once

#include "draw.h"
#include "timer.h"

#define INPUT_CURSOR_BLINK_INTERVAL 50 // 500ms = 50 x 10ms

#define INPUT_CURSOR_WIDTH FONT_WIDTH   // 输入光标宽度
#define INPUT_CURSOR_HEIGHT FONT_HEIGHT // 输入光标高度

typedef struct {
    unsigned char
        m_input_cursor[INPUT_CURSOR_HEIGHT * INPUT_CURSOR_WIDTH]; // 输入光标
    unsigned char m_input_cursor_color; // 输入光标颜色
} input_cursor_t;

// 输入光标图层
extern win_sheet_t *g_input_cursor_sht;

// 输入光标定时器
extern timer_t *g_input_cursor_timer;

// 初始化输入光标
void init_input_cursor(void);

// 显示输入光标
void input_cursor_show(int z);

// 隐藏输入光标
void input_cursor_hide(void);

// 移动输入光标
void input_cursor_move(unsigned int vx, unsigned int vy);

// 输入光标是否可见
bool input_cursor_is_visible(void);

// 闪烁输入光标
void input_cursor_blink(void);
