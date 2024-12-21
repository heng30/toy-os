#pragma once

#include "def.h"
#include "win_sheet.h"

typedef struct {
    win_sheet_t *m_sheet;
    const char *m_title;
} window_t;

window_t *window_new(unsigned int x, unsigned int y, unsigned int width,
                        unsigned int height, const char *title);

void window_free(window_t *p);

void window_show(window_t *p, int z);

void window_hide(window_t *p);
