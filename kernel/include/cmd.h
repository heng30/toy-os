#pragma once

#include "widgets/console.h"

// 外部命令与内核进行通信的共享内存，在没有实现系统调用前，会使用该地址开始的1024个字节.
// 其他程序不应使用该部分内存
// 0x6000保存eip的值，内核代码的起始地址是0x8000，所以0x6000-0x6404的内存是不会和内核代码冲突
#define EXTERNAL_BIN_AND_KERNEL_SHARED_MEMORY 0x6004
#define EXTERNAL_BIN_AND_KERNEL_SHARED_MEMORY_SIZE 1024

// 清空屏幕
void cmd_cls(console_t *p);

// 输出总共可用的内存
void cmd_mem(console_t *console);

// 输出当前可用的内存
void cmd_free(console_t *console);

// 列出文件系统的文件名
void cmd_ls(console_t *console);

// 输出文件内容到终端
void cmd_cat(console_t *console);

// 执行外部命令
void cmd_exe(console_t *console);
