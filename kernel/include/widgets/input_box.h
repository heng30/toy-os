#pragma once

#include "def.h"
#include "window.h"

#define INPUT_BOX_TEXT_MAX_LEN 512

typedef struct {
    window_t *m_win;
    char m_text[INPUT_BOX_TEXT_MAX_LEN]; // 输入的字符串
} input_box_t;

// 分配对象
input_box_t *input_box_new(unsigned int x, unsigned int y, unsigned int width, unsigned int height,
                           const char *title);

// 释放对象
void input_box_free(const input_box_t *p);

// 获取关注点
void input_box_focus(input_box_t *p);

// 显示对象
void input_box_show(input_box_t *p, int z);

// 隐藏对象
void input_box_hide(input_box_t *p);

// 绘制字符串
void input_box_draw_text(input_box_t *p, const char *text);

// 添加一个字符
void input_box_push(input_box_t *p, char c);

// 移除一个字符
void input_box_pop(input_box_t *p);

// 移动窗口回调函数
void input_box_moving(void *p);
