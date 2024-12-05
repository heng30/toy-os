#pragma once

// 等待输入
void io_hlt(void);

// 关中断
void io_cli(void);

void io_sti(void);

void io_stihlt(void);

// 从port中读取数据
char io_in8(int port);
short io_in16(int port);
int io_in32(int port);

// 向port中写入数据
void io_out8(int port, char data);
void io_out16(int port, short data);
void io_out32(int port, int data);

// 获取eflags寄存器
int io_load_eflags(void);

// 设置eflags寄存器
void io_store_eflags(int eflags);