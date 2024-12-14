#include "win_sheet.h"
#include "colo8.h"
#include "def.h"
#include "draw.h"
#include "kutil.h"
#include "memory.h"
#include "mouse.h"

// 不要修改这个值， 因为受到`map`类型为`unsigned char`限制
#define MAX_SHEETS 256

#define SHEET_UNUSE 0
#define SHEET_USE 1

// 窗口绘制层管理器
typedef struct {
    // 图层管理像素表，表中记录每个图层管理的像素。
    // 在刷新图层时，只刷新自己管理的像素
    unsigned char *m_map;

    int m_top; // 图层数量, 总是指向最高图层的下标
    win_sheet_t *m_focus_sheet;  // 当前获取焦点图层
    win_sheet_t *m_moving_sheet; // 需要移动的图层
    win_sheet_t *m_sheets[MAX_SHEETS];
    win_sheet_t m_sheets0[MAX_SHEETS];
} win_sheet_ctl_t;

static win_sheet_ctl_t *g_sheet_ctl = NULL;

void init_win_sheet_ctl(void) {
    g_sheet_ctl = (win_sheet_ctl_t *)memman_alloc_4k(sizeof(win_sheet_ctl_t));
    assert(g_sheet_ctl != NULL, "init_win_sheet_ctl alloc 4k failed");

    g_sheet_ctl->m_map = (unsigned char *)memman_alloc_4k(
        g_boot_info.m_screen_x * g_boot_info.m_screen_y);
    assert(g_sheet_ctl->m_map != NULL, "init_win_sheet_ctl alloc map error");

    g_sheet_ctl->m_focus_sheet = NULL;
    g_sheet_ctl->m_moving_sheet = NULL;
    g_sheet_ctl->m_top = -1;

    for (int i = 0; i < MAX_SHEETS; i++) {
        g_sheet_ctl->m_sheets0[i].m_flags = SHEET_UNUSE;
    }
}

// 更新各个图层像素管理映射表，表中保存每个像素归那个图层管理，图层更新时只更新自己管理的像素
static void _win_sheet_refreshmap(int vx0, int vy0, int vx1, int vy1, int h0) {
    if (vx0 < 0) {
        vx0 = 0;
    }

    if (vy0 < 0) {
        vy0 = 0;
    }

    if (vx1 > g_boot_info.m_screen_x) {
        vx1 = g_boot_info.m_screen_x;
    }

    if (vy1 > g_boot_info.m_screen_y) {
        vy1 = g_boot_info.m_screen_y;
    }

    for (int h = h0; h <= g_sheet_ctl->m_top; h++) {
        win_sheet_t *sht = g_sheet_ctl->m_sheets[h];
        unsigned char sid = sht - g_sheet_ctl->m_sheets0;

        // 计算局部坐标系，相对于绘制的窗口的偏移
        int bx0 = vx0 - sht->m_vx0, by0 = vy0 - sht->m_vy0;
        int bx1 = vx1 - sht->m_vx0, by1 = vy1 - sht->m_vy0;

        if (bx0 < 0) {
            bx0 = 0;
        }

        if (by0 < 0) {
            by0 = 0;
        }

        if (bx1 > sht->m_bxsize) {
            bx1 = sht->m_bxsize;
        }

        if (by1 > sht->m_bysize) {
            by1 = sht->m_bysize;
        }

        for (int by = by0; by < by1; by++) {
            int vy = sht->m_vy0 + by; // 转换为全局坐标系

            for (int bx = bx0; bx < bx1; bx++) {
                int vx = sht->m_vx0 + bx; // 转换为全局坐标系

                // 不是不可见像素，则归我管
                if (sht->m_buf[by * sht->m_bxsize + bx] != sht->m_col_inv) {
                    g_sheet_ctl->m_map[vy * g_boot_info.m_screen_x + vx] = sid;
                }
            }
        }
    }
}

