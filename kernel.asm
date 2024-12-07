%include "pm.inc"

org   0x9000

VRAM_ADDRESS  equ  0x000a0000

jmp   LABEL_BEGIN

[SECTION .gdt]
 ;                                  段基址          段界限                  属性
LABEL_GDT:          Descriptor        0,            0,                      0
LABEL_DESC_CODE32:  Descriptor        0,            SEG_CODE32_LEN - 1,     DA_C + DA_32
LABEL_DESC_VIDEO:   Descriptor        0B8000h,      0ffffh,                 DA_DRW
LABEL_DESC_VRAM:    Descriptor        0,            0ffffffffh,             DA_DRW
LABEL_DESC_STACK:   Descriptor        0,            TOP_OF_STACK,           DA_DRWA+DA_32

GdtLen     equ    $ - LABEL_GDT
GdtPtr     dw     GdtLen - 1
           dd     0

SELECTOR_CODE32    equ   LABEL_DESC_CODE32 -  LABEL_GDT
SELECTOR_VIDEO     equ   LABEL_DESC_VIDEO  -  LABEL_GDT
SELECTOR_STACK     equ   LABEL_DESC_STACK  -  LABEL_GDT
SELECTOR_VRAM      equ   LABEL_DESC_VRAM   -  LABEL_GDT

; 中断描述符
LABEL_IDT:
%rep  33
    Gate  SELECTOR_CODE32, SPURIOUS_HANDLER, 0, DA_386IGate
%endrep

.021h:
    Gate SELECTOR_CODE32, KEYBOARD_HANDLER, 0, DA_386IGate

%rep  10
    Gate  SELECTOR_CODE32, SPURIOUS_HANDLER, 0, DA_386IGate
%endrep

.2CH:
    Gate SELECTOR_CODE32, MOUSE_HANDLER, 0, DA_386IGate

IdtLen  equ $ - LABEL_IDT
IdtPtr  dw  IdtLen - 1
        dd  0

[SECTION  .s16]
[BITS  16]
LABEL_BEGIN:
    mov   ax, cs
    mov   ds, ax
    mov   es, ax
    mov   ss, ax
    mov   sp, 0100h

    ; 设置内存
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
    ; 设置色彩显示模式
    mov   al, 0x13
    mov   ah, 0
    int   0x10

    ; 初始花8259A芯片，开启中断处理
    call init8259A

    xor   eax, eax
    mov   ax,  cs
    shl   eax, 4
    add   eax, LABEL_SEG_CODE32
    mov   word [LABEL_DESC_CODE32 + 2], ax
    shr   eax, 16
    mov   byte [LABEL_DESC_CODE32 + 4], al
    mov   byte [LABEL_DESC_CODE32 + 7], ah

    ; 设置堆栈描述符
    xor eax, eax
    mov ax, cs
    shl eax, 4
    add eax, LABEL_STACK
    mov word [LABEL_DESC_STACK + 2], ax
    shr eax, 16
    mov byte [LABEL_DESC_STACK + 4], al
    mov byte [LABEL_DESC_STACK + 7], ah

    xor   eax, eax
    mov   ax, ds
    shl   eax, 4
    add   eax,  LABEL_GDT
    mov   dword  [GdtPtr + 2], eax

    lgdt  [GdtPtr]

    cli   ;关中断

    ; prepare for loading IDT
    xor   eax, eax
    mov   ax,  ds
    shl   eax, 4
    add   eax, LABEL_IDT
    mov   dword [IdtPtr + 2], eax
    lidt  [IdtPtr]

    in    al,  92h
    or    al,  00000010b
    out   92h, al

    mov   eax, cr0
    or    eax , 1
    mov   cr0, eax

    jmp   dword  SELECTOR_CODE32: 0

    %include "init_8259A.asm"

    [SECTION .s32]
    [BITS  32]
LABEL_SEG_CODE32:
    ; 初始化堆栈
    mov   ax, SELECTOR_STACK
    mov   ss, ax
    mov   esp, TOP_OF_STACK

    mov   ax, SELECTOR_VRAM
    mov   ds, ax

    mov   ax, SELECTOR_VIDEO
    mov   gs, ax

    sti ; 开中断

C_CODE_ENTRY:
    %include "entry.asm"

    jmp  $ ; 循环

HANDLER_CODE:
    %include "handler.asm"

IO_CODE:
    %include "io.asm"

RES_DATA:
    %include "font_data.inc"
    %include "kernel/build/cursor_icon.asm"

SEG_CODE32_LEN  equ  $ - LABEL_SEG_CODE32

MEM_CHK_BUF: times 256 db 0
MEMORY_CHK_NUMBER: dd 0

[SECTION .gs]
ALIGN 32
[BITS 32]
LABEL_STACK:
    times 512 db 0 ; 分配512字节的堆栈

TOP_OF_STACK equ $ - LABEL_STACK

EOF_KERNEL:
    db "EOF KERNEL"
