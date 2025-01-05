api_console_draw_ch:
    mov edx, SYSTEM_CALL_CONSOLE_DRAW_CH
    mov al, [esp + 4]
    int SYSTEM_CALL_INT
    ret

api_console_draw_text:
    mov edx, SYSTEM_CALL_CONSOLE_DRAW_TEXT
    mov ebx, [esp + 4]
    int SYSTEM_CALL_INT
    ret
