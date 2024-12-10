#include "win_sheet.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "memory.h"

#define MAX_SHEETS 256
#define SHEET_UNUSE 0
#define SHEET_USE 1

// 窗口绘制层管理器
typedef struct {
    int m_top; // 图层数量
    win_sheet_t *m_sheets[MAX_SHEETS];
    win_sheet_t m_sheets0[MAX_SHEETS];
} win_sheet_ctl_t;

static win_sheet_ctl_t *g_sheet_ctl = NULL;

int win_sheet_ctl_init(void) {
    g_sheet_ctl = (win_sheet_ctl_t *)memman_alloc_4k(sizeof(win_sheet_ctl_t));
    if (g_sheet_ctl == NULL) {
        return -1;
    }

    g_sheet_ctl->m_top = -1;

    for (int i = 0; i < MAX_SHEETS; i++) {
        g_sheet_ctl->m_sheets0[i].m_flags = SHEET_UNUSE;
    }

    return 0;
}

win_sheet_t *win_sheet_alloc(void) {
    for (int i = 0; i < MAX_SHEETS; i++) {
        if (g_sheet_ctl->m_sheets0[i].m_flags == SHEET_UNUSE) {
            win_sheet_t *sht = &g_sheet_ctl->m_sheets0[i];
            g_sheet_ctl->m_sheets[i] = sht;

            sht->m_vx0 = 0;
            sht->m_vy0 = 0;
            sht->m_flags = SHEET_USE;
            sht->m_height = HIDE_WIN_SHEET_HEIGHT;
            return sht;
        }
    }

    return NULL;
}

void win_sheet_free(win_sheet_t *sheet) {
    win_sheet_updown(sheet, HIDE_WIN_SHEET_HEIGHT);
    sheet->m_flags = SHEET_UNUSE;
}

void win_sheet_setbuf(win_sheet_t *sht, unsigned char *buf, int bxsize,
                      int bysize, int col_inv) {
    sht->m_buf = buf;
    sht->m_bxsize = bxsize;
    sht->m_bysize = bysize;
    sht->m_col_inv = col_inv;
}

void win_sheet_refreshsub(int vx0, int vy0, int vx1, int vy1) {
    int xsize = g_boot_info.m_screen_x, ysize = g_boot_info.m_screen_y;
    int max_pos = xsize * ysize;

    for (int h = 0; h <= g_sheet_ctl->m_top; h++) {
        win_sheet_t *sht = g_sheet_ctl->m_sheets[h];
        unsigned char *buf = sht->m_buf;

        for (int by = 0; by < sht->m_bysize; by++) {
            int vy = sht->m_vy0 + by;

            // 绘制1行图像
            for (int bx = 0; bx < sht->m_bxsize; bx++) {
                int vx = sht->m_vx0 + bx;

                // 跳过不在指定绘制区域内的像素
                if (vx0 <= vx && vx < vx1 && vy0 <= vy && vy < vy1) {
                    unsigned char c = buf[by * sht->m_bxsize + bx];

                    if (c != sht->m_col_inv) {
                        int pos = vy * xsize + vx;
                        if (pos >= 0 && pos < max_pos) {
                            g_boot_info.m_vga_ram[pos] = c;
                        }
                    }
                }
            }
        }
    }
}

void win_sheet_refresh(win_sheet_t *sht, int bx0, int by0, int bx1, int by1) {
    if (sht->m_height >= 0) {
        win_sheet_refreshsub(sht->m_vx0 + bx0, sht->m_vy0 + by0,
                             sht->m_vx0 + bx1, sht->m_vy0 + by1);
    }
}

void win_sheet_slide(win_sheet_t *sht, int vx0, int vy0) {
    int old_vx0 = sht->m_vx0, old_vy0 = sht->m_vy0;
    sht->m_vx0 = vx0;
    sht->m_vy0 = vy0;

    if (sht->m_height >= 0) {
        win_sheet_refreshsub(old_vx0, old_vy0, old_vx0 + sht->m_bxsize,
                             old_vy0 + sht->m_bysize);
        win_sheet_refreshsub(vx0, vy0, vx0 + sht->m_bxsize,
                             vy0 + sht->m_bysize);
    }
}

void win_sheet_updown(win_sheet_t *sht, int height) {
    int old = sht->m_height;
    win_sheet_ctl_t *ctl = g_sheet_ctl;

    if (height > ctl->m_top + 1) {
        height = ctl->m_top + 1;
    }

    if (height < -1) {
        height = -1;
    }

    sht->m_height = height;

    // 图层向下移动
    if (old > height) {
        // 可见图层见移动
        if (height >= 0) {
            // 将[height, old-1]之间的图层向上移一位到[height+1, old]
            for (int h = old; h > height; h--) {
                ctl->m_sheets[h] = ctl->m_sheets[h - 1];
                ctl->m_sheets[h]->m_height = h;
            }

            ctl->m_sheets[height] = sht;
        } else { // 隐藏图层
            if (ctl->m_top > old) {
                // 将[old+1, top]之间的图层向下移一位到[old, top-1]
                for (int h = old; h < ctl->m_top; h++) {
                    ctl->m_sheets[h] = ctl->m_sheets[h + 1];
                    ctl->m_sheets[h]->m_height = h;
                }
            }

            ctl->m_top--;
        }

        win_sheet_refreshsub(sht->m_vx0, sht->m_vy0, sht->m_vx0 + sht->m_bxsize,
                             sht->m_vy0 + sht->m_bysize);
    } else if (old < height) { // 图层向上移动
        // 可见图层间移动
        if (old >= 0) {
            // 将[old+1, height]之间的图层向下移一位到[old, height-1]
            for (int h = old; h < height; h++) {
                ctl->m_sheets[h] = ctl->m_sheets[h + 1];
                ctl->m_sheets[h]->m_height = h;
            }

            ctl->m_sheets[height] = sht;
        } else { // 隐藏图层移动到可见图层
            // 将[height, top]之间的图层向上移一位到[height+1, top+1]
            for (int h = ctl->m_top; h >= height; h--) {
                ctl->m_sheets[h + 1] = ctl->m_sheets[h];
                ctl->m_sheets[h + 1]->m_height = h + 1;
            }

            ctl->m_sheets[height] = sht;
            ctl->m_top++;
        }

        win_sheet_refreshsub(sht->m_vx0, sht->m_vy0, sht->m_vx0 + sht->m_bxsize,
                             sht->m_vy0 + sht->m_bysize);
    }
}