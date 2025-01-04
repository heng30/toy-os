#pragma once

#define SYSTEM_CALL_INT 0x2d // 系统调用中断号

#define SYSTEM_CALL_CONSOLE_DRAW_CH 1000
#define SYSTEM_CALL_CONSOLE_DRAW_TEXT 1001

/* 系统调用函数：
 *  系统调用中断号：0x2d:
 *  edx: 系统调用号
 */
void system_call_api(int edi, int esi, int ebp, int esp, int ebx, int edx,
                     int ecx, int eax);
