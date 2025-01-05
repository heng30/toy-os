%include "pm.inc"

org   0x8000 ; 内核代码开始执行的地址，也是编译时代码相对偏移地址

TASK_COUNTS equ 10 ; 任务数量
TASK_STACK_SIZE equ 1024 ; 任务堆栈大小

jmp   LABEL_BEGIN

; 处理器并不使用GDT中的第1个描述符。把这个“空描述符”的段选择符加载进一个数据段寄存器（DS、ES、FS或GS）并不会产生一个异常，但是若使用这些加载了空描述符的段选择符访问内存时就肯定会产生一般保护性异常。通过使用这个段选择符初始化段寄存器，那么意外引用未使用的段寄存器肯定会产生一个异常。

; LDT表存放在LDT类型的系统段中。此时GDT必须含有LDT的段描述符。如果系统支持多LDT的话，那么每个LDT都必须在GDT中有一个段描述符和段选择符。一个LDT的段描述符可以存放在GDT表的任何地方。

; 全局描述符表，用于标明不同内存段的功能和权限
;                                  段基址          段界限                  属性
[SECTION .gdt]
LABEL_GDT:          Descriptor        0,            0,                      0
LABEL_DESC_CODE32:  Descriptor        0,            0fffffh,                DA_C | DA_32 | DA_LIMIT_4K

; 一般在实模式中使用
; 屏幕大小28x80共4kb，偶地址字节存放字符代码，奇地址字节存放显示属性。
; 一共8kb显存，可以缓冲两帧画面第一帧由于显示到屏幕
LABEL_DESC_VIDEO:   Descriptor        0B8000h,      0fffffh,                DA_DRW

LABEL_DESC_VRAM:    Descriptor        0,            0fffffh,                DA_DRW | DA_LIMIT_4K
LABEL_DESC_STACK:   Descriptor        0,            LEN_OF_STACK_SECTION,   DA_DRWA | DA_32
LABEL_DESC_FONT:    Descriptor        0,            0fffffh,                DA_DRW | DA_LIMIT_4K

; 调用外部程序描述符
LABEL_DESC_6:       Descriptor        0,            0,                      0409Ah

; 调用外部命令分配的数据段描述符，与内核进行隔离
LABEL_DESC_7:       Descriptor        0,            0,                      0

; 保留
LABEL_DESC_8:       Descriptor        0,            0,                      0
LABEL_DESC_9:       Descriptor        0,            0,                      0

; 进程切换相关. 创建TSS32结构，用户保存任务相关寄存器
; TSS描述符, 指向任务的TSS32对象
%rep TASK_COUNTS
                    Descriptor        0,            0,                      0
%endrep

GDT_LEN     equ    $ - LABEL_GDT
GDT_PTR     dw     GDT_LEN - 1 ; 因为段描述符总是8字节长，因此GDT的限长值应该设置成总是8的倍数减1（即8N-1）
            dd     0           ; 这里会保存LABEL_GDT的地址

SELECTOR_CODE32    equ   LABEL_DESC_CODE32 -  LABEL_GDT
SELECTOR_VIDEO     equ   LABEL_DESC_VIDEO  -  LABEL_GDT
SELECTOR_STACK     equ   LABEL_DESC_STACK  -  LABEL_GDT
SELECTOR_VRAM      equ   LABEL_DESC_VRAM   -  LABEL_GDT
SELECTOR_FONT      equ   LABEL_DESC_FONT   -  LABEL_GDT

; 中断描述符
; 允许的向量号范围是0到255。其中0到31保留用作80X86处理器定义的异常和中断，不过目前该范围内的向量号并非每个都已定义了功能，未定义功能的向量号将留作今后使用。
LABEL_IDT:
%rep  32
    Gate SELECTOR_CODE32, SPURIOUS_HANDLER,     0, DA_386IGate
%endrep

.020h:
    Gate SELECTOR_CODE32, TIMER_HANDLER,        0, DA_386IGate

.021h:
    Gate SELECTOR_CODE32, KEYBOARD_HANDLER,     0, DA_386IGate

