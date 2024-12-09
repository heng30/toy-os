#pragma once

// 窗口绘制层
typedef struct {
    unsigned char *m_buf; // 绘制图像
    int m_bxsize;         // 绘制图像宽度
    int m_bysize;         // 绘制图像高度
    int m_vx0;            // 绘制开始的x轴
    int m_vy0;            // 绘制开始的y轴
    int m_col_inv;        // 不需要绘制像素颜色

    int m_height; // 图层高度
    int m_flags;  // 0: 可用, 1: 占用
} win_sheet_t;

// 初始化窗口图层管理器
int win_sheet_ctl_init(void);

// 分配一个图层
win_sheet_t *win_sheet_alloc(void);

// 设置一个图层信息
void win_sheet_setbuf(win_sheet_t *sht, unsigned char *buf, int bxsize, int bysize,
                  int col_inv);

// 刷新图层
void win_sheet_refresh(void);

// 移动图层
void win_sheet_slide(win_sheet_t *sht, int vx0, int vy0);

// 调整图层高度
void win_sheet_updown(win_sheet_t *sht, int height);
