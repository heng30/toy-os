#pragma once

#include "def.h"
#include "message_box.h"

typedef struct {
    win_sheet_t *m_sheet; // 对话框架, 必须是第一个成员
} input_box_t;

input_box_t *input_box_new(int x, int y, int width, int height,
                           const char *title);

void input_box_free(const input_box_t *p);
