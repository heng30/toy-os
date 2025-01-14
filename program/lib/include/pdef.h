#pragma once

#define NULL ((void *)0)
#define SPACE ' '

#define true 1
#define false 0
typedef unsigned char bool;
typedef unsigned int ptr_t;

// 内存大小单位
#define KB 1024
#define MB (KB * KB)
#define GB (MB * KB)

#define WINDOW_TITLE_BAR_HEIGHT 20 // 窗口标题栏高度
#define WINDOW_BORDER_SIZE 4       // 窗口边框大小
#define FONT_WIDTH 8               // 字体宽度
#define FONT_HEIGHT 16             // 字体高度
#define HEX_INT_FONT_WIDTH 80      // 一个数字16进制字符串宽度

#define DEFAULT_DELAY_LOOP 100

// 最大定时器连续触发此时，可以看作是不断触发的定时器
#define TIMER_MAX_RUN_COUNTS 0xffffffff

#define TIMER_ONE_SECOND_TIME_SLICE 100  // 1秒的时间片数量
