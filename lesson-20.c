/* https://www.youtube.com/watch?v=yKavhObop5I */ 
/* Usage of mmap */


#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(void) {
    int fd = open("lesson-15.c", O_RDONLY);
    printf("Open file descriptor: %d\n", fd);

    void *mem = mmap(NULL, 100, PROT_READ, MAP_FILE|MAP_SHARED, fd, 0);
    printf("File mapped at %p\n", mem);

    /* now we use pointer to mem to get character */
    char *s = mem;
    for (int i = 0; i < 10; i++ ){
        printf("s[%d] is %c\n", i, s[i]);
    }

    return 0;
}