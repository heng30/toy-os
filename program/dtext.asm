%include "include/ppm.inc"

[SECTION .s32]
BITS 32

entry:
    mov edx, SYSTEM_CALL_CONSOLE_DRAW_TEXT
    mov ebx, msg
    int SYSTEM_CALL_INT
    retf

[SECTION .data]
ALIGN 32
[BITS 32]
msg: db "hi from dtext.exe", 0
