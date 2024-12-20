#pragma once

#include "def.h"
#include "win_sheet.h"

win_sheet_t* message_box_new(unsigned int x, unsigned int y, unsigned int width, unsigned int height,
                               const char *title);

void message_box_free(win_sheet_t *p);
