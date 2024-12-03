#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "util.h"
#include "logger.h"

int max(int a, int b) { return a > b ? a : b; }

int min(int a, int b) { return a > b ? b : a; }

int rand_num(int l, int h, unsigned int seed) {
    assert(l <= h);
    srand(time(NULL) * seed);
    return l + (rand() % (h - l + 1));
}

int bound(int n, int l, int h) {
    assert(l <= h);
    return min(max(n, l), h);
}

long file_size(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return -1;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }

    long size = ftell(fp);
    fclose(fp);
    return size;
}

#ifdef __TEST__
#include <unistd.h>

void util_test(void) {
    assert(max(1, 2) == 2);
    assert(min(1, 2) == 1);

    assert(bound(2, 1, 3) == 2);
    assert(bound(-1, 1, 3) == 1);
    assert(bound(4, 1, 3) == 3);

    for (int i = 0; i < 100; i++) {
        int n = rand_num(1, 10, i);
        assert(n >= 1 && n <= 10);

        usleep(1 * 1000);
    }

    debug("util_test() Ok");
}
#endif
