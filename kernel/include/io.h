#pragma once

// 等待输入
void io_hlt(void);

// 关中断
void io_cli(void);

// 开中断
void io_sti(void);

// 开中断并等待输入
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

// 延时
void io_delay(void);

// 调试字符
void debug_char(void);

// 获取内存描述块数量
int get_memory_block_count(void);

// 获取内存描述块地址
void* get_memory_block_buffer(void);

// 获取字体数据 c << 4 + offset
char get_font_data(int c, int offset);
