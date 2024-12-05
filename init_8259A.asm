init8259A:
     mov  al, 011h
     out  02h, al
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

     mov  al, 003h
     out  021h, al
     call io_delay

     out  0A1h, al
     call io_delay

     mov  al, 11111101b ;允许键盘中断
     out  21h, al
     call io_delay

     mov  al, 11111111b
     out  0A1h, al
     call io_delay

     ret

io_delay:
     nop
     nop
     nop
     nop
     ret
