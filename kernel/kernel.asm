%include "pm.inc"

org   0x8000 ; 内核代码开始执行的地址，也是编译时代码相对偏移地址

TASK_COUNTS equ 10
TASK_STACK_SIZE equ 1024

jmp   LABEL_BEGIN

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

; 进程切换相关
LABEL_DESC_6:       Descriptor        0,            0fffffh,                0409Ah

; 创建6个TSS32结构，用户保存任务相关寄存器
%rep TASK_COUNTS
                    Descriptor        0,            0,                      0 ; 任务的TSS32对象
%endrep

GDT_LEN     equ    $ - LABEL_GDT
GDT_PTR     dw     GDT_LEN - 1
            dd     0            ; 这里会保存LABEL_GDT的地址

SELECTOR_CODE32    equ   LABEL_DESC_CODE32 -  LABEL_GDT
SELECTOR_VIDEO     equ   LABEL_DESC_VIDEO  -  LABEL_GDT
SELECTOR_STACK     equ   LABEL_DESC_STACK  -  LABEL_GDT
SELECTOR_VRAM      equ   LABEL_DESC_VRAM   -  LABEL_GDT
SELECTOR_FONT      equ   LABEL_DESC_FONT   -  LABEL_GDT

; 中断描述符
LABEL_IDT:
%rep  32
    Gate SELECTOR_CODE32, SPURIOUS_HANDLER, 0, DA_386IGate
%endrep

.020h:
    Gate SELECTOR_CODE32, TIMER_HANDLER,    0, DA_386IGate

.021h:
    Gate SELECTOR_CODE32, KEYBOARD_HANDLER, 0, DA_386IGate

%rep  10
    Gate SELECTOR_CODE32, SPURIOUS_HANDLER, 0, DA_386IGate
%endrep

.2CH:
    Gate SELECTOR_CODE32, MOUSE_HANDLER,    0, DA_386IGate

IDT_LEN  equ $ - LABEL_IDT
IDT_PTR  dw  IDT_LEN - 1
         dd  0

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
    db "EOF KERNEL"
