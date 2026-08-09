/**
* https://www.youtube.com/watch?v=vYODKK8TQGE Impariamo il C, lezione 23: scriviamo l'interprete Toy Forth
* https://www.youtube.com/watch?v=-QxrmHo-V7Y Impariamo il C, lezione 24: l'interprete Toy Forth (parte 2)
* https://www.youtube.com/watch?v=-1ZhCgaIPOk Impariamo il C, lezione 25: l'interprete Toy Forth (parte 3)
* https://www.youtube.com/watch?v=oMj3N6jYIUU Impariamo il C, lezione 26: Toy Forth, nei meandri della exec() (parte 4)
* https://www.youtube.com/watch?v=C4AHEK3fSjg Impariamo il C, lezione 27: Toy Forth, la registrazione delle funzioni (parte 5)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

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



/* === ALLOCATION WRAPPER === */
void *xmalloc(size_t size){
    void *ptr = malloc(size);
    if(ptr == NULL) {
        fprintf(stderr, "Out of memory allocating %zu bytes\n", size);
        exit(1);
    }
    return ptr;
}

void *xrealloc(void *oldptr, size_t size){
    void *ptr = realloc(oldptr, size);
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




void release(tfobj *o);

/* free the memory */
void freeObject(tfobj *o){
    switch(o->type){
        case TFOBJ_TYPE_LIST:   
            for(size_t j=0; j<o->list.len; j++){
                tfobj *ele = o->list.ele[j];
                release(ele);
            }                
            break;
        case TFOBJ_TYPE_SYMBOL: 
        case TFOBJ_TYPE_STR: 
            free(o->str.ptr);
            break;
        }
        free(o);
}


void retain(tfobj *o){
    o->refcount++;
}

void release(tfobj *o){
    assert(o->refcount > 0);
    o->refcount--;
    if(o->refcount == 0) freeObject(o);
}

/* === STRING OBJECT === */

tfobj *createStringObject(char *string, size_t len){
    tfobj *o = createObject(TFOBJ_TYPE_STR);
    o->str.ptr = xmalloc(len+1);
    o->str.len = len;
    memcpy( o->str.ptr, string, len);
    o->str.ptr[len] = '\0';
    return o;
}


tfobj *createSymbolObject(char *s, size_t len){
    tfobj *o = createStringObject(s, len);
    o->type = TFOBJ_TYPE_SYMBOL; 
    return o;
}

int compareStringObject(tfobj *a, tfobj *b){
    size_t minlen = a->str.len < b->str.len ? a->str.len : b->str.len;
    int cmp = memcmp(a->str.ptr, b->str.ptr, minlen);

    if(cmp == 0){
        if(a->str.len == b->str.len) return 0;
        if(a->str.len > b->str.len) return 1;
        else return -1;
    } else {
       if(cmp < 0) return -1;
       else return 1;
    }

}

/* === LIST OBJECT === */

tfobj *createListObject(){
    tfobj *o = createObject(TFOBJ_TYPE_LIST);
    o->list.ele = NULL;
    o->list.len = 0;
    return o;
}

/* add new element at the end of the list 
 * the caller should increment the reference count */
void listPush(tfobj *l, tfobj *ele) {
    l->list.ele = xrealloc(l->list.ele, sizeof(tfobj*) * (l->list.len+1));
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

int is_symbol_char(char c){
    char symchars[] = "+-*/%";
    return isalpha(c) || strchr(symchars, c) != NULL;
}

tfobj *parseSymbol(tfparser *parser){
    char *start = parser->p;

    while(parser->p[0] && is_symbol_char(parser->p[0])){
        parser->p++;
    }

    int len = parser->p - start;

    return createSymbolObject(start, len);
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

        if (isdigit(parser.p[0]) || (parser.p[0] == '-' && isdigit(parser.p[1]))) {
            o = parseNumber(&parser);
        } else if(is_symbol_char(parser.p[0])) {
            o = parseSymbol(&parser);
        } else {
            o = NULL;
        }

        if(o == NULL) {
            release(parsed);
            printf("Syntax error near %32s\n", toker_start);
            return NULL;
        } else {
            listPush(parsed, o);
        }
    }
    return parsed;
}

/* === PRINT OBJECT === */

void printObject(tfobj *o){
    switch(o->type){
        case TFOBJ_TYPE_INT: 
            printf("%d", o->i);
            break;
        case TFOBJ_TYPE_LIST:   
            printf("[");
            for(size_t j=0; j<o->list.len; j++){
                tfobj *ele = o->list.ele[j];
                printObject(ele);
                if(j < o->list.len - 1) printf(" ");
            }
                
            printf("]");
            break;
        case TFOBJ_TYPE_SYMBOL: 
            printf("%s", o->str.ptr);
            break;
        case TFOBJ_TYPE_STR: 
            printf("\"%s\"", o->str.ptr);
            break;
        default:
            printf("?");
            break;
    }
}

/* === CONTEXT === */

typedef struct tfctx tfctx;



/* Function table */
struct tfctx;

typedef struct FunctionTableEntry {
    tfobj *name;
    void (*callback) (struct tfctx *ctx, tfobj *name);
    tfobj *user_func;
} tffuncentry;

struct FunctionTable {
    tffuncentry **func_table;
    size_t func_count;
};

typedef struct tfctx {
    tfobj *stack;
    struct FunctionTable functable;
} tfctx;


tffuncentry *registerFunction(tfctx *ctx, tfobj *name){
    ctx->functable.func_table = xrealloc(ctx->functable.func_table, sizeof(tffuncentry*) * (ctx->functable.func_count + 1));
    tffuncentry *fe = xmalloc(sizeof(tffuncentry));
    ctx->functable.func_table[ctx->functable.func_count] = fe;
    ctx->functable.func_count++;
    fe->name = name;
    retain(name);
    fe->callback = NULL;
    fe->user_func = NULL;
    return fe;
}


/* === BASIC FUNCTIONS === */

void basicMathFunction(tfctx *ctx, tfobj *name){
    if(ctxCheckStackMinLen(ctx, 2)) return;
    tfobj *b = ctxStackPop(ctx, TFOBJ_TYPE_INT);
    tfobj *a = ctxStackPop(ctx, 0, TFOBJ_TYPE_INT);    
    if(a == NULL || b == NULL) return;

    int result;

    switch(ctx->str.ptr[0]){
        case '+': result = a->i + b->i; break;
        case '-': result = a->i - b->i; break;
        case '*': result = a->i * b->i; break;
        case '/': result = a->i / b->i; break;
        case '%': result = a->i % b->i; break;
    }

    ctxStackPush(ctx, createIntObject(result));
}


tffuncentry *getFunctionByName(tfctx *ctx, tfobj *name);

int registerCFunction(tfctx *ctx, char *name, void (*callback) (tfctx *ctx, tfobj *name)){
    tfobj *oname = createStringObject(name, strlen(name));
    tffuncentry *fe = getFunctionByName(ctx, oname);
    if(fe){
        if(fe->user_func){
            release(fe->user_func);
            fe->user_func = NULL;
        }
        fe->callback = callback;
    }else {
        fe = registerFunction(ctx, oname);
        fe->callback = callback;
    }

    release(oname);
    
}

tffuncentry *getFunctionByName(tfctx *ctx, tfobj *name) {
    for(size_t j=0; j< ctx->functable.func_count; j++){
        tffuncentry *fe = ctx->functable.func_table[j];

        if(compareStringObject(fe->name, name) == 0) 
            return fe;
    }
    return NULL;
}

tfctx *createContext(void){
    tfctx *ctx = xmalloc(sizeof(*ctx));
    ctx->stack = createListObject();
    ctx->functable.func_table = NULL;
    ctx->functable.func_count = 0;
    registerCFunction(ctx, "*", basicMathFunction);
    
    return ctx;
}



/* solve the function associated with the symbol named 'word'
 * return 0 if it matches some functions, otherwise returns 1 */
int  callSymbol(tfctx *ctx, tfobj *word){
    tffuncentry *fe = getFunctionByName(ctx, word);

    if(fe == NULL) return 1;
    return 0;
}

/* execute the toy forth program stored into the list 'prg' */
void exec(tfctx *ctx, tfobj *prg) {
    assert(prg->type == TFOBJ_TYPE_LIST);

    for(size_t j=0; j<prg->list.len; j++){
        tfobj *word = prg->list.ele[j];
        switch(word->type){
            case TFOBJ_TYPE_SYMBOL:
                callSymbol(ctx, word);
                break;        
            default:
                listPush(ctx->stack, word);
                retain(word);
                break;
        }
    }
}

/* === MAIN === */

int main(int argc, char **argv){

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
    printObject(prg);

    printf("\n");
    tfctx *ctx = createContext();
    exec(ctx, prg);
    
    printf("Stack content:");
    printObject(ctx->stack);
    printf("\n");

    return 0;
}