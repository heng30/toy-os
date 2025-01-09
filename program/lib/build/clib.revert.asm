; Disassembly of file: /home/blue/Code/toy-os/program/lib/build/clib.o
; Thu Jan  9 16:05:49 2025
; Type: ELF32
; Syntax: NASM
; Instruction set: Pentium Pro


global max: function
global min: function


SECTION .text   align=1 execute                         ; section number 1, code

max:    ; Function begin
        push    ebp                                     ; 0000 _ 55
        mov     ebp, esp                                ; 0001 _ 89. E5
        mov     edx, dword [ebp+0CH]                    ; 0003 _ 8B. 55, 0C
        mov     eax, dword [ebp+8H]                     ; 0006 _ 8B. 45, 08
        cmp     edx, eax                                ; 0009 _ 39. C2
        cmovge  eax, edx                                ; 000B _ 0F 4D. C2
        pop     ebp                                     ; 000E _ 5D
        ret                                             ; 000F _ C3
; max End of function

min:    ; Function begin
        push    ebp                                     ; 0010 _ 55
        mov     ebp, esp                                ; 0011 _ 89. E5
        mov     edx, dword [ebp+8H]                     ; 0013 _ 8B. 55, 08
        mov     eax, dword [ebp+0CH]                    ; 0016 _ 8B. 45, 0C
        cmp     edx, eax                                ; 0019 _ 39. C2
        cmovle  eax, edx                                ; 001B _ 0F 4E. C2
        pop     ebp                                     ; 001E _ 5D
        ret                                             ; 001F _ C3
; min End of function


SECTION .data   align=1 noexecute                       ; section number 2, data


SECTION .bss    align=1 noexecute                       ; section number 3, bss


