[SECTION .s32]
BITS 32

entry:
    mov eax, 1*8
    mov ss, ax ; 这条命令会引发一个异常
    mov edx, 1
    int 0x2d
