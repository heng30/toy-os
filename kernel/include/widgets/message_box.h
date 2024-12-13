#pragma once

#include "def.h"
#include "win_sheet.h"

#define MESSAGE_BOX_TITLE_HEIGHT 20

win_sheet_t* message_box_new(int x, int y, int width, int height,
                               const char *title);

void message_box_free(win_sheet_t *p);
