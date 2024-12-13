#include "colo8.h"
#include "draw.h"
#include "fifo8.h"
#include "io.h"
#include "keyboard.h"
#include "keyboard_mouse.h"
#include "kutil.h"

#define KEY_UNPRESSED 0 // 按键没有被按下
#define KEY_PRESSED 1   // 按键正在按下
#define KEY_CHECKED 2   // 给capslock等有状态按键使用

char keydown_code2char_table[0x54] = {
    0, 0,    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=',
    0, 0,    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']',
    0, 0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,    '*',
    0, ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,
    0, '7',  '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0',  '.',
};

// 保持特殊按键的状态
// [capslock, shift, ctrl, alt]
static unsigned char g_modkey_status[4] = {0};

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
    char ch = code < sizeof(keydown_code2char_table)
                  ? keydown_code2char_table[code]
                  : 0;

    if (ch > 0 && (is_capslock_checked() || is_shift_key_pressed())) {
        ch = to_uppercast(ch);
    }

    return ch;
}

bool is_shift_key_down(unsigned char code) {
    return code == 0x2a || code == 0x36;
}

bool is_shift_key_up(unsigned char code) {
    return code == 0xaa || code == 0xb6;
}

bool is_ctrl_key_down(unsigned char code) { return code == 0x1d; }

bool is_ctrl_key_up(unsigned char code) { return code == 0x9d; }

bool is_alt_key_down(unsigned char code) { return code == 0x38; }

bool is_alt_key_up(unsigned char code) { return code == 0xb8; }

bool is_backspace_down(unsigned char code) { return code == 0x0e; }

bool is_enter_down(unsigned char code) { return code == 0x1c; }

bool is_enter_up(unsigned char code) { return code == 0x9c; }

bool is_capslock_down(unsigned char code) { return code == 0x3a; }

bool is_capslock_up(unsigned char code) { return code == 0xba; }

// modkey status
bool is_capslock_checked() { return g_modkey_status[0] == KEY_CHECKED; }

bool is_shift_key_pressed() { return g_modkey_status[1] == KEY_PRESSED; }

bool is_ctrl_key_pressed() { return g_modkey_status[2] == KEY_PRESSED; }

bool is_alt_key_pressed() { return g_modkey_status[3] == KEY_PRESSED; }

void set_modkey_status(unsigned char code) {
    if (is_capslock_up(code)) {
        g_modkey_status[0] =
            g_modkey_status[0] == KEY_UNPRESSED ? KEY_CHECKED : KEY_UNPRESSED;
    } else if (is_shift_key_down(code)) {
        g_modkey_status[1] = KEY_PRESSED;
    } else if (is_shift_key_up(code)) {
        g_modkey_status[1] = KEY_UNPRESSED;
    } else if (is_ctrl_key_down(code)) {
        g_modkey_status[2] = KEY_PRESSED;
    } else if (is_ctrl_key_up(code)) {
        g_modkey_status[2] = KEY_UNPRESSED;
    } else if (is_alt_key_down(code)) {
        g_modkey_status[3] = KEY_PRESSED;
    } else if (is_alt_key_up(code)) {
        g_modkey_status[3] = KEY_UNPRESSED;
    }
}
