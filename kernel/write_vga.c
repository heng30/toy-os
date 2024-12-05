void io_hlt(void);

// 向显存中写入数据
void write_vga(void) {
    for (int i = 0xa0000; i < 0xaffff; i++)
        *(unsigned char *)i = i & 0x0f;

    for (;;)
        io_hlt();
}
