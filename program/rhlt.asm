%include "include/ppm.inc"

jmp entry

[SECTION .text]
[BITS 32]

entry:
    call copy_msg

    ; 返回到调用该外部命令的任务中
    push CODE32_TR * 8
    mov  eax, [CALL_EXTERNAL_BIN_SAVE_EIP_ADDR]
    push eax
    retf

; 代码有问题，无法复制字符串到EXTERNAL_BIN_AND_KERNEL_SHARED_MEMORY
; 可能是权限的问题，可以读取内容，但无法写入内容
copy_msg:
    ; 复制字符串
    push esi
    push edi
    mov esi, msg
    mov edi, EXTERNAL_BIN_AND_KERNEL_SHARED_MEMORY

    _copy_msg_loop:
        mov al, [esi]
        mov [edi], al
        inc esi
        inc edi
        cmp al, 0
        jne _copy_msg_loop

    mov [edi], al

    pop edi
    pop esi
    ret

[SECTION .data]
ALIGN 32
[BITS 32]
msg: db "hi from rhlt.exe", 0
