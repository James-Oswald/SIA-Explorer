

#ifndef VM_H
#define VM_H

/*
	James Oswald
	ICSI 404
	Virtual Machine Assignment
	NO LONGER VALID
	Compiled using gcc 9.3.0 via msys64
*/

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>

typedef uint8_t u8;
typedef uint32_t u32;

#define memorySize 1000

u32 registers[16] = {0};
u32 programCounter = 0;
u32 currentInstruction = 0; 
u32 opcode = 0;		//used to preserve the opcode contained in currentInstruction to avoid recalculating it each instruction eval step
u32 oprand1 = 0; 
u32 oprand2 = 0;
u32 info1 = 0;		//info1 is used for return register destination and for any instruction that needs 3+ slots for data
u32 info2 = 0;		//info2 used for br1 instructions which need 4 slots for reg1, reg2, branch type, and address
u32 result = 0;
u32 result2 = 0; 	//used when more then 1 var needs to be transfered from Execute to Store
u32 result3 = 0;	//used to for store which needs 3 results passed to it
u8 memory[memorySize] = {0};
bool haltFlag;

void printRegisters(){
	for(u8 i = 0; i < 16; i++)
		printf("r%02d:0x%08x\n", i, registers[i]);
	printf("PC :0x%08x\n", programCounter);
	
	//assignment didn't specify if we should print internal registers
	printf("CI :0x%08x\n", currentInstruction);
	printf("OP1 :0x%08x\n", oprand1);
	printf("OP2 :0x%08x\n", oprand2);
	printf("IN1 :0x%08x\n", info1);
	printf("IN2 :0x%08x\n", info2);
	printf("RES1 :0x%08x\n", result);
	printf("RES2 :0x%08x\n", result2);
	printf("RES3 :0x%08x\n", result3);
}

void printMemory(){
	printf("============================================================\n");
	for(u32 i = 0; i < memorySize; i++){
		if(i % 20 == 0 && i != 0)
			printf("\n");
		printf("%02x ", memory[i]);
	}
	printf("\n");
}

//protects against memory out-of-bounds errors since all memory access goes though this
//returns a pointer so it can also be used to set memory locations
u8* memoryAt(u32 address){
	if(address < 0 || address > memorySize - 1){
		printf("RUNTIME ERROR: address 0x%x is out of bounds!\n", address);
		haltFlag = true;  
		return memory; 	//prevent a null pointer dereference
	}
	return memory + address;
}

void write(u32 address, u32 data){
	*memoryAt(address)=data>>24;
	*memoryAt(address + 1)=data>>16&0xff;
	*memoryAt(address + 2)=data>>8&0xff;
	*memoryAt(address + 3)=data&0xff;
}

void read(u32 address, u32* data){
	*data = *memoryAt(address)<<24 | *memoryAt(address + 1)<<16 | *memoryAt(address+2)<<8 | *memoryAt(address+3);
}

//Macros for decodes using the currentInstruction obtained with fetch
#define N1 (currentInstruction>>28)	                //Nibble 0 of the instruction 
#define N2 (currentInstruction>>24&0xf)             //Nibble 1 of the instruction
#define N3 (currentInstruction>>20&0xf)             //Nibble 2 of the instruction
#define N4 (currentInstruction>>16&0xf)             //Nibble 3 of the instruction
#define B2 (currentInstruction>>16&0xff)            //Byte 2 of the instruction,
#define W2 (currentInstruction&0xffff)              //Word 2 of the instruction, only used for br1 type instructions
#define B2W2 (currentInstruction&0xffffff)          //Bytes 2 through 4, only used for br2 type instructions
#define C5 (currentInstruction>>22&0b11)            //Crumb 5, only used for stack type instructions

//Abbreviations to make the macro code more concise 
#define PC programCounter
#define OP1 oprand1
#define OP2 oprand2
#define INF1 info1
#define INF2 info2
#define RES result
#define RES2 result2
#define RES3 result3
#define HALT haltFlag
#define MEM(n)(*memoryAt(vm, n))
#define REG(n) (registers[n])
#define BRANCH(n, m) (branchLookupTable[n](m)())
#define STACK(n, m) (stackLookupTable[n](m)())
#define INTERRUPT(n) (interruptLookupTable[n]())
#define WRITE(L, D) write(L, D)
#define READ(L, D) read(L, &D)
//denotes that the computer does nothing on the given DECODE, EXECUTE, STORE
#define NOTHING 

