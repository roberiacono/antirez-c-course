#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct line {
    char *s;
    struct line *next;   
};

int main(int argc, char **argv){

    if(argc != 2) {
        printf("missed file name\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if(fp == NULL){
        printf("file does not exist\n");
        return 1;   
    }

    char buf[1024];
    struct line *head = NULL;
    while(fgets(buf, sizeof(buf), fp) != NULL){
        struct line *l = malloc(sizeof(struct line));
        size_t linelen = strlen(buf);
        l->s = malloc(linelen+1);
        for(int i = 0; i < linelen+1; i++){
            l->s[i] = buf[i];
        }
        l->next = head;
        head = l;
    }

    fclose(fp);

    while(head != NULL){
        printf("%s", head->s);
        head = head->next;
    }
    return 0;
}