win_sheet_t *win_sheet_alloc(void) {
    if (g_sheet_ctl->m_top >= MAX_SHEETS - 1) {
        return NULL;
    }

    for (int i = 0; i < MAX_SHEETS; i++) {
        if (g_sheet_ctl->m_sheets0[i].m_flags == SHEET_UNUSE) {
            win_sheet_t *sht = &g_sheet_ctl->m_sheets0[i];
            sht->m_name = NULL;
            sht->m_vx0 = 0;
            sht->m_vy0 = 0;
            sht->m_flags = SHEET_USE;
            sht->m_index = -1;
            sht->m_z = HIDE_WIN_SHEET_Z;
            sht->m_is_transparent_layer = false;

            sheet_userdata_t userdata = {
                .m_type = SHEET_USERDATA_TYPE_NONE,
                .m_data = NULL,
            };
            sht->m_userdata = userdata;
            return sht;
        }
    }

    return NULL;
}

void win_sheet_setbuf(win_sheet_t *sht, const char *name, unsigned char *buf,
                      int bxsize, int bysize, int col_inv) {
    sht->m_name = name;
    sht->m_buf = buf;
    sht->m_bxsize = bxsize;
    sht->m_bysize = bysize;
    sht->m_col_inv = col_inv;
}

static unsigned char _color_under_transparent_layer(int index, int vx, int vy) {
    for (; index >= 0; index--) {
        win_sheet_t *sht = g_sheet_ctl->m_sheets[index];
        int x0 = sht->m_vx0, y0 = sht->m_vy0;
        int x1 = sht->m_vx0 + sht->m_bxsize, y1 = sht->m_vy0 + sht->m_bysize;

        // 像素点落在图层内
        if (x0 <= vx && vx < x1 && y0 <= vy && vy < y1) {
            int dx = vx - x0, dy = vy - y0;
            int pos = sht->m_bxsize * dy + dx;
            unsigned char c = sht->m_buf[pos];

            if (c != COLOR_INVISIBLE)
                return c;
        }
    }

    return COLOR_INVISIBLE;
}

void win_sheet_refreshsub(int vx0, int vy0, int vx1, int vy1, int h0, int h1) {
    int xsize = g_boot_info.m_screen_x, ysize = g_boot_info.m_screen_y;
    int max_pos = xsize * ysize;

    if (h0 < 0)
        h0 = 0;

    for (int h = h0; h <= h1; h++) {
        win_sheet_t *sht = g_sheet_ctl->m_sheets[h];
        unsigned char *buf = sht->m_buf;
        unsigned char sid = sht - g_sheet_ctl->m_sheets0;

        for (int by = 0; by < sht->m_bysize; by++) {
            int vy = sht->m_vy0 + by;

            // 绘制1行图像
            for (int bx = 0; bx < sht->m_bxsize; bx++) {
                int vx = sht->m_vx0 + bx;

                // 跳过不在指定绘制区域内的像素
                if (vx0 <= vx && vx < vx1 && vy0 <= vy && vy < vy1) {
                    unsigned char c = buf[by * sht->m_bxsize + bx];
                    int pos = vy * xsize + vx;

                    // 如果是透明图层强制绘制像素
                    if (sht->m_is_transparent_layer) {
                        // 这个像素的管理图层高度没当前图层高，侧需要进行处理
                        if (g_sheet_ctl->m_map[pos] == sid && pos >= 0 &&
                            pos < max_pos) {
                            //  获取下层可见像素
                            if (c == sht->m_col_inv) {
                                c = _color_under_transparent_layer(
                                    sht->m_index - 1, vx, vy);

                                if (c != COLOR_INVISIBLE) {
                                    g_boot_info.m_vga_ram[pos] = c;
                                }
                            } else {
                                g_boot_info.m_vga_ram[pos] = c;
                            }
                        }
                    } else {
                        // 判断是否是不可见像素
                        if (c != sht->m_col_inv) {
                            // 判断像素是否归自己管
                            if (g_sheet_ctl->m_map[pos] == sid && pos >= 0 &&
                                pos < max_pos) {
                                g_boot_info.m_vga_ram[pos] = c;
                            }
                        }
                    }
                }
            }
        }
    }
}

// 刷新当前图层，不会刷新其他图层; 不会刷新map
void win_sheet_refresh(win_sheet_t *sht, int bx0, int by0, int bx1, int by1) {
    if (sht->m_index >= 0) {
        if (sht->m_is_transparent_layer) {
            _win_sheet_refreshmap(sht->m_vx0 + bx0, sht->m_vy0 + by0,
                                  sht->m_vx0 + bx1, sht->m_vy0 + by1,
                                  sht->m_index);
        }

        win_sheet_refreshsub(sht->m_vx0 + bx0, sht->m_vy0 + by0,
                             sht->m_vx0 + bx1, sht->m_vy0 + by1, sht->m_index,
                             sht->m_index);
    }
}

