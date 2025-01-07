[SECTION .s23]
[BITS 32]

; 什么也不做，直接跳回到终端任务
entry:
    ; 通过系统调用放回到内核代码中
    mov edx, 1
    int 0x2d

