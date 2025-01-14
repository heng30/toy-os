#include "pdraw.h"

void boxfill8(unsigned char *vram, unsigned int xsize, unsigned char c,
              unsigned int x0, unsigned int y0, unsigned int x1,
              unsigned int y1) {
    for (unsigned int y = y0; y <= y1; y++)
        for (unsigned int x = x0; x <= x1; x++)
            vram[y * xsize + x] = c;
}
