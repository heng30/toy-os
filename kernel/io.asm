io_hlt:
  hlt
  ret

io_cli:
  cli
  ret

io_sti:
  sti
  ret

io_stihlt:
  sti
  hlt
  ret

io_in8:
  mov  edx, [esp + 4]
  mov  eax, 0
  in   al, dx
  ret

io_in16:
  mov  edx, [esp + 4]
  mov  eax, 0
  in   ax, dx
  ret

io_in32:
  mov edx, [esp + 4]
  in  eax, dx
  ret

io_out8:
   mov edx, [esp + 4]
   mov al, [esp + 8]
   out dx, al
   ret

io_out16:
   mov edx, [esp + 4]
   mov eax, [esp + 8]
   out dx, ax
   ret

io_out32:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret

io_load_eflags:
    pushfd
    pop  eax
    ret

io_store_eflags:
    mov eax, [esp + 4]
    push eax
    popfd
    ret

io_delay:
    nop
    nop
    nop
    nop
    ret

debug_char:
    mov  ah, 0Ch
    mov  al, 'U'
    mov  [gs:((80 * 0 + 67) * 2)], ax
    ret

get_memory_block_count:
    mov  eax, [MEMORY_CHK_NUMBER]
    ret

get_memory_block_buffer:
    mov  eax, MEM_CHK_BUF
    ret

get_boot_info:
    mov eax, BOOT_INFO
    ret

get_font_data:
    mov ax, SELECTOR_FONT
    mov es, ax
    xor edi, edi
    mov edi, [esp + 4]
    shl edi, 4
    add edi, [esp + 8]
    xor eax, eax
    mov al, byte [es:edi]
    ret

get_addr_gdt:
    mov  eax, LABEL_GDT
    ret

get_code32_addr:
    mov  eax, LABEL_SEG_CODE32
    ret

get_stack_start_addr:
    mov  eax, LABEL_STACK
    ret

; 指令LTR和STR分别用于加载和保存TR寄存器的段选择符部分。
; 当使用LTR指令把选择符加载进任务寄存器时，
; TSS描述符中的段基地址、段限长度以及描述符属性会被自动地加载到任务寄存器中。
; 当执行任务切换时，处理器会把新任务的TSS的段选择符和段描述符自动地加载进任务寄存器TR中
load_tr:
    ltr  [esp + 4]
    ret

taskswitch6:
    jmp  6*8:0
    ret

taskswitch7:
    jmp  7*8:0
    ret

taskswitch8:
    jmp  8*8:0
    ret

taskswitch9:
    jmp 9*8:0
    ret

farjmp:
    mov  eax, [esp]
    push 1*8    ; 保存代码段描述符偏移
    push eax    ; 保存跳转前任务下一条要执行的代码位置, eip寄存器值
    jmp FAR [esp + 12]  ; 跳过2个push指令值和eip值，[esp+12]就是参数的传入值
    ret

; 调用外部命令：
;   - 保存当前任务的寄存器
;   - 切换到外部任务数据段
start_cmd:
    cli                 ; 关中断
    pushad              ; 保存8个通用寄存器, 占用32字节
    mov eax, [esp + 36] ; eip, 这里32+4,其中4字节是返回地址eip
    mov ecx, [esp + 40] ; cs
    mov edx, [esp + 44] ; esp
    mov ebx, [esp + 48] ; ds

    ; 切换到外部命令的段描述符，并进行寄存器切换
    mov  [0xfe4], esp   ; 保存当前任务esp，从外部命令返回后需要使用
    mov  ds,  bx        ; ds
    mov  ss,  bx        ; ds
    mov  esp, edx       ; esp
    sti                 ; 开中断
    push ecx            ; cs
    push eax            ; eip

    call far [esp]      ; 跳转到外部命令代码去执行

    ; 切换到内核任务的段描述符，并进行寄存器切换
    mov  ax, SELECTOR_VRAM
    mov  ds, ax
    mov  esp, [0xfe4]   ; 恢复esp
    mov  ax, SELECTOR_STACK
    mov  ss, ax

    popad               ; 弹出8个通用寄存器
    ret

io_copy_msg:
    ; 获取函数参数
    mov edi, [esp + 4] ; dst
    mov esi, [esp + 8] ; src

    ; 复制字符串
    _io_copy_msg_loop:
        mov al, [esi]
        mov [edi], al
        inc esi
        inc edi
        cmp al, 0
        jne _io_copy_msg_loop

    ; 复制结束符
    mov [edi], al

    ret