%rep  10
    Gate SELECTOR_CODE32, SPURIOUS_HANDLER,     0, DA_386IGate
%endrep

.2CH:
    Gate SELECTOR_CODE32, MOUSE_HANDLER,        0, DA_386IGate

; 系统调用终端，具体的使用方法看system_call函数
.2DH:
    Gate SELECTOR_CODE32, SYSTEM_CALL_HANDLER,   0, DA_386IGate

IDT_LEN  equ $ - LABEL_IDT
IDT_PTR  dw  IDT_LEN - 1 ; 因为段描述符总是8字节长，因此GDT的限长值应该设置成总是8的倍数减1（即8N-1）
         dd  0

; 在能够切换到保护模式之前，软件初始化代码还必须设置以下系统寄存器：
;   - 全局描述符表基地址寄存器GDTR；
;   - 中断描述符表基地址寄存器IDTR；
;   - 控制寄存器CR1--CR3；
; 在初始化了这些数据结构、代码模块和系统寄存器之后，通过设置CR0寄存器的保护模式标志PE（位0），处理器就可以切换到保护模式下运行。

; 实模式代码
[SECTION  .s16]
[BITS  16]
LABEL_BEGIN:
    mov   ax, cs
    mov   ds, ax
    mov   es, ax
    mov   ss, ax
    mov   sp, 0100h

    ; 获取内存描述块
COMPUTE_MEMORY:
    mov   ebx, 0
    mov   di, MEM_CHK_BUF
.compute_memory_loop:
    mov   eax, 0E820h
    mov   ecx, 20
    mov   edx, 0534D4150h
    int   15h
    jc    LABEL_MEM_CHK_FAIL
    add   di, 20
    inc   dword [MEMORY_CHK_NUMBER]
    cmp   ebx, 0
    jne   .compute_memory_loop
    jmp   LABEL_MEM_CHK_OK

LABEL_MEM_CHK_FAIL:
    mov    dword [MEMORY_CHK_NUMBER], 0

