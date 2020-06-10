
#include<stdio.h>
#include<emscripten.h>
#include "assembler.h"
#include "vm.h"

EMSCRIPTEN_KEEPALIVE
int ret(int a){
    printf("Hello World\n");
    return a;
}

int main(){
    EM_ASM(start());
}