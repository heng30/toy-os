%include "include/ppm.inc"

jmp entry

[SECTION .data]
ALIGN 32
[BITS 32]
msg: db "hi from rhlt.exe", 0

[map symbols rhlt.map]
[SECTION .text]
BITS 32

; 代码有问题，内核无法输出字符串
entry:
    ; 复制字符串
    push esi
    push edi
    mov esi, msg
    mov edi, EXTERNAL_BIN_AND_KERNEL_SHARED_MEMORY

_loop:
    mov al, [esi]
    mov [edi], al
    inc esi
    inc edi
    cmp al, 0
    jne _loop

    mov [edi], al

    pop edi
    pop esi

    ; 返回到调用该外部命令的任务中
    push CODE32_TR * 8
    mov  eax, [CALL_EXTERNAL_BIN_SAVE_EIP_ADDR]
    push eax
    retf

