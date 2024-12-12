; 如何修该分辨率？
; 1. 显存起始地址: 0xa0000; 分辨率为320x200.
; mov   al, 0x13
; mov   ah, 0
; int   0x10

; 2. 显存起始地址: 0xe0000000
;   bx=0x4101, 分辨率为640x400
;   bx=0x4103, 分辨率为800x640
;   bx=0x4105, 分辨率为1024x768
;   bx=0x4107, 分辨率为1280x1024

mov   bx, 0x4101
mov   ax, 0x4f02
int   0x10

mov    dword [BOOT_INFO], 0xe0000000
mov    dword [BOOT_INFO + 4], 640
mov    dword [BOOT_INFO + 8], 480

