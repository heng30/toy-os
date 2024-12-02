#pragma once

typedef enum LOGGER_LEVEL {
    LOGGER_LV_TRACE,
    LOGGER_LV_DEBUG,
    LOGGER_LV_INFO,
    LOGGER_LV_WARN,
    LOGGER_LV_ERROR,
    LOGGER_LV_FATAL,
} logger_level_t;

//  设置日志级别
void logger_set_level(logger_level_t level);

#ifdef __TEST__
void logger_test(void);
#endif

//	写日志文件
void logger_do(unsigned char level, const char *file, const char *func,
               int line, const char *format, ...);

#define trace(format, ...)                                                     \
    logger_do(LOGGER_LV_TRACE, __FILE__, __FUNCTION__, __LINE__, format "\n",  \
              ##__VA_ARGS__)

#define debug(format, ...)                                                     \
    logger_do(LOGGER_LV_DEBUG, __FILE__, __FUNCTION__, __LINE__, format "\n",  \
              ##__VA_ARGS__)

#define info(format, ...)                                                      \
    logger_do(LOGGER_LV_INFO, __FILE__, __FUNCTION__, __LINE__, format "\n",   \
              ##__VA_ARGS__)

#define warn(format, ...)                                                      \
    logger_do(LOGGER_LV_WARN, __FILE__, __FUNCTION__, __LINE__, format "\n",   \
              ##__VA_ARGS__)

#define error(format, ...)                                                     \
    logger_do(LOGGER_LV_ERROR, __FILE__, __FUNCTION__, __LINE__, format "\n",  \
              ##__VA_ARGS__)

#define fatal(format, ...)                                                     \
    logger_do(LOGGER_LV_FATAL, __FILE__, __FUNCTION__, __LINE__, format "\n",  \
              ##__VA_ARGS__)
