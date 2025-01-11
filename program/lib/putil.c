#include "puitl.h"
#include "pdef.h"

int max(int a, int b) { return a > b ? a : b; }
int min(int a, int b) { return a > b ? b : a; }

void delay(unsigned int loop) {
    for(unsigned int i = 0; i != loop; i++);
}
