_SPURIOUS_HANDLER:
SPURIOUS_HANDLER  equ _SPURIOUS_HANDLER - $$
    iretd

_KEYBOARD_HANDLER:
KEYBOARD_HANDLER equ _KEYBOARD_HANDLER - $$
    push es
    push ds
    pushad
    mov  eax, esp
    push eax

    call int_handler_from_c

    pop  eax
    mov  esp, eax
    popad
    pop  ds
    pop  es
    iretd


_MOUSE_HANDLER:
MOUSE_HANDLER equ _MOUSE_HANDLER - $$
    push es
    push ds
    pushad
    mov  eax, esp
    push eax

    call int_handler_for_mouse

    pop  eax
    mov  esp, eax
    popad
    pop  ds
    pop  es
    iretd

; _TIMER_HANDLER:
; TIMER_HANDLER equ _TIMER_HANDLER - $$
;     push es
;     push ds
;     pushad
;     mov  eax, esp
;     push eax

;     call int_handler_for_timer

;     pop  eax
;     mov  esp, eax
;     popad
;     pop  ds
;     pop  es
;     iretd

_TIMER_HANDLER:
TIMER_HANDLER equ _TIMER_HANDLER - $$
    push es
    push ds
    pushad

    ; 定时器中断时是否运行在内核态
    mov  ax, ss
    cmp  ax, SELECTOR_STACK     ; 内核堆栈段
    jne  .from_cmd
    ; 以上代码还运行在外部命令态

    ; 运行在内核态，不需要进行段寄存器切换
    push fs
    push gs
    call int_handler_for_timer
    pop gs
    pop fs
    popad
    pop ds
    pop es
    iretd

    ; 运行在外部命令态，需要保存外部命令段寄存器，并进行切换
    .from_cmd:
        ; 恢复内核态的段寄存器
        mov  ax, SELECTOR_VRAM
        mov  ds, ax
        mov  es, ax
        mov  ecx, [KERNEL_ESP]       ; 获取内核esp

        mov  bx, ss
        mov  edx, esp

        ; 切换到内核堆栈段
        mov  ax, SELECTOR_STACK
        mov  ss, ax
        mov  esp, ecx           ; 更新内核esp

        ; 保存外部命令段寄存器
        push bx
        push edx

        call int_handler_for_timer

        ; 恢复外部命令段寄存器
        pop  edx                ; 外部命令esp
        pop  ebx                ; 外部命令ss
        mov  ss, bx
        mov  esp, edx

        ; 弹出函数刚开始入栈的10各寄存器
        popad
        pop  ds
        pop  es
        iretd

; 系统调用中断处理函数
; _SYSTEM_CALL_HANDLER:
; SYSTEM_CALL_HANDLER equ _SYSTEM_CALL_HANDLER - $$
;         pushad ; 保存8个寄存器参数

;         ; 为调用system_call_api传递8个寄存器参数
;         pushad
;         call system_call_api
;         add esp, 32 ; 弹出system_call_api的8个函数参数

;         popad ; 恢复8个寄存器参数
;         iretd

; 系统调用中断处理函数
_SYSTEM_CALL_HANDLER:
SYSTEM_CALL_HANDLER equ _SYSTEM_CALL_HANDLER - $$
    push ds
    push es
    pushad ; 保存8个寄存器参数
    ; 以上代码还运行在外部命令态

    ; 切换到内核数据段
    mov  ax, SELECTOR_VRAM
    mov  ds, ax
    mov  es, ax
    mov  ecx, [KERNEL_ESP]           ; 获取内核esp

    ; 因为gdt表中栈描述符的基地址是LABEL_STACK
    ; 而数据栈描述符的基地址是0,
    ; 所以想要通过通过内存访问操作栈上的数据需要esp加上LABEL_STACK
    ; 此时[ds:ecx]上的位置正好等于[ss:esp]
    add  ecx, LABEL_STACK

    ; 将函数开始push了10各参数，占用40各字节. 复制到内核堆栈上
    ; 内核堆栈从低地址到高地址： edi, esi, ebp, esp, ebx, edx, ecx, eax, esp, ss
    add  ecx, -40               ; 将更新后的esp保存到ecx寄存器中
    mov  [ecx + 32], esp
    mov  [ecx + 36], ss

    ; 将pushad 压入到外部命令堆栈的值复制到内核堆栈
    mov edx, [esp]
    mov ebx, [esp + 4]
    mov [ecx], edx      ; edi
    mov [ecx + 4], ebx  ; esi

    mov edx, [esp + 8]
    mov ebx, [esp + 12]
    mov [ecx + 8], edx  ; ebp
    mov [ecx + 12], ebx ; esp

    mov edx, [esp + 16]
    mov ebx, [esp + 20]
    mov [ecx + 16], edx ; ebx
    mov [ecx + 20], ebx ; edx

    mov edx, [esp + 24]
    mov ebx, [esp + 28]
    mov [ecx + 24], edx ; ecx
    mov [ecx + 28], ebx ; eax

    ; 切换到内核堆栈段
    mov  ax, SELECTOR_STACK
    mov  ss, ax

    ; 更新esp
    sub  ecx, LABEL_STACK
    mov  esp, ecx
    sti                 ; 开中断

    ; 进行系统调用
    call system_call_api

    ; 执行完系统调用后，把数据段和堆栈段重新切回到外部命令
    mov  ecx, [esp + 32]    ; 恢复外部命令的esp
    mov  eax, [esp + 36]    ; 恢复外部命令的ss
    cli                     ; 关中断
    mov  ss, ax
    mov  esp, ecx

    ; 弹出函数开头保存的10各寄存器
    popad
    pop  es
    pop  ds
    sti                 ; 开中断
    iretd

; 外部命令出错会调用该中断函数
_EXCEPTION_HANDLER:
EXCEPTION_HANDLER equ _EXCEPTION_HANDLER - $$
    cli
    mov  ax, SELECTOR_VRAM  ; 数据段切换到内核
    mov  ds, ax
    mov  es, ax
    mov  ax, SELECTOR_STACK ; 切换到内核堆栈段
    mov  ss, ax
    mov  esp, [KERNEL_ESP]   ; 获取内核堆栈指针

    ; 当作函数参数
    push ss
    push esp
    call int_handler_for_exception

    ; 通过把CPU交给内核的方式直接杀掉应用程序
    mov  esp, [KERNEL_ESP]
    sti

    ; start_cmd函数的开头有一个pushad的指令
    ; 所以这里需要使用popad弹出栈中的数据
    ; 这里使用ret是模拟start_cmd因为异常没有走往的返回流程
    popad
    ret
