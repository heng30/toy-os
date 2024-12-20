#pragma once

#include "def.h"
#include "message_box.h"

#define INPUT_BOX_TEXT_MAX_LEN 512

typedef struct {
    win_sheet_t *m_sheet; // 对话框架, 必须是第一个成员
    char m_text[INPUT_BOX_TEXT_MAX_LEN]; // 输入的字符串
} input_box_t;

// 分配对象
input_box_t *input_box_new(unsigned int x, unsigned int y, unsigned int width, unsigned int height,
                           const char *title);

// 释放对象
void input_box_free(const input_box_t *p);

// 获取关注点
void input_box_focus(input_box_t *box);

// 显示对象
void input_box_show(input_box_t *p, int z);

// 隐藏对象
void input_box_hide(input_box_t *p);

// 绘制字符串
void input_box_draw_text(input_box_t *box, const char *text);

// 添加一个字符
void input_box_push(input_box_t *box, char c);

// 移除一个字符
void input_box_pop(input_box_t *box);

// 移动窗口回调函数
void input_box_moving(void *p);
