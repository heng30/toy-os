#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"

// 字母`A`例子
// 每个字符由16字节组成，每行为1个字节，每1个bit代表1个像素。
// `.`代表不显示颜色.  `*`代表显示颜色。 要将`.`转换为0, `*`转换为1
/*
char 0x41
........
...**...
...**...
...**...
...**...
..*..*..
..*..*..
..*..*..
..*..*..
.******.
.*....*.
.*....*.
.*....*.
***..***
........
........
*/

typedef struct {
    unsigned char m_data[16]; // 16 bytes per character
} font_char_t;

unsigned char line2byte(const char *line) {
    unsigned char b = 0;
    for (int i = 0; i < 8; i++) {
        if (line[i] == '*') {
            b |= (1 << (7 - i));
        }
    }

    return b;
}

void mk_font(const char *filename) {
    FILE *fp_in = fopen(filename, "r");
    assert(fp_in);

    FILE *fp_out = fopen("font_data.inc", "w");
    assert(fp_out);

    fprintf(fp_out, "system_font:\n");

    char line[100];
    while (fgets(line, sizeof(line), fp_in) != NULL) {
        if (strstr(line, "char") != NULL) {
            font_char_t font_char;
            for (int i = 0; i < 16; i++) {
                fgets(line, sizeof(line), fp_in);
                font_char.m_data[i] = line2byte(line);
            }

            fprintf(fp_out, "db ");
            for (int i = 0; i < 16; i++) {
                fprintf(fp_in, "0x%02X", font_char.m_data[i]);
                if (i < 15)
                    fprintf(fp_out, ", ");
            }
            fprintf(fp_out, "\n");
        }
    }

    fclose(fp_in);
    fclose(fp_out);
    debug("Font data written to font_data.inc");
}
