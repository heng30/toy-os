#pragma once

// 系统调用中断号
#define SYSTEM_CALL_INT 0x2d

// 非法系统调用
#define SYSTEM_CALL_INVALID 0

// 需要终端打印的字符放到al寄存器中
#define SYSTEM_CALL_CONSOLE_DRAW_CH 1000

// 需要终端打印的字符串放到ebx寄存器中
#define SYSTEM_CALL_CONSOLE_DRAW_TEXT 1001

/* 使用：`int 0x2d`. 0x2d: 系统调用中断号
 * 系统调用函数：
 *  edx: 系统调用号
 */
void system_call_api(int edi, int esi, int ebp, int esp, int ebx, int edx,
                     int ecx, int eax);
