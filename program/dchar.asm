%include "include/ppm.inc"

[SECTION .s32]
BITS 32

entry:
    mov edx, SYSTEM_CALL_CONSOLE_DRAW_CH
    mov al, 'A'
    int SYSTEM_CALL_INT
    retf
