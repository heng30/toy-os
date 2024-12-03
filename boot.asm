org 0x7c00

jmp entry
db 0x90
db "OSKERNEL"
db 512
db 1
dw 1
db 2
dw 224
dw 2880
db 0xf0
dw 9
dw 18
dw 2
dd 0
dd 2880
db 0,0,0x29
dd 0xffffffff
db "MYFIRSTOS"
db "FAT12"
resb 18

entry:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov si, msg

readFloppy:
    mov ch, 1 ; 柱面号, 编号从0开始
    mov dh, 0 ; 磁头号, 0/1
    mov cl, 2 ; 扇区号, 编号从1开始

    mov bx, msg ; 存储缓冲区

    mov ah, 0x02 ; 读盘操作
    mov al, 1  ; 连续读取几个扇区
    mov dl, 0 ; 驱动器编号, 只有一个磁盘，所以这里写死为0

    int 0x13 ; 调用bios中断实现磁盘读取功能
    jc error

putloop:
    mov al, [si]
    add si, 1
    cmp al, 0
    je fin
    mov ah, 0x0e
    mov bx, 15
    int 0x10
    jmp putloop

fin:
    hlt
    jmp fin

error:
    mov si, errmsg
    jmp putloop

msg:
    resb 64

errmsg:
    db "error"
