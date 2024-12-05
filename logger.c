#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "logger.h"

// foreground color
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_WHITE "\x1b[37m"
#define ANSI_COLOR_BLACK "\x1b[30m"
#define ANSI_COLOR_RESET "\x1b[0m"

// background color
#define ANSI_BG_RED "\x1b[41m"
#define ANSI_BG_GREEN "\x1b[42m"
#define ANSI_BG_YELLOW "\x1b[43m"
#define ANSI_BG_BLUE "\x1b[44m"
#define ANSI_BG_MAGENTA "\x1b[45m"
#define ANSI_BG_CYAN "\x1b[46m"
#define ANSI_BG_WHITE "\x1b[47m"

static struct {
    logger_level_t m_level;
} g_lg = {
    .m_level = LOGGER_LV_DEBUG,
};

const static char *_level_flag_get(logger_level_t level,
                                   const char **color_code,
                                   const char **bg_color_code) {
    switch (level) {
    case LOGGER_LV_TRACE:
        *color_code = ANSI_COLOR_BLACK;
        *bg_color_code = ANSI_BG_WHITE;
        return "TRACE";
    case LOGGER_LV_DEBUG:
        *color_code = ANSI_COLOR_WHITE;
        *bg_color_code = ANSI_BG_BLUE;
        return "DEBUG";
    case LOGGER_LV_INFO:
        *color_code = ANSI_COLOR_BLACK;
        *bg_color_code = ANSI_BG_GREEN;
        return "INFO";
    case LOGGER_LV_WARN:
        *color_code = ANSI_COLOR_BLACK;
        *bg_color_code = ANSI_BG_YELLOW;
        return "WARN";
    case LOGGER_LV_ERROR:
        *color_code = ANSI_COLOR_WHITE;
        *bg_color_code = ANSI_BG_RED;
        return "ERROR";
    case LOGGER_LV_FATAL:
        *color_code = ANSI_COLOR_WHITE;
        *bg_color_code = ANSI_BG_MAGENTA;
        return "FATAL";
    default:
        *color_code = ANSI_COLOR_RESET;
        *bg_color_code = ANSI_COLOR_RESET;
        return "UNKNOWN";
    }
}

void logger_set_level(logger_level_t level) { g_lg.m_level = level; }

void time_str(char *buf, unsigned int size) {
    struct tm ltm = {0};
    time_t nt = time((time_t *)NULL);
    localtime_r(&nt, &ltm);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    int ms = tv.tv_usec / 1000;

    char tmp[64] = {0};
    strftime(tmp, size, "%Y-%m-%dT%H:%M:%S", &ltm);

    snprintf(buf, size, "%s.%d", tmp, ms);
}

void logger_do(unsigned char level, const char *file, const char *func,
               int line, const char *format, ...) {
    if (level < g_lg.m_level)
        return;

    const char *color_code = NULL;
    const char *bg_color_code = NULL;
    const char *level_c = _level_flag_get(level, &color_code, &bg_color_code);

    char buf[64] = {0};
    time_str(buf, sizeof(buf));

    va_list vl;
    va_start(vl, format);
    fprintf(stdout, "%s %s%s%s%s [%s %s:%d]: ", buf, bg_color_code, color_code,
            level_c, ANSI_COLOR_RESET, file, func, line);
    vfprintf(stdout, format, vl);
    va_end(vl);
}

#ifdef __TEST__
void logger_test(void) {
    logger_set_level(LOGGER_LV_TRACE);

    char *str = "hello world!";
    trace("%s", str);
    debug("%s", str);
    info("%s", str);
    warn("%s", str);
    error("%s", str);
    fatal("%s", str);

    debug("logger_test() Ok");
}
#endif
