#pragma once

#include "def.h"
#include "message_box.h"

typedef struct {
    bool is_focus;        // 是否获取到鼠标的焦点
    message_box_t *m_box; // 对话框架
} input_box_t;

input_box_t *input_box_new(int x, int y, int width, int height,
                           const char *title);

void input_box_free(const input_box_t *p);

void input_box_show(input_box_t *p, int sheet_z);

void input_box_hide(input_box_t *p);

bool input_box_is_visible(input_box_t *p);

const char *input_box_get_name(input_box_t *p);

void input_box_set_name(input_box_t *p, const char *name);
