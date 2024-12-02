#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "logger.h"

static struct {
    logger_level_t m_level;
} g_lg = {
    .m_level = LOGGER_LV_DEBUG,
};

const static char *_level_flag_get(logger_level_t level) {
    switch (level) {
    case LOGGER_LV_TRACE:
        return "TRACE";
    case LOGGER_LV_DEBUG:
        return "DEBUG";
    case LOGGER_LV_INFO:
        return "INFO";
    case LOGGER_LV_WARN:
        return "WARN";
    case LOGGER_LV_ERROR:
        return "ERROR";
    case LOGGER_LV_FATAL:
        return "FATAL";
    }
    return "UNKNOWN";
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

    const char *level_c = _level_flag_get(level);

    char buf[64] = {0};
    time_str(buf, sizeof(buf));

    va_list vl;
    va_start(vl, format);
    fprintf(stdout, "%s %s [%s %s:%d]: ", buf, level_c, file, func, line);
    vfprintf(stdout, format, vl);
    va_end(vl);
}

#ifdef __TEST__
void logger_test(void) {
    logger_set_level(LOGGER_LV_DEBUG);

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
