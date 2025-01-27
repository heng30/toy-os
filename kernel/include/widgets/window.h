#pragma once

#include "def.h"
#include "multi_task.h"
#include "win_sheet.h"

// 窗口id,用于标识不同的窗口
#define WINDOW_ID_NONE 0
#define WINDOW_ID_INPUT_BOX 1
#define WINDOW_ID_CONSOLE 2
#define WINDOW_ID_USER 3 // 用户态创建的窗口对象

// 鼠标点击的位置
#define WINDOW_CTL_MOUSE_CLICK_FLAG_NONE 0
#define WINDOW_CTL_MOUSE_CLICK_FLAG_TITLE 1
#define WINDOW_CTL_MOUSE_CLICK_FLAG_CLOSEBTN 2
#define WINDOW_CTL_MOUSE_CLICK_FLAG_WINDOW 4

typedef struct {
    bool m_enabled;              // 是否启用
    bool m_have_input_cursor;    // 是否能够输入
    bool m_is_waiting_for_close; // 是否等待关闭
    win_sheet_t *m_sheet;        // 图层
    const char *m_title;         // 窗口标题
    unsigned char m_id;          // 窗口id
    void *m_instance; // 真正的实例对象，NULL则是window_t的对象
    task_t *m_task;   // 关联的任务
} window_t;

typedef struct {
    window_t *m_focus_window;         // 当前获取焦点窗口
    window_t *m_moving_window;        // 需要移动的窗口
    unsigned char m_mouse_click_flag; // 鼠标按下时在图层的什么位置
    unsigned int m_top;               // 指向下一个可以添加的下标
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

//  移动一个窗口
void window_moving(window_t *p);

// 设置窗口获得焦点
void window_focus(window_t *p);

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

// 鼠标是否点击了关闭按钮
bool window_ctl_is_click_closebtn(void);

// 鼠标是否点击了标题栏
bool window_ctl_is_click_title(void);

// 鼠标是否点击了窗口
bool window_ctl_is_click_window(void);

// 将窗口移到最高层
void window_ctl_move_window_to_top(window_t *p);

// 将焦点移动到当前焦点窗口的下一个窗口
void window_ctl_focus_next_window(void);

// 判断一个窗口是否存在
bool window_ctl_is_window_exist(window_t *p);

// 通过id来找窗口
window_t *window_ctl_find_window_by_id(unsigned char id);

// 关闭窗口
void window_ctl_close_window_by_id(unsigned char id);

// 关闭窗口
void window_ctl_close_window(window_t *p);

// 关闭所有等待关闭的窗口
void window_ctl_close_all_waiting_window(void);
