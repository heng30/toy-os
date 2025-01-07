[SECTION .s23]
[BITS 32]

entry:
    ; 通过系统调用返回到内核代码中
    mov edx, 1
    int 0x2d
