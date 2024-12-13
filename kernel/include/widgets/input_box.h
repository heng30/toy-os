#pragma once

#include "def.h"
#include "message_box.h"

#define INPUT_BOX_TEXT_MAX_LEN 511

typedef struct {
    win_sheet_t *m_sheet; // 对话框架, 必须是第一个成员
    char m_text[INPUT_BOX_TEXT_MAX_LEN + 1]; // 输入的字符串
} input_box_t;

input_box_t *input_box_new(int x, int y, int width, int height,
                           const char *title);

void input_box_free(const input_box_t *p);

void input_box_focus(input_box_t *box);

// 绘制字符串
void input_box_draw_text(input_box_t *box, const char *text);

// 添加一个字符
void input_box_push(input_box_t *box, char c);

void input_box_pop(input_box_t *box);
