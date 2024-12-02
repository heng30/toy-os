#pragma once

// return value scope: [l, h]
int rand_num(int l, int h, unsigned int seed);

int max(int a, int b);

int min(int a, int b);

// if n <= l, return l;
// if n >= h, return h;
// if l < n < h, return n;
int bound(int n, int l, int h);

#ifdef __TEST__
void util_test(void);
#endif
