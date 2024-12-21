#pragma once

#include "def.h"
#include "sheet_userdata.h"

// 不要修改这个值， 因为受到`map`类型为`unsigned char`限制
#define MAX_SHEETS 256

#define HIDE_WIN_SHEET_Z -1
#define BOTTOM_WIN_SHEET_Z 0
#define TOP_WIN_SHEET_Z 99

#define CANVAS_WIN_SHEET_Z (BOTTOM_WIN_SHEET_Z + 1)
#define MOUSE_WIN_SHEET_Z TOP_WIN_SHEET_Z

// 图层id,用于标识不同图层的用途
#define WIN_SHEET_ID_NONE 0
#define WIN_SHEET_ID_BACKGROUND 1
#define WIN_SHEET_ID_MOUSE 2
#define WIN_SHEET_ID_INPUT_COURSE 3
#define WIN_SHEET_ID_CANVAS 4
#define WIN_SHEET_ID_WINDOW 5

// 窗口绘制层
typedef struct {
    unsigned char m_id;      // 图层id
    unsigned char *m_buf;    // 绘制图像
    unsigned int m_bxsize;   // 绘制图像宽度
    unsigned int m_bysize;   // 绘制图像高度
    unsigned int m_vx0;      // 绘制开始的x轴
    unsigned int m_vy0;      // 绘制开始的y轴
    unsigned char m_col_inv; // 不需要绘制像素颜色

    int m_index;                 // 图层在图层数组中的下标
    int m_z;                     // 图层Z轴高度
    bool m_is_transparent_layer; // 是否为透明图层
    int m_flags;                 // 0: 可用, 1: 占用

    sheet_userdata_t m_userdata; // 用户数据
} win_sheet_t;

// 初始化窗口图层管理器
void init_win_sheet_ctl(void);

// 分配一个图层
win_sheet_t *win_sheet_alloc(void);

// 释放一个图层
void win_sheet_free(win_sheet_t *sheet);

// 设置一个图层信息
void win_sheet_setbuf(win_sheet_t *sht, unsigned char id, unsigned char *buf,
                      unsigned int bxsize, unsigned int bysize,
                      unsigned char col_inv);

// 刷新图层指定区域
void win_sheet_refreshsub(unsigned int vx0, unsigned int vy0, unsigned int vx1, unsigned int vy1, int h0, int h1);

// 刷新图层, 非透明图层不会刷新map
void win_sheet_refresh(win_sheet_t *sht, unsigned int bx0, unsigned int by0, unsigned int bx1, unsigned int by1);

// 强制刷新图层和map表
void win_sheet_refresh_force(win_sheet_t *sht, unsigned int bx0, unsigned int by0, unsigned int bx1,
                             unsigned int by1);

// 移动图层
void win_sheet_slide(win_sheet_t *sht, unsigned int vx0, unsigned int vy0);

// 判断一个图层是否合法
bool win_sheet_is_valid_z(int z);

// 图层是否可见
bool win_sheet_is_visible(win_sheet_t *p);

// 显示图层
void win_sheet_show(win_sheet_t *p, int sheet_z);

// 隐藏图层
void win_sheet_hide(win_sheet_t *p);

