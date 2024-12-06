; jmp write_vga
; %include "kernel/build/write_vga.asm"

; jmp write_vga_palette
; %include "kernel/build/write_vga_palette.asm"
; %include "kernel/build/palette_table_rgb.asm"

; jmp write_vga_rectangle
; %include "kernel/build/write_vga_rectangle.asm"
; %include "kernel/build/palette_table_rgb.asm"

; jmp write_vga_desktop
; %include "kernel/build/write_vga_desktop.asm"
; %include "kernel/build/palette_table_rgb.asm"

; jmp write_vga_desktop_single_char
; %include "kernel/build/write_vga_desktop_single_char.asm"
; %include "kernel/build/palette_table_rgb.asm"

; jmp write_vga_desktop_system_font
; %include "kernel/build/write_vga_desktop_system_font.asm"
; %include "kernel/build/palette_table_rgb.asm"

; jmp write_vga_desktop_string
; %include "kernel/build/write_vga_desktop_string.asm"
; %include "kernel/build/palette_table_rgb.asm"

; jmp write_vga_cursor
; %include "kernel/build/write_vga_cursor.asm"
; %include "kernel/build/palette_table_rgb.asm"

; jmp write_vga_desktop_int_handler
; %include "kernel/build/write_vga_desktop_int_handler.asm"
; %include "kernel/build/palette_table_rgb.asm"

jmp keyboard_input
%include "kernel/build/keyboard_input.asm"
%include "kernel/build/palette_table_rgb.asm"
