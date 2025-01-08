%include "ppm.inc"

[SECTION .s32]
BITS 32
call main ; 调用C语言反汇编的代码

; 结束命令返回到内核代码
mov edx, SYSTEM_CALL_END_CMD
int SYSTEM_CALL_INT

; 打印错误信息
mov edx, SYSTEM_CALL_CONSOLE_DRAW_CH
mov al, 'E'
int SYSTEM_CALL_INT

fin:
    jmp fin

%include "api.asm"
; entry.asm End of file

