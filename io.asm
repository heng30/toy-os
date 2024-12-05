io_hlt:
  HLT
  RET

io_cli:
  CLI
  RET

io_sti:
  STI
  RET

io_stihlt:
  STI
  HLT
  RET

io_in8:
  mov  edx, [esp + 4]
  mov  eax, 0
  in   al, dx

io_in16:
  mov  edx, [esp + 4]
  mov  eax, 0
  in   ax, dx

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
    pushfd ; 将eflags寄存器的值存入堆栈
    pop  eax
    ret

io_store_eflags:
    mov eax, [esp + 4]
    push eax
    popfd ; 从堆栈中获取一个值，并赋值给eflags寄存器
    ret

debug_char:
    mov  ah, 0Ch
    mov  al, 'U'
    mov  [gs:((80 * 0 + 67) * 2)], ax
    ret
