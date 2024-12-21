#pragma once

#include "def.h"
#include "win_sheet.h"

// 窗口id,用于标识不同的窗口
#define WINDOW_ID_NONE 0
#define WINDOW_ID_INPUT_BOX 1
#define WINDOW_ID_CONSOLE 2

// 鼠标点击的位置
#define WINDOW_CTL_MOUSE_CLICK_FLAG_NONE 0
#define WINDOW_CTL_MOUSE_CLICK_FLAG_TITLE 1
#define WINDOW_CTL_MOUSE_CLICK_FLAG_CLOSEBTN 2
#define WINDOW_CTL_MOUSE_CLICK_FLAG_BODY 3

typedef struct {
    win_sheet_t *m_sheet; // 图层J
    const char *m_title;  // 窗口标题
    unsigned char m_id;   // 窗口id
    void *m_instance;     // 真正的实例对象，NULL则是window_t的对象
} window_t;

typedef struct {
    window_t *m_focus_window;  // 当前获取焦点窗口
    window_t *m_moving_window; // 需要移动的窗口
    unsigned char m_mouse_click_flag;   // 鼠标按下时在图层的什么位置
    unsigned int m_top; // 指向下一个可以添加的下标
    window_t *m_windows[MAX_SHEETS];
} window_ctl_t;

extern window_ctl_t g_window_ctl;

// 创建一个窗口
window_t *window_new(unsigned int x, unsigned int y, unsigned int width,
                     unsigned int height, unsigned char id, const char *title,
                     void *instance);

// 释放一个窗口
void window_free(window_t *p);

// 显示一个窗口
void window_show(window_t *p, int z);

// 隐藏一个窗口
void window_hide(window_t *p);

// 向窗口管理器添加一个窗口
void window_ctl_add(window_t *p);

// 从窗口管理器中移除一个窗口
void window_ctl_remove(window_t *p);

// 图层获取焦点
void window_ctl_set_focus_window(window_t *p);

// 判断图层是否是焦点图层
bool window_ctl_is_focus_window(window_t *p);

// 设置图层是否移动
void window_ctl_set_moving_window(window_t *p);

// 判断图层释放可移动
bool window_ctl_is_moving_window(window_t *p);

// 获取当前移动图层
window_t *window_ctl_get_moving_window(void);

// 获取鼠标下的窗口
window_t *window_ctl_get_mouse_click_window(void);
