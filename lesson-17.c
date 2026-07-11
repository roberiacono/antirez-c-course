// developing hexdump manually

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct pls {
    long len;
    char str[20];
};

#define HEXDUMP_CHARS_PER_LINE 16
void hexdump(void *p, size_t len){
    unsigned char *byte = p;
    size_t printed_offset = 0;
    for(size_t j = 0; j < len; j++){
        printf("%02x ", byte[j]);
        if((j+1) % HEXDUMP_CHARS_PER_LINE == 0 || j == len-1){
            if(j == len - 1){
                int padding = HEXDUMP_CHARS_PER_LINE - (len % HEXDUMP_CHARS_PER_LINE);
                padding %= HEXDUMP_CHARS_PER_LINE;
                for(int i = 0; i < padding; i++) printf("~~ ");
            }
            printf("\t");
            for(size_t k = printed_offset; k < j + 1; k++){
                char c = isprint(byte[k]) ? byte[k] : '.';
                printf("%c", c);
            }
            printed_offset = j + 1;
            printf("\n");
        }
    }
    printf("\n");
}

int main(void) {
    
    struct pls s;

    // set to FF non initialized memory
   // memset(&s, 0xFF, sizeof(s));

    s.len = 10;
    memcpy (s.str, "1234567890", 11); // 10+1 for null terminator
    //s.str[s.len] = '\0'; // Null-terminate the string

    hexdump(&s, sizeof(s));

    return 0;
}