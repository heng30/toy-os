#pragma once

#include "def.h"
#include "draw.h"
#include "fifo8.h"
#include "keyboard_mouse.h"
#include "multi_task.h"
#include "timer.h"
#include "win_sheet.h"

#include "widgets/window.h"

#define CURSOR_ICON_SIZE 16 // 鼠标图标大小

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

#define MOUSE_PHASE_UNINIT 0
#define MOUSE_PHASE_ONE 1
#define MOUSE_PHASE_TWO 2
#define MOUSE_PHASE_THREE 3

typedef struct {
    unsigned char m_cursor[CURSOR_ICON_SIZE * CURSOR_ICON_SIZE]; // 鼠标图像
    unsigned char m_buf[3];        // 保存鼠标移动解析后的输入数据
    unsigned char m_phase;         // 解析鼠标数据的阶段
    unsigned char m_btn;           // 鼠标按键
    int m_rel_x, m_rel_y;          // 鼠标相对偏移
    unsigned int m_abs_x, m_abs_y; // 鼠标位置
} mouse_dec_t;

// 在中断函数中, 保存鼠标数据
extern fifo8_t g_mouseinfo;

// 鼠标位置信息
extern mouse_dec_t g_mdec;

// 鼠标图层
extern win_sheet_t *g_mouse_sht;

// 鼠标任务
extern task_t *g_mouse_task;

// 初始化鼠标
void init_mouse(void);

// 鼠标中断函数
void int_handler_for_mouse(char *esp);

// 启用鼠标
void enable_mouse(void);

// 计算鼠标位置
void compute_mouse_position(void);

// 解析鼠标数据
int mouse_decode(unsigned char data);

// 初始化鼠标图层
void init_mouse_sheet(void);

// 保证鼠标在最高图层
void keep_mouse_sheet_on_top(void);

// 鼠标左键压下
bool is_mouse_left_btn_pressed(void);

// 鼠标右键压下
bool is_mouse_right_btn_pressed(void);

// 鼠标中键压下
bool is_mouse_middle_btn_pressed(void);

// 初始化鼠标任务
void init_mouse_task(void);

// 鼠标点击窗口，窗口获得焦点
void mouse_focus_window(window_t *win);