// 强制刷新图层和map表
void win_sheet_refresh_force(win_sheet_t *sht, int bx0, int by0, int bx1,
                             int by1) {
    _win_sheet_refreshmap(sht->m_vx0 + bx0, sht->m_vy0 + by0, sht->m_vx0 + bx1,
                          sht->m_vy0 + by1, sht->m_index);

    win_sheet_refreshsub(sht->m_vx0 + bx0, sht->m_vy0 + by0, sht->m_vx0 + bx1,
                         sht->m_vy0 + by1, sht->m_index, sht->m_index);
}

void win_sheet_slide(win_sheet_t *sht, int vx0, int vy0) {
    int old_vx0 = sht->m_vx0, old_vy0 = sht->m_vy0;
    sht->m_vx0 = vx0;
    sht->m_vy0 = vy0;

    if (sht->m_index >= 0) {
        _win_sheet_refreshmap(old_vx0, old_vy0, old_vx0 + sht->m_bxsize,
                              old_vy0 + sht->m_bysize, 0);

        _win_sheet_refreshmap(vx0, vy0, vx0 + sht->m_bxsize,
                              vy0 + sht->m_bysize, sht->m_index);

        // 刷新当前图层以下的图层
        win_sheet_refreshsub(old_vx0, old_vy0, old_vx0 + sht->m_bxsize,
                             old_vy0 + sht->m_bysize, 0, sht->m_index - 1);

        // 刷新当前图层
        win_sheet_refreshsub(vx0, vy0, vx0 + sht->m_bxsize, vy0 + sht->m_bysize,
                             sht->m_index, sht->m_index);
    }
}

static int _calc_index(int z) {
    for (int i = 0; i <= g_sheet_ctl->m_top; i++) {
        if (z < g_sheet_ctl->m_sheets[i]->m_z) {
            return i;
        }
    }
    return g_sheet_ctl->m_top + 1;
}

