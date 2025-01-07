#pragma once

#include "widgets/console.h"

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

// 结束外部命令进程
void cmd_kill_process(void);
