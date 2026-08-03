/*
*   Impariamo il C, lezione 21: union e bitfield
*   https://www.youtube.com/watch?v=TM4jgODgdFY
*/ 


#include <stdio.h>

struct foo {
    union {
        int i;
        unsigned char a[4];
    };
};

int main(void) {
    struct foo f;

    f.i = 10;

    printf("%d [%d %d %d %d]\n", f.i, f.a[0], f.a[1], f.a[2], f.a[3]);

    return 0;
}