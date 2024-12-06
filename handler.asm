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
