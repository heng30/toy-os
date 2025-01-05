// 提供给C语言调用的系统调用函数

#pragma once

// 在终端打印一个字符
void api_console_draw_ch(char ch);

// 在终端打印字符串
void api_console_draw_text(const char *text);
