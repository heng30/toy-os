#include "string.h"
#include "system_call.h"

#include "widgets/console.h"

static void _console_draw_ch(int eax) {
    for (unsigned int i = 0; i < g_window_ctl.m_top; i++) {
        window_t *w = g_window_ctl.m_windows[i];
        if (w->m_id == WINDOW_ID_CONSOLE) {
            console_draw_ch((console_t *)w->m_instance, (char)(eax & 0xff));
            return;
        }
    }
}

static void _console_draw_text(int ebx) {
    for (unsigned int i = 0; i < g_window_ctl.m_top; i++) {
        window_t *w = g_window_ctl.m_windows[i];
        if (w->m_id == WINDOW_ID_CONSOLE) {
            console_t *p = (console_t *)w->m_instance;
            if (!p->m_cmd->m_data)
                return;

            const char *text = (const char *)(p->m_cmd->m_data + ebx);
            console_draw_text(p, text);
            return;
        }
    }
}

static void _console_draw_invalid_system_call(int edx) {
    for (unsigned int i = 0; i < g_window_ctl.m_top; i++) {
        window_t *w = g_window_ctl.m_windows[i];
        if (w->m_id == WINDOW_ID_CONSOLE) {
            console_t *p = (console_t *)w->m_instance;
            console_draw_text(p, "invalid system call: ");
            console_draw_text(p, int2hexstr((unsigned int)edx));
            console_move_to_next_line(p);
            return;
        }
    }
}

void system_call_api(int edi, int esi, int ebp, int esp, int ebx, int edx,
                     int ecx, int eax) {
    if (edx == SYSTEM_CALL_CONSOLE_DRAW_CH) {
        _console_draw_ch(eax);
    } else if (edx == SYSTEM_CALL_CONSOLE_DRAW_TEXT) {
        _console_draw_text(ebx);
    } else {
        _console_draw_invalid_system_call(edx);
    }
}
