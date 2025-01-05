%include "ppm.inc"

[SECTION .s32]
BITS 32
call main ; 调用C语言反汇编的代码
retf

%include "api.asm"
; entry.asm End of file

