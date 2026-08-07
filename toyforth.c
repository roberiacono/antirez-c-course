/**
* https://www.youtube.com/watch?v=vYODKK8TQGE
* Impariamo il C, lezione 23: scriviamo l'interprete Toy Forth
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

tfobj *createListObject(int i){
    tfobj *o = createObject(TFOBJ_TYPE_LIST);
    o->list.ele = NULL;
    o->list.len = 0;
    return o;
}

tfobj *createSymbolObject(char *s, size_t len){
    tfobj *o = createStringObject(s, len);
    o->type = TFOBJ_TYPE_SYMBOL; 
    return o;
}

/* === MAIN === */

int main (int argc, char **argv){

    if ( argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0] );
        return 1;
    }

    //tbobj *prg = compile(prgtext);
    //exec(prg);

    return 0;
}