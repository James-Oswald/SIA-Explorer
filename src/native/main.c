
#include<stdio.h>
#include<emscripten.h>
#include "assembler.h"
#include "vm.h"

EMSCRIPTEN_KEEPALIVE
int main(int lol){
    printf("Hello World");
}