// 移动图层
static void _win_sheet_updown(win_sheet_t *sht, int z) {
    if (sht->m_z == z)
        return;

    int old_index = sht->m_index, new_index = -1;
    win_sheet_ctl_t *ctl = g_sheet_ctl;

    if (z > TOP_WIN_SHEET_Z) {
        z = TOP_WIN_SHEET_Z;
    } else if (z < BOTTOM_WIN_SHEET_Z) {
        z = HIDE_WIN_SHEET_Z;
    } else {
        z = z;
    }

    if (z == HIDE_WIN_SHEET_Z) {
        new_index = -1;
    } else {
        new_index = _calc_index(z);
    }

    if ((old_index < 0 && new_index < 0) || (old_index == new_index))
        return;

    sht->m_index = new_index;
    sht->m_z = z;

    // 图层向下移动
    if (old_index > new_index) {
        // 可见图层间移动
        if (new_index >= 0) {
            // 将[new_index, old_index-1]之间的图层向上移一位到[new_index+1,
            // old_index]
            for (int h = old_index; h > new_index; h--) {
                ctl->m_sheets[h] = ctl->m_sheets[h - 1];
                ctl->m_sheets[h]->m_index = h;
            }

            ctl->m_sheets[new_index] = sht;

            // 重新计算[new_index+1, top]图层到map的映射
            // new_index层在向下移动的后管理的像素只会由于其上层图层而减少，不会增多.
            // 所以从`new_index+1`开始
            _win_sheet_refreshmap(sht->m_vx0, sht->m_vy0,
                                  sht->m_vx0 + sht->m_bxsize,
                                  sht->m_vy0 + sht->m_bysize, new_index + 1);

            // 因为是图层间的向下移动，所以只需刷新[new_index+1,
            // old_index]之间的图层
            win_sheet_refreshsub(
                sht->m_vx0, sht->m_vy0, sht->m_vx0 + sht->m_bxsize,
                sht->m_vy0 + sht->m_bysize, new_index + 1, old_index);
        } else { // 隐藏图层
            if (ctl->m_top > old_index) {
                // 将[old_index+1, top]之间的图层向下移一位到[old_index,
                // top-1]
                for (int h = old_index; h < ctl->m_top; h++) {
                    ctl->m_sheets[h] = ctl->m_sheets[h + 1];
                    ctl->m_sheets[h]->m_index = h;
                }
            }

            ctl->m_top--;

            // 重新计算所有图层到map的映射
            _win_sheet_refreshmap(sht->m_vx0, sht->m_vy0,
                                  sht->m_vx0 + sht->m_bxsize,
                                  sht->m_vy0 + sht->m_bysize, 0);

            // 因为是隐藏图层，所以只需刷新隐藏图层以下的图层[0,
            // old_index-1]
            win_sheet_refreshsub(sht->m_vx0, sht->m_vy0,
                                 sht->m_vx0 + sht->m_bxsize,
                                 sht->m_vy0 + sht->m_bysize, 0, old_index - 1);
        }
    } else if (old_index < new_index) { // 图层向上移动
        // 可见图层间移动
        if (old_index >= 0) {
            // 将[old_index+1, new_index]之间的图层向下移一位到[old_index,
            // new_index-1]
            for (int h = old_index; h < new_index; h++) {
                ctl->m_sheets[h] = ctl->m_sheets[h + 1];
                ctl->m_sheets[h]->m_index = h;
            }

            ctl->m_sheets[new_index] = sht;
        } else { // 隐藏图层移动到可见图层
            // 将[new_index, top]之间的图层向上移一位到[new_index+1, top+1]
            for (int h = ctl->m_top; h >= new_index; h--) {
                ctl->m_sheets[h + 1] = ctl->m_sheets[h];
                ctl->m_sheets[h]->m_index = h + 1;
            }

            ctl->m_sheets[new_index] = sht;
            ctl->m_top++;
        }

        // 重新计算[new_index, top]图层到map的映射
        _win_sheet_refreshmap(sht->m_vx0, sht->m_vy0,
                              sht->m_vx0 + sht->m_bxsize,
                              sht->m_vy0 + sht->m_bysize, new_index);

        // 刷新当前图层
        win_sheet_refreshsub(sht->m_vx0, sht->m_vy0, sht->m_vx0 + sht->m_bxsize,
                             sht->m_vy0 + sht->m_bysize, new_index, new_index);
    }
}

void win_sheet_free(win_sheet_t *sheet) {
    _win_sheet_updown(sheet, HIDE_WIN_SHEET_Z);
    sheet->m_flags = SHEET_UNUSE;

    if (g_sheet_ctl->m_focus_sheet == sheet)
        win_sheet_set_focus(NULL);

    if (g_sheet_ctl->m_moving_sheet == sheet)
        win_sheet_set_moving(NULL);
}

bool win_sheet_is_valid_z(int z) { return z <= TOP_WIN_SHEET_Z; }

bool win_sheet_is_visible(win_sheet_t *p) {
    return p->m_z >= BOTTOM_WIN_SHEET_Z;
}

const char *win_sheet_get_name(win_sheet_t *p) { return p->m_name; }

void win_sheet_set_name(win_sheet_t *p, const char *name) { p->m_name = name; }

void win_sheet_show(win_sheet_t *p, int sheet_z) {
    _win_sheet_updown(p, sheet_z);
}

void win_sheet_hide(win_sheet_t *p) {
    _win_sheet_updown(p, HIDE_WIN_SHEET_Z);

    if (g_sheet_ctl->m_focus_sheet == p)
        win_sheet_set_focus(NULL);

    if (g_sheet_ctl->m_moving_sheet == p)
        win_sheet_set_moving(NULL);
}

void win_sheet_set_focus(win_sheet_t *p) { g_sheet_ctl->m_focus_sheet = p; }

bool win_sheet_is_focus(win_sheet_t *p) {
    return g_sheet_ctl->m_focus_sheet == p;
}

void win_sheet_set_moving(win_sheet_t *p) { g_sheet_ctl->m_moving_sheet = p; }

bool win_sheet_is_moving(win_sheet_t *p) {
    return g_sheet_ctl->m_moving_sheet == p;
}

win_sheet_t* win_sheet_get_moving_sheet(void) {
    return g_sheet_ctl->m_moving_sheet;
}