/*
Using the closest thing I have to lambda expressions in C, function pointers and macros.
This firstly make the code look cleaner and easier to follow since the DECODE, EXECUTE, or STORE
can all be seen next to each other despite being executed separately. Secondly This allows me to put
the pointers to these functions into an array, creating a map that can look up instructions in O(1) time,
in contrast to the O(n) time that would take going through an if-else chain or switch case block. 
*/


//definition of instruction step executors and functions to return them from a map
typedef void (*step)();
typedef step (*stepLookup)(int);

/*"lambda expression" generator:
for each template generate a functions for DECODE, EXECUTE, and STORE, place them in a O(1) lookup table (array)
and create a function to pick which function gets used. The need for the executor lookup function is due issues constructing
arrays or arrays of function pointers though macros in the global name space, which leads to lots of issues like the inability to
compute array initializer members at run time.
*/
#define TEMPLATE(N, D, E, S) \
void N##Decode(){D} \
void N##Execute(){E} \
void N##Store(){S} \
step N##Lookup[3] = {N##Decode, N##Execute, N##Store}; \
step N(int i){return N##Lookup[i];}

//effectively enums for deciding which mode to lookup 
#define DECODE 0
#define EXECUTE 1
#define STORE 2

const step interruptLookupTable[] = {printRegisters, printMemory};

/*Format: Template(Instruction Name, Decode Code, Execute Code, Store Code,)*/

//          Name                               		Decode                                       Execute                         Store
TEMPLATE(branchIfLess,            OP1 = REG(N3); OP2 = REG(N4); INF1 = N2; INF2 = W2;,   RES = OP1 < OP2 ? 2 * INF2 : 4;,    PC += RES;)
TEMPLATE(branchIfLessOrEqual,     OP1 = REG(N3); OP2 = REG(N4); INF1 = N2; INF2 = W2;,   RES = OP1 <= OP2 ? 2 * INF2 : 4;,   PC += RES;)
TEMPLATE(branchIfEqual,           OP1 = REG(N3); OP2 = REG(N4); INF1 = N2; INF2 = W2;,   RES = OP1 == OP2 ? 2 * INF2 : 4;,   PC += RES;)
TEMPLATE(branchIfNotEqual,        OP1 = REG(N3); OP2 = REG(N4); INF1 = N2; INF2 = W2;,   RES = OP1 != OP2 ? 2 * INF2 : 4;,   PC += RES;)
TEMPLATE(branchIfGreater,         OP1 = REG(N3); OP2 = REG(N4); INF1 = N2; INF2 = W2;,   RES = OP1 > OP2 ? 2 * INF2 : 4;,    PC += RES;)
TEMPLATE(branchIfGreaterOrEqual,  OP1 = REG(N3); OP2 = REG(N4); INF1 = N2; INF2 = W2;,   RES = OP1 >= OP2 ? 2 * INF2 : 4;,   PC += RES;)
TEMPLATE(call,                    OP1 = B2W2;,                                           RES = OP1 * 2;,                     REG(15) -= 4; WRITE(REG(15), PC + 4); PC = RES;)
TEMPLATE(jump,                    OP1 = B2W2;,                                           RES = B2W2 * 2;,                    PC = RES;)

const stepLookup branchLookupTable[] =
{branchIfLess, branchIfLessOrEqual, branchIfEqual, branchIfNotEqual, branchIfGreater, branchIfGreaterOrEqual, call, jump};

//		Name                Decode                      EXECUTE                            Store
TEMPLATE(pop,	    OP1 = N2; READ(REG(15), OP2);,     RES = OP1; RES2 = OP2;,       REG(RES) = RES2; REG(15) += 4; PC += 2;)
TEMPLATE(push,	    OP1 = REG(N2); OP2 = REG(15);,     RES = OP1; RES2 = OP2 - 4;,   WRITE(RES2, RES); REG(15) = RES2; PC += 2;)
TEMPLATE(return_,   READ(REG(15), OP1);,               RES = OP1;,                   REG(15) -= 4; PC = RES;)

const stepLookup stackLookupTable[] = {return_, push, pop};

//		Name                            Decode                           EXECUTE                            Store
TEMPLATE(halt,      NOTHING;,                                   NOTHING;,                             HALT = true;)
TEMPLATE(add,       OP1 = REG(N2); OP2 = REG(N3); INF1 = N4;,   RES = OP1 + OP2; RES2 = INF1;,        REG(RES2) = RES; PC += 2;)
TEMPLATE(and_,      OP1 = REG(N2); OP2 = REG(N3); INF1 = N4;,   RES = OP1 & OP2; RES2 = INF1;,        REG(RES2) = RES; PC += 2;)
TEMPLATE(divide,    OP1 = REG(N2); OP2 = REG(N3); INF1 = N4;,   RES = OP1 / OP2; RES2 = INF1;,        REG(RES2) = RES; PC += 2;)
TEMPLATE(multiply,  OP1 = REG(N2); OP2 = REG(N3); INF1 = N4;,   RES = OP1 * OP2; RES2 = INF1;,        REG(RES2) = RES; PC += 2;)
TEMPLATE(or_,       OP1 = REG(N2); OP2 = REG(N3); INF1 = N4;,   RES = OP1 | OP2; RES2 = INF1;,        REG(RES2) = RES; PC += 2;)
TEMPLATE(subtract,  OP1 = REG(N2); OP2 = REG(N3); INF1 = N4;,   RES = OP1 - OP2; RES2 = INF1;,        REG(RES2) = RES; PC += 2;)
TEMPLATE(branch,    BRANCH(N2, DECODE);,                        BRANCH(N2, EXECUTE);,                 BRANCH(N2, STORE);)
TEMPLATE(load_,     OP1 = N2; READ(2*REG(N3)+2*N4, OP2);,       RES = OP1; RES2 = OP2;,               REG(RES) = RES2; PC += 2;)
TEMPLATE(store_,    OP1 = N2; OP2 = N3; INF1 = N4;,             RES = OP1; RES2 = OP2; RES3 = INF1;,  WRITE(2 * REG(RES2) + 2 * RES3, REG(RES)); PC += 2;)
TEMPLATE(stack,     STACK(C5, DECODE);,                         STACK(C5, EXECUTE);,                  STACK(C5, STORE);)
TEMPLATE(move_,     OP1 = N2; OP2 = B2;,                        RES = OP1; RES2 = OP2;,               REG(RES) = RES2; PC += 2;)
TEMPLATE(interrupt, OP1 = B2;,                                  INTERRUPT(OP1);,                      PC += 2;)


const stepLookup opcodeLookupTable[] = 
{halt, add, and_, divide, multiply, subtract, or_, branch, load_, store_, stack, move_, interrupt};

#undef HALT
#undef N2
#undef N3
#undef N4
#undef B2
#undef W2
#undef B2W2
#undef C5
#undef PC
#undef OP1
#undef OP2
#undef INF1 
#undef INF2 
#undef RES 
#undef RES2 
#undef RES3 
#undef HALT 
#undef MEM
#undef REG
#undef BRANCH
#undef STACK
#undef INTERRUPT
#undef WRITE
#undef READ
#undef NOTHING
#undef TEMPLATE

void fetch(){
	read(programCounter, &currentInstruction);
	opcode = currentInstruction>>28;
}

void decode(){
	opcodeLookupTable[opcode](DECODE)();
}

void execute(){
	opcodeLookupTable[opcode](EXECUTE)();
}

void store(){
	opcodeLookupTable[opcode](STORE)();
}

void load(char* filename){
	FILE *in = fopen(filename,"r");
	fseek(in, 0, SEEK_END);
	int fsize = ftell(in);
	rewind(in);
	fread(memory, sizeof(u8), fsize, in);
}

/*
int main(int argc, char **argv){
	if (argc != 2){
		printf ("vm inputFile.bin\n");
		exit(1); 
	}
	load(argv[1]);
	registers[15] = memorySize;
	while(!haltFlag){
		fetch();
		decode();
		execute();
		store();
	}
}*/

#endif
