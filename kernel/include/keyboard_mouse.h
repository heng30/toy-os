#pragma once

#define PORT_KEYDAT 0x0060
#define PIC_OCW2 0x20
#define PIC1_OCW2 0xA0

#define PORT_KEYSTA 0x0064
#define KEYSTA_SEND_NOTREADY 0x02

#define PORT_KEYDAT 0x0060
#define PORT_KEYCMD 0x0064
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

// 等待键盘准备好
void wait_KBC_sendready();
