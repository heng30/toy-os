api_console_draw_ch:
  mov edx, SYSTEM_CALL_CONSOLE_DRAW_CH
  mov al, [esp + 4]
  int SYSTEM_CALL_INT
  ret

api_console_draw_text:
  mov edx, SYSTEM_CALL_CONSOLE_DRAW_TEXT
  mov ebx, [esp + 4]
  mov eax, [esp + 8]
  int SYSTEM_CALL_INT
  ret

api_new_window:
  push edi
  push esi
  push ebx
  mov  edx, SYSTEM_CALL_NEW_WINDOW
  mov  ebx, [esp + 16] ; x
  mov  esi, [esp + 20] ; y
  mov  edi, [esp + 24] ; xsize
  mov  eax, [esp + 28] ; ysize
  mov  ecx, [esp + 32] ; title
  int  SYSTEM_CALL_INT
  pop  ebx
  pop  esi
  pop  edi
  ret

api_close_window:
  push ebx
  mov  edx, SYSTEM_CALL_CLOSE_WINDOW
  mov  ebx, [esp + 8] ; win
  int  SYSTEM_CALL_INT
  pop  ebx
  ret

api_refresh_window:
  push  edi
  push  esi
  push  ebx
  mov   edx, SYSTEM_CALL_REFRESH_WINDOW
  mov   ebx, [esp + 16] ; win
  mov   eax, [esp + 20] ; x0
  mov   ecx, [esp + 24] ; y0
  mov   esi, [esp + 28] ; x1
  mov   edi, [esp + 32] ; y1
  int   SYSTEM_CALL_INT
  pop   ebx
  pop   esi
  pop   edi
  ret

api_is_close_window:
  mov edx, SYSTEM_CALL_IS_CLOSE_WINDOW
  int SYSTEM_CALL_INT
  ret

api_draw_text_in_window:
  push edi
  push esi
  push ebp
  push ebx
  mov  edx, SYSTEM_CALL_DRAW_TEXT_IN_WINDOW
  mov  ebx, [esp + 20]  ; win
  mov  esi, [esp + 24]  ; x
  mov  edi, [esp + 28]  ; y
  mov  eax, [esp + 32]  ; col
  mov  ecx, [esp + 36]  ; text
  mov  ebp, [esp + 40]  ; is_literal
  int  SYSTEM_CALL_INT
  pop  ebx
  pop  ebp
  pop  esi
  pop  edi
  ret

api_draw_box_in_window:
  push edi
  push esi
  push ebp
  push ebx
  mov  edx, SYSTEM_CALL_DRAW_BOX_IN_WINDOW
  mov  ebx, [esp + 20] ; win
  mov  eax, [esp + 24] ; x0
  mov  ecx, [esp + 28] ; y0
  mov  esi, [esp + 32] ; x1
  mov  edi, [esp + 36] ; y1
  mov  ebp, [esp + 40] ; col
  int  SYSTEM_CALL_INT
  pop  ebx
  pop  ebp
  pop  esi
  pop  edi
  ret

api_draw_point_in_window:
  push edi
  push esi
  push ebx
  mov  edx, SYSTEM_CALL_DRAW_POINT_IN_WINDOW
  mov  ebx, [esp + 16]  ; win
  mov  esi, [esp + 20]  ; x
  mov  edi, [esp + 24]  ; y
  mov  eax, [esp + 28]  ; col
  int  SYSTEM_CALL_INT
  pop  ebx
  pop  esi
  pop  edi
  ret

api_draw_line_in_window:
  push edi
  push esi
  push ebp
  push ebx
  mov  edx, SYSTEM_CALL_DRAW_LINE_IN_WINDOW
  mov  ebx, [esp + 20] ; win
  mov  eax, [esp + 24] ; x0
  mov  ecx, [esp + 28] ; y0
  mov  esi, [esp + 32] ; x1
  mov  edi, [esp + 36] ; y1
  mov  ebp, [esp + 40] ; col
  int  SYSTEM_CALL_INT
  pop  ebx
  pop  ebp
  pop  esi
  pop  edi
  ret

api_rand_uint:
  push ebx
  mov  edx, SYSTEM_CALL_RAND_UINT
  mov  ebx, [esp + 8]
  int  SYSTEM_CALL_INT
  pop  ebx
  ret

api_show_debug_uint:
  push ebx
  mov  edx, SYSTEM_CALL_SHOW_DEBUG_UINT
  mov  ebx, [esp + 8]
  mov  eax, [esp + 12]
  mov  ecx, [esp + 16]
  int  SYSTEM_CALL_INT
  pop  ebx
  ret

api_timer_alloc:
  mov  edx, SYSTEM_CALL_TIMER_ALLOC
  int  SYSTEM_CALL_INT
  ret

api_timer_free:
  push ebx
  mov  edx, SYSTEM_CALL_TIMER_FREE
  mov  ebx, [esp + 8]
  int  SYSTEM_CALL_INT
  pop  ebx
  ret

api_timer_set:
  push ebx
  mov  edx, SYSTEM_CALL_TIMER_SET
  mov  ebx, [esp + 8]
  mov  eax, [esp + 12]
  mov  ecx, [esp + 16]
  int  SYSTEM_CALL_INT
  pop  ebx
  ret

api_timer_is_timeout:
  push ebx
  mov  edx, SYSTEM_CALL_TIMER_IS_TIMEOUT
  mov  ebx, [esp + 8]
  int  SYSTEM_CALL_INT
  pop  ebx
  ret

api_file_open:
  push ebx
  mov  edx, SYSTEM_CALL_FILE_OPEN
  mov  ebx, [esp + 8]
  mov  eax, [esp + 12]
  int  SYSTEM_CALL_INT
  pop  ebx
  ret

api_file_read:
  push edi
  push ebx
  mov  edx, SYSTEM_CALL_FILE_READ
  mov  ebx, [esp + 12]
  mov  eax, [esp + 16]
  mov  ecx, [esp + 20]
  mov  edi, [esp + 24]
  int  SYSTEM_CALL_INT
  pop  ebx
  pop  edi
  ret

api_file_write:
  push edi
  push ebx
  mov  edx, SYSTEM_CALL_FILE_WRITE
  mov  ebx, [esp + 12]
  mov  eax, [esp + 16]
  mov  ecx, [esp + 20]
  mov  edi, [esp + 24]
  int  SYSTEM_CALL_INT
  pop  ebx
  pop  edi
  ret

api_file_close:
  push ebx
  mov  edx, SYSTEM_CALL_FILE_CLOSE
  mov  ebx, [esp + 8]
  int  SYSTEM_CALL_INT
  pop  ebx
  ret

api_file_size:
  push ebx
  mov  edx, SYSTEM_CALL_FILE_SIZE
  mov  ebx, [esp + 8]
  int  SYSTEM_CALL_INT
  pop  ebx
  ret

api_strlen_cs:
  push ebx
  mov  edx, SYSTEM_CALL_STRLEN_CS
  mov  ebx, [esp + 8]
  int  SYSTEM_CALL_INT
  pop  ebx
  ret

api_memcpy_cs2ds:
  push ebx
  mov  edx, SYSTEM_CALL_MEMCPY_CS2DS
  mov  ebx, [esp + 8]
  mov  eax, [esp + 12]
  mov  ecx, [esp + 16]
  int  SYSTEM_CALL_INT
  pop  ebx
  ret