LABEL_MEM_CHK_OK:
    ; 设置分辨率
    %include "resolution.asm"

    ; 初始花8259A芯片，开启中断处理
    call init8259A

    ; 设置代码描述符
    xor   eax, eax
    mov   ax,  cs
    shl   eax, 4
    add   eax, LABEL_SEG_CODE32
    mov   word [LABEL_DESC_CODE32 + 2], ax
    shr   eax, 16
    mov   byte [LABEL_DESC_CODE32 + 4], al
    mov   byte [LABEL_DESC_CODE32 + 7], ah

    ; 设置字体描述符
    xor   eax, eax
    mov   ax,  cs
    shl   eax, 4
    add   eax, LABEL_SYSTEM_FONT
    mov   word [LABEL_DESC_FONT + 2], ax
    shr   eax, 16
    mov   byte [LABEL_DESC_FONT + 4], al
    mov   byte [LABEL_DESC_FONT + 7], ah

    ; 设置堆栈描述符
    xor eax, eax
    mov ax, cs
    shl eax, 4
    add eax, LABEL_STACK
    mov word [LABEL_DESC_STACK + 2], ax
    shr eax, 16
    mov byte [LABEL_DESC_STACK + 4], al
    mov byte [LABEL_DESC_STACK + 7], ah

    ; 准备加载中断描述表
    xor   eax, eax
    mov   ax, ds
    shl   eax, 4
    add   eax,  LABEL_GDT
    mov   dword  [GDT_PTR + 2], eax

    lgdt  [GDT_PTR] ; 加载中断描述表

    cli   ;关中断

    ; 加载中断描述符
    xor   eax, eax
    mov   ax,  ds
    shl   eax, 4
    add   eax, LABEL_IDT
    mov   dword [IDT_PTR + 2], eax
    lidt  [IDT_PTR]

    in    al,  92h
    or    al,  00000010b
    out   92h, al

    ; 1. 开启保护模式
    ; CR0的位0是启用保护（Protection Enable）标志。
    ; 当设置该位时即开启了保护模式；当复位时即进入实地址模式。
    ; 这个标志仅开启段级保护，而并没有启用分页机制。
    ; 2. 若要启用分页机制，那么PE和PG标志都要置位CR0的位31是分页（Paging）标志。
    ; 当设置该位时即开启了分页机制；当复位时则禁止分页机制，此时所有线性地址等同于物理地址。
    ; 在开启这个标志之前必须已经或者同时开启PE标志。即若要启用分页机制，那么PE和PG标志都要置位。
    ; 所以这里并没有启用分页机制
    ;   a. 如果PE=0、PG=0，处理器工作在实地址模式下；
    ;   b. 如果PG=0、PE=1，处理器工作在没有开启分页机制的保护模式下；
    ;   c. 如果PG=1、PE=0，此时由于不在保护模式下不能启用分页机制，
    ;      因此处理器会产生一个一般保护异常，即这种标志组合无效；
    ;   d. 如果PG=1、PE=1，则处理器工作在开启了分页机制的保护模式下。
    ; 3. CR2和CR3用于分页机制。CR3含有存放页目录表页面的物理地址，因此CR3也被称为PDBR。
    ; 因为页目录表页面是页对齐的，所以该寄存器只有高20位是有效的。
    ; 而低12位保留供更高级处理器使用，因此在往CR3中加载一个新值时低12位必须设置为0。
    mov   eax, cr0
    or    eax , 1
    mov   cr0, eax

    ; 跳转到保护模式代码执行
    ; 在修改该了PE位之后程序必须立刻使用一条跳转指令，
    ; 以刷新处理器执行管道中已经获取的不同模式下的任何指令。
    ; 在设置PE位之前，程序必须初始化几个系统段和控制寄存器。
    ; 在系统刚上电时，处理器被复位成PE=0、PG=0（即实模式状态），
    ; 以允许引导代码在启用分段和分页机制之前能够初始化这些寄存器和数据结构
    jmp   dword  SELECTOR_CODE32: 0
    ; jmp   dword  1*8: 0

    %include "init_8259A.asm"

; 实模式数据
; 保存内存块描述
; 这个代码必须放在代码前
; 因为在实模式下会保存内存信息到`MEM_CHK_BUF`中
; 如果代码长度很大就会超过实模式能访问的最大距离, 导致无法启动镜像
[SECTION .s16.data]
[BITS 16]
MEM_CHK_BUF: times 256 db 0
MEMORY_CHK_NUMBER: dd 0
BOOT_INFO: times 3 dd 0

; 保护模式代码
[SECTION .s32]
[BITS  32]
LABEL_SEG_CODE32:
    ; 初始化堆栈段
    mov   ax, SELECTOR_STACK
    mov   ss, ax
    mov   esp, TOP_OF_STACK_MAIN

    ; 初始化数据段
    mov   ax, SELECTOR_VRAM
    mov   ds, ax

    ; 初始化显存段
    mov   ax, SELECTOR_VIDEO
    mov   gs, ax

    sti ; 开中断

C_CODE_ENTRY:
    call start_kernel
    %include "io.asm"
    %include "handler.asm"
    %include "build/ckernel.asm"

SEG_CODE32_LEN  equ  $ - LABEL_SEG_CODE32

; 保护模式数据
[SECTION .s32.ds]
ALIGN 32
[BITS 32]
LABEL_STACK:
    times TASK_COUNTS * TASK_STACK_SIZE db 0 ; 分配1024字节作为1个任务的堆栈，一共分配6个

TOP_OF_STACK_MAIN equ TASK_STACK_SIZE ; 分配第1个1024字节作为主任务的堆栈
LEN_OF_STACK_SECTION equ $ - LABEL_STACK

LABEL_SYSTEM_FONT:
    %include "font_data.inc"

SYSTEM_FONT_LENGTH equ $ - LABEL_SYSTEM_FONT

EOF_KERNEL:
    db "EOF KERNEL", 0
