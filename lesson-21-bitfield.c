/*
*   Impariamo il C, lezione 21: union e bitfield
*   https://www.youtube.com/watch?v=TM4jgODgdFY
*/ 


#include <stdio.h>

struct foo {
    unsigned char a:4;
    unsigned char b:4;
    unsigned char c:8;
};

int main(void) {
    struct foo f;

    f.a = 16;
    f.b = 2;
    f.c = 3;

    printf("%d %d %d\n", f.a, f.b, f.c);

    return 0;
}