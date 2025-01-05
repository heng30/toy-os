#pragma once

#define NULL ((void *)0)
#define SPACE ' '

#define true 1
#define false 0
typedef unsigned char bool;
typedef unsigned int ptr_t;

// 内存大小单位
#define KB 1024
#define MB (KB * KB)
#define GB (MB * KB)

// 要和kernel.asm中的调用外部程序描述符一致
#define GDT_CONSOLE_CMD_TR 6
#define GDT_CONSOLE_CMD_DS_TR 7
