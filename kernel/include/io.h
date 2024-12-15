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
unsigned char io_in8(unsigned int port);
unsigned short io_in16(unsigned int port);
unsigned int io_in32(unsigned int port);

// 向port中写入数据
void io_out8(int port, unsigned char data);
void io_out16(int port, unsigned short data);
void io_out32(int port, unsigned int data);

// 获取eflags寄存器
int io_load_eflags(void);

// 设置eflags寄存器
void io_store_eflags(int eflags);

// 延时
void io_delay(void);

// 调试字符
void debug_char(void);

// 获取内存描述块数量
unsigned int get_memory_block_count(void);

// 获取内存描述块地址
void* get_memory_block_buffer(void);

// 获取启动信息
void* get_boot_info(void);

// 获取字体数据 c << 4 + offset
char get_font_data(int c, int offset);

// 获取段描述符表
unsigned int get_addr_gdt(void);

// 获取代码段地址
unsigned int get_code32_addr(void);

// 告诉cpu第n (value = n << 3)个段描述符中存储一个TSS32对象,
// 当有进程切换时会见当前的TSS32对象存到第n个段描述符中
void load_tr(unsigned int value);

// 任务切换
void taskswitch6(void);
void taskswitch7(void);
void taskswitch8(void);
void taskswitch9(void);

// 进程切换用的远跳转
void farjmp(int a, int tr);
