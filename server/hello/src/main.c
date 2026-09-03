#include <stdio.h>
#include "sum.c"

int main(void) {
    printf("Hello, world!!!\n");

    int n = sum(5, 3);

    printf(n);

    return 0;
}