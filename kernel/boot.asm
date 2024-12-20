org 0x7c00

LoadAddr EQU  08000h ; 将内核写入到系统的内存位置
BufferAddr EQU 7E0h  ; 保存一个扇区数据的地址。 该值会被赋值给段寄存器，真实的地址为 7e00h = 7e0h << 4

BaseOfStack     equ 07c00h

jmp  entry

; 内核相关信息
db   0x90
db   "BOOT LOADER"
dw   512
db   1
dw   1
db   2
dw   224
dw   2880
db   0xf0
dw   9
dw   18
dw   2
dd   0
dd   2880
db   0,0,0x29
dd   0xFFFFFFFF
db   "TOY OS"
db   "FAT12"
times 18  db 0 ; 保留18个字节的空间

entry:
    mov  ax, 0
    mov  ss, ax
    mov  ds, ax

    mov  ax, BufferAddr
    mov  es, ax

    mov  ax, 0
    mov  ss, ax
    mov  sp, BaseOfStack    ; 设置栈指针
    mov  di, ax
    mov  si, ax

    mov          BX, 0        ; ES:BX 数据存储缓冲区
    mov          CH, 1        ; CH 用来存储柱面号
    mov          DH, 0        ; DH 用来存储磁头号
    mov          CL, 0        ; CL 用来存储扇区号

;每次都把一个扇区内容写入地址 07E00处
readFloppy:
    cmp          byte [load_count], 0 ; 判断是否读取完指定数量的柱面
    je           beginLoad

    mov          bx, 0
    inc          CL
    mov          AH, 0x02      ; AH = 02 表示要做的是读盘操作
    mov          AL, 1        ; AL 表示要练习读取几个扇区
    mov          DL, 0         ; 驱动器编号，一般我们只有一个软盘驱动器，所以写死为0
    INT          0x13          ; 调用BIOS中断实现磁盘读取功能
    JC           fin

;把刚写入07E00h的一个扇区的内容写入到08000h开始的地址
copySector:
    push si
    push di
    push cx

    mov  cx, 0200h ; 512字节
    mov  di, 0
    mov  si, 0
    mov  ax, word [load_section] ; ax = 0800h
    mov  ds, ax     ; ds 段寄存器，[ds] = 0800h << 4 == 0800h

; 07E00h -> 08000h. 复制一个扇区内容
copy:
    cmp  cx, 0          ; 判断是否读取完512个字节
    je   copyend

    mov  al, byte [es:si]
    mov  byte [ds:di], al

    inc  di
    inc  si
    dec  cx
    jmp  copy

copyend:
    pop cx
    pop di
    pop si

    mov bx, ds
    add bx, 020h    ; 512 = 0x20 << 4
    mov ax, 0
    mov ds, ax
    mov word [load_section], bx ; 更新下一个扇区写入的位置
    mov bx, 0

    cmp          CL, 18     ; 判断是否复制完18个扇区，即一个柱面
    jb           readFloppy ; 重新读取一个扇区

    inc          CH         ; 柱面号加1
    mov          CL, 0      ; 扇区号归零
    dec          byte [load_count]  ; 更新需要读取的柱面数
    jmp          readFloppy ; 重新读取一个柱面

beginLoad:
    mov  ax, 0
    mov  ds, ax
    jmp          LoadAddr   ; 调整到内核中执行代码

load_count db 10            ; 连续读取几个柱面
load_section dw 0800h

fin:
    hlt
    jmp  fin
