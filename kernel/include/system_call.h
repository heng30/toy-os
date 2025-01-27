#pragma once

#include "def.h"

extern volatile unsigned int g_rand_number;

// 系统调用中断号
#define SYSTEM_CALL_INT 0x2d

// 非法系统调用
#define SYSTEM_CALL_INVALID 0

// 结束外部命令，返回到内核态
#define SYSTEM_CALL_END_CMD 1

// 计算代码段常量字符串的长度
#define SYSTEM_CALL_STRLEN_CS 50

// 将代码段的数据复制到数据段上
#define SYSTEM_CALL_MEMCPY_CS2DS 51

// 获取一个随机数
#define SYSTEM_CALL_RAND_UINT 100

// 显示一个调试数字
#define SYSTEM_CALL_SHOW_DEBUG_UINT 101

// 分配一个定时器
#define SYSTEM_CALL_TIMER_ALLOC 102

// 释放一个定时器
#define SYSTEM_CALL_TIMER_FREE 103

// 设置定时器
#define SYSTEM_CALL_TIMER_SET 104

// 获取定时器是否超时
#define SYSTEM_CALL_TIMER_IS_TIMEOUT 105

// 打开文件
#define SYSTEM_CALL_FILE_OPEN 200

// 读取文件
#define SYSTEM_CALL_FILE_READ 201

// 写入文件
#define SYSTEM_CALL_FILE_WRITE 202

// 关闭文件
#define SYSTEM_CALL_FILE_CLOSE 203

// 获取文件大小
#define SYSTEM_CALL_FILE_SIZE 204

// 创建一个新窗口
#define SYSTEM_CALL_NEW_WINDOW 400

// 刷新窗口指定区域
#define SYSTEM_CALL_REFRESH_WINDOW 401

// 检查是否应该关闭程序
#define SYSTEM_CALL_IS_CLOSE_WINDOW 402

// 关闭程序
#define SYSTEM_CALL_CLOSE_WINDOW 403

// 在窗口中绘制一个字符串
#define SYSTEM_CALL_DRAW_TEXT_IN_WINDOW 501

// 在窗口中绘制一个矩形
#define SYSTEM_CALL_DRAW_BOX_IN_WINDOW 502

// 在窗口绘制一个像素点
#define SYSTEM_CALL_DRAW_POINT_IN_WINDOW 503

// 在窗口绘制一条直线
#define SYSTEM_CALL_DRAW_LINE_IN_WINDOW 504

// 获取图层的副本
#define SYSTEM_CALL_DUMP_WINDOW_SHEET 505

// 覆盖指定图层区域
#define SYSTEM_CALL_COVER_WINDOW_SHEET 506

// 需要终端打印的字符放到al寄存器中
#define SYSTEM_CALL_CONSOLE_DRAW_CH 1000

// 需要终端打印的字符串放到ebx寄存器中
#define SYSTEM_CALL_CONSOLE_DRAW_TEXT 1001

/* 使用：`int 0x2d`. 0x2d: 系统调用中断号
 * 系统调用函数：
 *  edx: 系统调用号
 */
ptr_t *system_call_api(unsigned int edi, unsigned int esi, unsigned int ebp,
                       unsigned int esp, unsigned int ebx, unsigned int edx,
                       unsigned int ecx, unsigned int eax);
