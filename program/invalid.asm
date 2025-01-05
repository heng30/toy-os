[SECTION .s32]
BITS 32

entry:
    mov edx, 0
    int 0x2d
    retf
