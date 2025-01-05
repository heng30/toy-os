[SECTION .s32]
BITS 32

entry:
    mov edx, 1001
    mov ebx, msg
    int 0x2d
    retf

[SECTION .data]
ALIGN 32
[BITS 32]
msg: db "hi from dtext.exe", 0
