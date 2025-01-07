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

get_esp:
    mov eax ,esp
    ret

get_ss:
    xor eax, eax
    mov eax, ss
    ret

; 调用外部命令. 函数参数: eip, cs, esp, ds, &esp0
start_cmd:
    pushad              ; 8个通用寄存器入栈

    ; 保存内核esp到tss->m_esp0,
    ; 让cpu从用户态切换到内核态能够自动实现esp0->esp, ss0->ss的寄存器赋值
    mov eax, [esp + 52] ; 获取&esp0的值
    mov [eax], esp
    mov [eax + 4], ss   ; 在TSS32_t结构中esp0的下一个成员是ss0

    ; 因为堆栈上第一个值是eip，然后是8个通用寄存器
    mov eax, [esp + 36]  ; eip
    mov ecx, [esp + 40]  ; cs
    mov edx, [esp + 44]  ; esp
    mov ebx, [esp + 48]  ; ds

    ; 切换到用户态数据段
    mov  ds,  bx
    mov  es,  bx

    ; 进行特权级切换，从0->3
    or ecx, 3   ; cs
    or ebx, 3   ; ds

    push ebx    ; ds
    push edx    ; esp
    push ecx    ; cs
    push eax    ; eip

    ; retf的作用是从堆栈中弹出两个4字节，作为eip和cs, 并跳转到cs:eip
    ; 如果发现cs是一个特权级调整，会继续从堆栈中弹出两个4字节作为esp和ds
    retf

kill_cmd:
    sti
    mov eax, [esp + 4]
    mov esp, [eax]
    mov DWORD [eax + 4], 0  ; 重置tss->m_ss0
    popad                   ; 对应start_cmd函数开头的pushad
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
