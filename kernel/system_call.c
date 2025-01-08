#include "system_call.h"
#include "string.h"

#include "widgets/console.h"

static void _console_draw_ch(int eax) {
    console_t *p = console_get();
    console_draw_ch(p, (char)(eax & 0xff));
}

static void _console_draw_text(int ebx) {
    console_t *p = console_get();
    if (!p->m_cmd->m_data)
        return;

    const char *text = (const char *)(p->m_cmd->m_data + ebx);
    console_draw_text(p, text);
}

static void _console_draw_invalid_system_call(int edx) {
    console_t *p = console_get();
    console_draw_text(p, "invalid system call: ");
    console_draw_text(p, int2hexstr((unsigned int)edx));
    console_move_to_next_line(p);
}

ptr_t *system_call_api(int edi, int esi, int ebp, int esp, int ebx, int edx,
                       int ecx, int eax) {
    if (edx == SYSTEM_CALL_CONSOLE_DRAW_CH) {
        _console_draw_ch(eax);
    } else if (edx == SYSTEM_CALL_CONSOLE_DRAW_TEXT) {
        _console_draw_text(ebx);
    } else if (edx == SYSTEM_CALL_END_CMD) {
        return &g_multi_task_ctl->m_current_task->m_tss.m_esp0;
    } else {
        _console_draw_invalid_system_call(edx);
    }

    return NULL;
}
