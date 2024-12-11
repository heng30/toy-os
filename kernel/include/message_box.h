#pragma once

#include "def.h"
#include "win_sheet.h"

typedef struct {
    win_sheet_t *m_sheet; // 图层缓冲区
} message_box_t;

message_box_t *message_box_new(int x, int y, int width, int height,
                               int sheet_z, const char *title);

void message_box_free(const message_box_t *p);

void message_box_show(message_box_t *p, int sheet_z);

void message_box_hide(message_box_t *p);

bool message_box_is_visible(message_box_t *p);
