SYSTEM_CALL_INT equ 0x2d

; 非法系统调用
SYSTEM_CALL_INVALID equ 0

; 结束外部命令，返回到内核态
SYSTEM_CALL_END_CMD equ 1

; 需要终端打印的字符放到al寄存器中
SYSTEM_CALL_CONSOLE_DRAW_CH equ 1000

; 需要终端打印的字符串放到ebx寄存器中
SYSTEM_CALL_CONSOLE_DRAW_TEXT equ 1001
