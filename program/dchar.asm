[SECTION .s32]
BITS 32

entry:
    mov edx, 1000
    mov al, 'A'
    int 0x2d

    mov edx, 1
    int 0x2d
