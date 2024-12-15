#pragma once

#define PIC_OCW2 0x20
#define PIC1_OCW2 0xA0

#define KBC_MODE 0x47
#define PORT_KEYSTA 0x64
#define PORT_KEYDAT 0x60
#define PORT_KEYCMD 0x64
#define KEYCMD_WRITE_MODE 0x60
#define KEYSTA_SEND_NOTREADY 0x02

// 等待键盘准备好
void wait_KBC_sendready(void);
