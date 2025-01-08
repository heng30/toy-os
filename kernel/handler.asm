_SPURIOUS_HANDLER:
SPURIOUS_HANDLER  equ _SPURIOUS_HANDLER - $$
    iretd

_KEYBOARD_HANDLER:
KEYBOARD_HANDLER equ _KEYBOARD_HANDLER - $$
    push es
    push ds
    push fs
    push gs
    pushad
    mov  eax, esp
    push eax

    call int_handler_from_c

    pop  eax
    mov  esp, eax
    popad
    pop gs
    pop fs
    pop ds
    pop es
    iretd

_MOUSE_HANDLER:
MOUSE_HANDLER equ _MOUSE_HANDLER - $$
    push es
    push ds
    push fs
    push gs
    pushad
    mov  eax, esp
    push eax

    call int_handler_for_mouse

    pop  eax
    mov  esp, eax
    popad
    pop gs
    pop fs
    pop ds
    pop es
    iretd

; 时钟中断处理函数
; 中断会触发内核特权级的切换3->0, [ss0:esp0]->[ss:esp]
_TIMER_HANDLER:
TIMER_HANDLER equ _TIMER_HANDLER - $$
    push es
    push ds
    pushad
    mov  eax, esp
    push eax

    ; 切换用户态数据段到内核数据段
    mov  ax, SELECTOR_VRAM
    mov  ds, ax
    mov  es, ax

    call int_handler_for_timer

    pop eax
    popad
    pop ds
    pop es
    iretd

; 外部命令使用超出数据段大小的内存会调用该中断函数
; 中断会触发内核特权级的切换3->0, [ss0:esp0]->[ss:esp]
_STACK_OVERFLOW_HANDLER:
STACK_OVERFLOW_HANDLER equ _STACK_OVERFLOW_HANDLER - $$
    sti
    push es
    push ds
    pushad

    ; int_handler_for_stack_overflow函数参数
    mov eax, esp
    push eax

    ; 把数据段切换到内核数据段
    mov  ax, SELECTOR_VRAM
    mov  ds, ax
    mov  es, ax

    call int_handler_for_stack_overflow

    jmp near end_cmd

; 外部命令出错会调用该中断函数
; 中断会触发内核特权级的切换3->0, [ss0:esp0]->[ss:esp]
_EXCEPTION_HANDLER:
EXCEPTION_HANDLER equ _EXCEPTION_HANDLER - $$
    sti
    push es
    push ds
    pushad

    ; int_handler_for_exception函数参数
    mov eax, esp
    push eax

    ; 把数据段切换到内核数据段
    mov  ax, SELECTOR_VRAM
    mov  ds, ax
    mov  es, ax

    call int_handler_for_exception

    ; 从异常中恢复，并结束外部命令
    jmp near end_cmd

; 系统调用中断处理函数
; 中断会触发内核特权级的切换3->0, [ss0:esp0]->[ss:esp]
_SYSTEM_CALL_HANDLER:
SYSTEM_CALL_HANDLER equ _SYSTEM_CALL_HANDLER - $$
    sti
    push ds
    push es
    pushad  ; 为了从system_call_api返回后能够正确恢复寄存器
    pushad  ; 为了给system_call_api传递参数

    ; 把数据段切换到内核数据段
    mov  ax, SELECTOR_VRAM
    mov  ds, ax
    mov  es, ax

    call system_call_api

    ; 通过system_call_api的返回值来判断是否结束外部命令
    ; 返回值应该是保存了esp的一个地址
    cmp eax, 0
    jne end_cmd    ; 不为0则结束外部命令

    ; 恢复寄存器，并返回到外部命令继续执行
    popad
    popad
    pop es
    pop ds
    iretd

end_cmd:
    mov esp, [eax]  ; 从system_call_api中获取到esp的值
    popad           ; 这个popad对应的是start_cmd函数开头的pushad
    ret             ; 模拟外部命令从start_cmd函数正常返回
