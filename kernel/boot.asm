; 开机后，biso检查完硬件后，会将软盘的第一个扇区的内容复制到0x7c00处，并跳转到这里继续执行代码。

; 关键变量：
;   boot代码起始地址：0x7c00
;   临时缓冲区起始地址：0x7e00
;   内核代码起始地址：0x8000
;   栈顶指针：sp = 0x7c00, 栈指针从高地址向低地址生长，所以不会覆盖掉boot代码

; 执行的过程：
;   1. 首先将软盘中内核代码所在的512字节复制到0x7e00处。
;   2. 接着将这512各字节复制到0x8000处，直到将所有的内核代码复制完成
;   3. 最后调整到0x8000处，执行内核代码

org 0x7c00

LoadAddr EQU  08000h ; 将内核写入到系统的内存位置
BufferAddr EQU 7E0h  ; 保存一个扇区数据的地址。 该值会被赋值给段寄存器，真实的地址为 7e00h = 7e0h << 4

BaseOfStack     equ 07c00h

jmp entry

; 系统相关信息
db  "BOOT-LOADER: TOY-LOADER"
db  "SYSTEM: TOY-OS"
db  "FILESYSTEM: TOY-FAT32"
db  "SECTOR = 512B"
db  "CYLINDERS = 18 * SECTOR"
db  "HALF-DISK = 80 * CYLINDERS"
db  "FLOPPY = 2 * 80 * 18 * 512B"
times 10 db 0

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

; 连续读取15个柱面: kernel.img(10 cylinders) + fs.img(5 cylinders)
load_count db 15
load_section dw 0800h

fin:
    hlt
    jmp  fin
