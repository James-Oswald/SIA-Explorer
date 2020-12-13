
#include<stdio.h>
#include<emscripten.h>
#include "assembler.h"
#include "vm.h"

EMSCRIPTEN_KEEPALIVE
int getMaxLineSize(){
    return MAX_LINE_SIZE;
}

EMSCRIPTEN_KEEPALIVE
int getMaxErrorSize(){
    return MAX_ERR_SIZE;
}

EMSCRIPTEN_KEEPALIVE
void assembleLineW(char* line, uint8_t* outputBytes, uint32_t* numberOutputBytes, uint32_t lineNumber, char* errorMessage, char* warningMessage){
    assembleLine(line, outputBytes, numberOutputBytes, lineNumber, errorMessage, warningMessage);
}

int main(){
    EM_ASM(start());
}