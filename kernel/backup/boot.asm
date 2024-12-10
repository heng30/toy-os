org 0x7c00

load_addr equ 0x8000 ; 将内核写入到系统的内存位置

jmp  entry

db   0x90
db   "OSKERNEL"
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
db   "MYFIRSTOS  "
db   "FAT12   "
resb  18

; 最多只能连续加载3个柱面(3*18=54个扇区).
; 因为ex固定为0, 只能通过修bx来控制每个柱面加载到内存的位置.
; 而且起始位置为0x9000. 所以3=((1 << 16) - 0x9000) / (18 * 512)
entry:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov si, ax

    mov bx, load_addr ; ex:bx 存储缓冲区. address = ex << 4 + bx
    mov ch, 1 ; 柱面号, 编号从0开始

read_floppy:
    cmp          byte [load_count], 0
    je           begin_load

    mov dh, 0 ; 磁头号, 0/1
    mov cl, 1 ; 扇区号, 编号从1开始

    mov ah, 0x02 ; 读盘操作
    mov al, 18  ; 连续读取几个扇区, 一个柱面一共18个扇区
    mov dl, 0 ; 驱动器编号, 只有一个磁盘，所以这里写死为0

    int 0x13 ; 调用bios中断实现磁盘读取功能

    jc           fin ; 如果读取出错，就跳转到错误

    ; 更新数据读取下一个柱面
    inc          ch
    dec          byte [load_count]
    add          bx, 0x2400 ; 偏移到下1个柱面所在的内存位置
    jmp          read_floppy

; 开始执行内核代码
begin_load:
    jmp          load_addr

load_count db 3 ;连续读取几个柱面

fin:
    hlt
    jmp fin
