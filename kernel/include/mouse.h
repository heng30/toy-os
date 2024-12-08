#pragma once

#include "keyboard_mouse.h"
#include "fifo8.h"

#define CURSOR_ICON_SIZE 16 // 鼠标图标大小


#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

#define MOUSE_PHASE_UNINIT 0
#define MOUSE_PHASE_ONE 1
#define MOUSE_PHASE_TWO 2
#define MOUSE_PHASE_THREE 3

typedef struct {
    unsigned char m_buf[3];
    unsigned char m_phase;
    unsigned char m_btn;
    int m_rel_x, m_rel_y;
    int m_abs_x, m_abs_y;
} mouse_dec_t;

// 保存鼠标图标
extern char g_mcursor[256];

// 在中断函数中, 保存鼠标数据
extern fifo8_t g_mouseinfo;

// 鼠标位置信息
extern mouse_dec_t g_mdec;

// 保存输入的鼠标信息
extern unsigned char g_mousebuf[512];

// 初始化鼠标
void init_mouse_cursor(char *mouse, char bc);

// 鼠标中断函数
void int_handler_for_mouse(char *esp);

// 启用鼠标
void enable_mouse(void);

// 清除鼠标
void erase_mouse(unsigned char *vram);

// 绘制鼠标
void draw_mouse(unsigned char *vram);

// 计算鼠标位置
void compute_mouse_position(void);

// 解析鼠标数据
int mouse_decode(unsigned char data);
