%include "include/ppm.inc"

[SECTION .s32]
BITS 32

entry:
    mov edx, SYSTEM_CALL_INVALID
    int SYSTEM_CALL_INT
    retf
