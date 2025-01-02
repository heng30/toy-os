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
    mov edi, [esp + 4] ;char
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

; farjmp:
;     jmp FAR [esp + 4] ; 获取到第一个参数的值
;     ret

farjmp:
    ; 保存跳转之前的eip值到0x6000，外部任务执行完后通过这个地址返回
    xor  eax, eax
    mov  eax, [esp]
    mov  [CALL_EXTERNAL_BIN_SAVE_EIP_ADDR], eax
    jmp FAR [esp + 4]
    ret

call_external_bin_save_eip_addr:
    mov eax, CALL_EXTERNAL_BIN_SAVE_EIP_ADDR
    ret
