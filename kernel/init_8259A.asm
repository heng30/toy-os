init8259A:
    mov  al, 011h
    out  020h, al
    call io_delay

    out 0A0h, al
    call io_delay

    mov al, 020h
    out 021h, al
    call io_delay

    mov  al, 028h
    out  0A1h, al
    call io_delay

    mov  al, 004h
    out  021h, al
    call io_delay

    mov  al, 002h
    out  0A1h, al
    call io_delay

    mov  al, 001h
    out  021h, al
    call io_delay

    out  0A1h, al
    call io_delay

    mov  al, 11111001b ;允许键盘中断
    out  021h, al
    call io_delay

    mov  al, 11101111b ;允许鼠标中断
    out  0A1h, al
    call io_delay

    ret
