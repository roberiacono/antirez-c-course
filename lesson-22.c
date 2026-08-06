/* 
* https://www.youtube.com/watch?v=OIseV5lcx8w&list=WL
* Impariamo il C, lezione 22: i puntatori a funzione
*/

#include <stdio.h>

void hello(void){
    printf("hello\n");
}

void baubau(void){
    printf("baubau\n");
}

void call_n_times(int n, void (*x)(void)){
    while(n--){
        x();
    }
}

int main(void){

    void (*x)(void);

    call_n_times(10, hello);
    call_n_times(10, baubau);

    return 0;
}