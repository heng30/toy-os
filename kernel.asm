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
%rep  255
    Gate  SELECTOR_CODE32, SPURIOUS_HANDLER, 0, DA_386IGate
%endrep

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

     设置色彩显示模式
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

_SPURIOUS_HANDLER:
SPURIOUS_HANDLER  equ _SPURIOUS_HANDLER - $$
    push es
    push ds
    pushad
    mov  eax, esp
    push eax

    call int_handler_from_c

    pop  eax
    mov  esp, eax
    popad
    pop  ds
    pop  es
    sti

    iretd

IO_CODE:
    %include "io.asm"

RES_DATA:
    %include "font_data.inc"
    %include "kernel/build/cursor_icon.asm"

SEG_CODE32_LEN  equ  $ - LABEL_SEG_CODE32

[SECTION .gs]
ALIGN 32
[BITS 32]
LABEL_STACK:
    times 512 db 0 ; 分配512字节的堆栈

TOP_OF_STACK equ $ - LABEL_STACK

EOF_KERNEL:
    db "EOF KERNEL"
