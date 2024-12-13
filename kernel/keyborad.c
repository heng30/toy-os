#include "fifo8.h"
#include "io.h"
#include "keyboard.h"
#include "keyboard_mouse.h"

char keydown_table[0x54] = {
    0, 0,    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=',
    0, 0,    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[',  ']',
    0, 0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',
    0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0,    '*',
    0, ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,
    0, '7',  '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0',  '.',
};

static unsigned char g_keybuf[64];
fifo8_t g_keyinfo = {
    .m_buf = g_keybuf,
    .m_size = sizeof(g_keybuf),
    .m_free = sizeof(g_keybuf),
    .m_flags = 0,
    .m_p = 0,
    .m_q = 0,
};

void int_handler_from_c(char *esp) {
    io_out8(PIC_OCW2, 0x20);
    unsigned char data = io_in8(PORT_KEYDAT);
    fifo8_put(&g_keyinfo, data);
}

void init_keyboard(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}

char get_pressed_char(unsigned char code) {
    return code < sizeof(keydown_table) ? keydown_table[code] : 0;
}

bool is_ctrl_key_down(unsigned char code) { return code == 0x1d; }

bool is_ctrl_key_up(unsigned char code) { return code == 0x9d; }

bool is_alt_key_down(unsigned char code) { return code == 0x38; }

bool is_alt_key_up(unsigned char code) { return code == 0xb8; }

bool is_backspace_down(unsigned char code) { return code == 0x0e; }

bool is_enter_down(unsigned char code) { return code == 0x1c; }

bool is_enter_up(unsigned char code) { return code == 0x9c; }
