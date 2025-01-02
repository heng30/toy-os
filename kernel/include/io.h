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

/* 1. 标志寄存器EFLAGS的中断允许标志IF（Interrupt
 * enableFlag）能够禁止为处理器INTR引脚上收到的可屏蔽硬件中断提供服务。当IF=0时，处理器禁止发送到INTR引脚的中断；当IF=1时，则发送到INTR引脚的中断信号会被处理器进行处理。
 * 2. EFLAGS中的IF标志不能够屏蔽使用INT指令从软件中产生的中断。
 * 3. IF标志并不影响发送到NMI引脚的非屏蔽中断，也不影响处理器产生的异常。
 * 如同EFLAGS中的其他标志一样，处理器在响应硬件复位操作时会清除IF标志（IF=0）。IF标志可以使用指令STI和CLI来设置或清除。只有当程序的CPL<=IOPL时才可执行这两条指令，否则将引发一般保护性异常。
 */

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
void *get_memory_block_buffer(void);

// 获取启动信息
void *get_boot_info(void);

// 获取字体数据 c << 4 + offset
char get_font_data(int c, int offset);

// 获取段描述符表
unsigned int get_addr_gdt(void);

// 获取代码段地址
unsigned int get_code32_addr(void);

// 获取堆栈开始地址
unsigned int get_stack_start_addr(void);

// 告诉cpu第n (value = n << 3)个段描述符中存储一个TSS32对象,
// 当有进程切换时会见当前的TSS32对象存到第n个段描述符中
void load_tr(unsigned int value);

// 任务切换
void taskswitch6(void);
void taskswitch7(void);
void taskswitch8(void);
void taskswitch9(void);

// 进程切换用的远跳转
// 将esp+4指向的栈中的值赋值给eip寄存器(指向下一条要执行的代码)
// 并且自动读取接下来的2个字节作为段描述符的索引下标，即gdt+n的位置, n = tr >> 3
// 从段描述符中加载TSS32对象到cpu中，从而实现任务切换
//
// 处理器可使用一下4种方式之一执行任务切换操作：
//  1.当前任务对GDT中的TSS描述符执行JMP或CALL指令；
//  2.当前任务对GDT或LDT中的任务门描述符执行JMP或CALL指令；
//  3.中断或异常向量指向IDT表中的任务门描述符；
//  4.当EFLAGS中的NT标志置位时当前任务执行IRET指令。
//
// 当任务切换是由JMP指令造成，那么新任务就不会是嵌套的。也即，NT标志会被设置为0，并且不使用前一任务链接字段。JMP指令用于不希望出现嵌套的任务切换中。
void farjmp(unsigned int a, unsigned short tr);

// 获取保存eip值的位置
unsigned int call_external_bin_save_eip_addr(void);
