[map symbols ovfl.map]
[SECTION .s32]
BITS 32

entry:
    ; 这条命令会引发一个异常. 访问的内存位置超出的数据段的范围
    mov eax, 1024*1024
    mov byte [eax], 1
    mov edx, 1
    int 0x2d
