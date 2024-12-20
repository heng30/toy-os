%include "pm.inc"

org   0x8000

TASK_COUNTS equ 6
TASK_STACK_SIZE equ 1024

jmp   LABEL_BEGIN

[SECTION .gdt]
 ;                                  段基址          段界限                  属性
LABEL_GDT:          Descriptor        0,            0,                      0
LABEL_DESC_CODE32:  Descriptor        0,            0fffffh,                DA_C | DA_32 | DA_LIMIT_4K
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
            dd     0

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

    lgdt  [GDT_PTR]

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

    mov   eax, cr0
    or    eax , 1
    mov   cr0, eax

    jmp   dword  SELECTOR_CODE32: 0
    ; jmp   dword  1*8: 0

    %include "init_8259A.asm"

; 保存内存块描述
; 这个代码必须放在代码前
; 因为在实模式下会保存内存信息到`MEM_CHK_BUF`中
; 如果代码长度很大就会超过实模式能访问的最大距离, 导致无法启动镜像
[SECTION .data]
ALIGN 32
[BITS 32]
MEM_CHK_BUF: times 256 db 0
MEMORY_CHK_NUMBER: dd 0
BOOT_INFO: times 3 dd 0

[SECTION .s32]
[BITS  32]
LABEL_SEG_CODE32:
    ; 初始化堆栈
    mov   ax, SELECTOR_STACK
    mov   ss, ax
    mov   esp, TOP_OF_STACK_MAIN

    mov   ax, SELECTOR_VRAM
    mov   ds, ax

    mov   ax, SELECTOR_VIDEO
    mov   gs, ax

    sti ; 开中断

C_CODE_ENTRY:
    call start_kernel
    %include "io.asm"
    %include "handler.asm"
    %include "build/ckernel.asm"

SEG_CODE32_LEN  equ  $ - LABEL_SEG_CODE32

[SECTION .gs]
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
