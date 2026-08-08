/**
* https://www.youtube.com/watch?v=vYODKK8TQGE Impariamo il C, lezione 23: scriviamo l'interprete Toy Forth
* https://www.youtube.com/watch?v=-QxrmHo-V7Y Impariamo il C, lezione 24: l'interprete Toy Forth (parte 2)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

/* === DATA STRUCTURES === */

#define TFOBJ_TYPE_INT 0
#define TFOBJ_TYPE_STR 1
#define TFOBJ_TYPE_BOOL 2
#define TFOBJ_TYPE_LIST 3
#define TFOBJ_TYPE_SYMBOL 4


typedef struct tfobj {
    int refcount;
    int type;
    union{
        int i;
        struct {
            char *ptr;
            size_t len;
        } str;
        struct {
            struct tfobj **ele;
            size_t len;
            size_t alloc_len; // use to realloc
        } list;
    };

} tfobj;


typedef struct parser {
    char *prg;
    char *p; // next token to parse
} tfparser;

typedef struct tfctx {
    tfobj *stack;
} tcfx;

/* === ALLOCATION WRAPPER === */
void *xmalloc(size_t size){
    void *ptr = malloc(size);
    if(ptr == NULL) {
        fprintf(stderr, "Out of memory allocating %zu bytes\n", size);
        exit(1);
    }
    return ptr;
}

/* === OBJECT FUNCTIONS === */

/* create and initialized a new toyforth object */

tfobj *createObject(int type){
    tfobj *o = xmalloc(sizeof(tfobj));
    o->type = type;
    o->refcount = 1;
    return o;
}



tfobj *createIntObject(int i){
    tfobj *o = createObject(TFOBJ_TYPE_INT);
    o->i = i;
    return o;
}

tfobj *createBoolObject(int i){
    tfobj *o = createObject(TFOBJ_TYPE_BOOL);
    o->i = i;
    return o;
}

tfobj *createStringObject(char *string, size_t len){
    tfobj *o = createObject(TFOBJ_TYPE_STR);
    o->str.ptr = string;
    o->str.len = len;
    return o;
}

tfobj *createSymbolObject(char *s, size_t len){
    tfobj *o = createStringObject(s, len);
    o->type = TFOBJ_TYPE_SYMBOL; 
    return o;
}

tfobj *createListObject(){
    tfobj *o = createObject(TFOBJ_TYPE_LIST);
    o->list.ele = NULL;
    o->list.len = 0;
    return o;
}

/* add new element at the end of the list 
 * the caller should increment the reference count */
void listPush(tfobj *l, tfobj *ele) {
    l->list.ele = realloc(l->list.ele, sizeof(tfobj*) * (l->list.len+1));
    l->list.ele[l->list.len] = ele;
    l->list.len++;
}



/* === TURN PROGRAM INTO TOY FORTH LIST === */

void parserSkipSpaces(tfparser *parser){
    while (isspace(parser->p[0])) {
        parser->p++;
    }
}

#define MAX_NUM_LEN 128
tfobj *parseNumber(tfparser *parser){
    char buf[MAX_NUM_LEN];
    char *start = parser->p;
    char *end;

    if(parser->p[0] == '-') parser->p++;

    while(parser->p[0] && isdigit(parser->p[0])){
         parser->p++;
    }

    end = parser->p;
    int numlen = end - start;

    if(numlen >= MAX_NUM_LEN) return NULL;

    memcpy(buf, start, numlen);
    buf[numlen] = 0;

    tfobj *o = createIntObject(atoi(buf));
    return o;
}

tfobj *compile(char *prg){
    tfparser parser;

    parser.prg = prg;
    parser.p = prg;

    tfobj *parsed = createListObject();

    while (parser.p[0] != '\0') {
        tfobj *o;
        char *toker_start = parser.p;

        parserSkipSpaces(&parser);

        if(parser.p[0] == '\0') break; // End of program reached.

        if (isdigit(parser.p[0]) || parser.p[0] == '-') {
            o = parseNumber(&parser);
        } else {
            o = NULL;
        }

        if(o == NULL) {
            printf("Syntax error near %32s\n", toker_start);
        } else {
            listPush(parsed, o);
        }
    }
    return parsed;
}

void exec(tfobj *prg){
    printf("[");
    for(size_t j=0; j<prg->list.len; j++){
        tfobj *o = prg->list.ele[j];
        switch(o->type){
        case TFOBJ_TYPE_INT:
            printf("%d", o->i);
            break;
        default: 
            printf("?");
            break;
        }
        printf(" ");
    }
        
    printf("]\n");
}

/* === MAIN === */

int main (int argc, char **argv){

    if ( argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0] );
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");

    if(fp == NULL){
        fprintf(stderr, "It is not possible to read the file.\n" );
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    char *prgtext = xmalloc( file_size + 1); // + 1 is null term \0
    fseek(fp, 0, SEEK_SET);
    fread(prgtext, file_size, 1, fp);
    prgtext[file_size] = 0;
    fclose(fp);

    printf("Program text: %s\n", prgtext);

    tfobj *prg = compile(prgtext);
    exec(prg);

    return 0;
}