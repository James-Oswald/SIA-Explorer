
#include<stdio.h>
#include<emscripten.h>
#include "assembler.h"
#include "vm.h"

EMSCRIPTEN_KEEPALIVE
int getMaxLineSize(){
    return MAX_LINE_SIZE;
}

/*
void setPixel(){
	EM_ASM(setPixel(););
}

void display(){
	EM_ASM(display(););
}*/

EMSCRIPTEN_KEEPALIVE
int getMaxErrorSize(){
    return MAX_ERR_SIZE;
}

EMSCRIPTEN_KEEPALIVE
void assembleLineW(char* line, uint8_t* outputBytes, uint32_t* numberOutputBytes, uint32_t lineNumber, char* errorMessage, char* warningMessage){
    assembleLine(line, outputBytes, numberOutputBytes, lineNumber, errorMessage, warningMessage);
}

EMSCRIPTEN_KEEPALIVE
int getMemorySize(){
    return memorySize;
}

EMSCRIPTEN_KEEPALIVE
void loadProgram(uint8_t* pgrm, int pgrmLength){
	programCounter = 0;
	haltFlag = false;
	memset(registers, 0, 16 * sizeof(u32));
	registers[15] = memorySize;
	memset(memory, 0, memorySize);
	memcpy(memory, pgrm, pgrmLength);
}

EMSCRIPTEN_KEEPALIVE
uint8_t* getMem(){
	return memory;
}

EMSCRIPTEN_KEEPALIVE
uint32_t* getReg(){
	return registers;
}

EMSCRIPTEN_KEEPALIVE
uint32_t getPC(){
	return programCounter;
}

EMSCRIPTEN_KEEPALIVE
uint32_t getHalt(){
	return (uint32_t)haltFlag;
}

EMSCRIPTEN_KEEPALIVE
void vmStep(){
	if(!haltFlag){
		fetch();
		decode();
		execute();
		store();
	}
}

int main(){
    EM_ASM(start(););
}