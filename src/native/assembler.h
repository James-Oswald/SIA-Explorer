
#ifndef ASM_H
#define ASM_H

/*
	James Oswald
	ICSI 404
	Assembler Assignment
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>

#define MAX_ERR_SIZE 200
#define MAX_LINE_SIZE 100

const uint32_t maxErrorSize = MAX_ERR_SIZE;
const uint32_t maxLineSize = MAX_LINE_SIZE;

uint8_t getRegister(char* registerName, uint32_t lineNumber, char* errorMessage){
	if(registerName == NULL)
		return 0;
	char* endptr = NULL;
	uint32_t registerNumber = strtoul(registerName + 1, &endptr, 10);
	if(registerName[0] != 'r' || endptr[0] != '\0' || registerNumber > 15 || registerNumber < 0){
		snprintf(errorMessage, MAX_ERR_SIZE, "ERROR Line %d: %s is not a valid register name!\n", lineNumber, registerName);
		return 0;
	}
	return registerNumber;
}

uint32_t getImmediate(char* immediateAsString, uint8_t maxBits, uint32_t lineNumber, char* errorMessage, char* warningMessage){
	if(immediateAsString == NULL)
		return 0;
	uint32_t maxValue = ((1<<maxBits) - 1);
	char* endptr = NULL;
	uint32_t num = strtoul(immediateAsString, &endptr, 10);
	if(endptr[0] != '\0'){
		snprintf(errorMessage, MAX_ERR_SIZE, "ERROR Line %d: %s is not a valid number!\n", lineNumber, immediateAsString);
		return 0;
	}
	if(num > maxValue)
		snprintf(warningMessage, MAX_ERR_SIZE, "WARNING Line %d: %d was too large to fit inside an immediate %d bit wide location, extra bits will be trimmed!\n", lineNumber, num, maxBits);
	return maxValue & num;
}

//assemblerFunc function signature type
typedef uint32_t (*assemblerFunc)(uint8_t Opcode, uint8_t information, char** operands, uint32_t lineNumber, char* errorMessage, char* warningMessage);

//creates a new assembler function with name N and body F using an assemblerFunc signature
#define ASSEMBLER(N, F) uint32_t N(uint8_t opc, uint8_t inf, char** opr, uint32_t ln, char* errMsg, char* warnMsg){F}

#define OPCODE opc
#define INFO inf			
#define OPERAND1 opr[0]		
#define OPERAND2 opr[1]		
#define OPERAND3 opr[2]

//wrapper for getRegister
#define GET_REGISTER(N) getRegister(N, ln, errMsg)

//wrapper for getImm
#define GET_IMMEDIATE(N, S) getImmediate(N, S, ln, errMsg, warnMsg)

//		     Name										Assembler Logic
ASSEMBLER(assemble_3R,      return OPCODE<<12 | GET_REGISTER(OPERAND1)<<8 | GET_REGISTER(OPERAND2)<<4 | GET_REGISTER(OPERAND3);)
ASSEMBLER(assemble_br1,     return OPCODE<<28 | INFO<<24 | GET_REGISTER(OPERAND1)<<20 | GET_REGISTER(OPERAND2)<<16 | GET_IMMEDIATE(OPERAND3, 16);)
ASSEMBLER(assemble_br2,     return OPCODE<<28 | INFO<<24 | GET_IMMEDIATE(OPERAND1, 24);)
ASSEMBLER(assemble_ls,      return OPCODE<<12 | GET_REGISTER(OPERAND1)<<8 | GET_REGISTER(OPERAND2)<<4 | GET_IMMEDIATE(OPERAND3, 4);)
ASSEMBLER(assemble_stack,   return OPCODE<<12 | GET_REGISTER(OPERAND1)<<8 | INFO<<6;)
ASSEMBLER(assemble_move,    return OPCODE<<12 | GET_REGISTER(OPERAND2)<<8 | GET_IMMEDIATE(OPERAND1, 8);)

#undef OPCODE
#undef INFO
#undef OPERAND1
#undef OPERAND2
#undef OPERAND3
#undef GET_REGISTER
#undef GET_IMMEDIATE
#undef ASSEMBLER

//struct representing a SIA instruction, used for instruction lookup and error checking
typedef struct instruction_type{
	const char* name;				
	uint8_t opcode;					
	uint8_t size;					
	uint8_t numberOfOperands;			
	assemblerFunc assembler;		//The instruction type, stored as a pointer to its assembler function 
	uint8_t info;					//Additional info, used for branch type and stack operation type
} instruction;

//used by qsort, compares two instructions by name
int instrCmp(const void* instruction1, const void* instruction2){
	const char* instruction1Name = ((instruction*)instruction1)->name;
	const char* instruction2Name = ((instruction*)instruction2)->name;
	return strcmp(instruction1Name, instruction2Name);
}

//used by bsearch, compares a string and an instruction by name
int strInstrCmp(const void *s, const void *i){
	char* x = (char*)s;
	const char* y = ((instruction*)i)->name;
	return strcmp(x, y);
}

#define Null 0

//Finds an instruction by name in O(log(n)) time
//better then an if else chain, switch case, or linear search which would have been O(n)
instruction* instrLookup(char* name){
	//static const uint8_t Null = 0;
	static bool first = true;
	static instruction instructionTable[22] =
	{
		//Name                   opcode size  numOpr   assembler           info
		{"add",                     1,   2,     3,    assemble_3R,         Null},
		{"and",                     2,   2,     3,    assemble_3R,         Null},
		{"divide",                  3,   2,     3,    assemble_3R,         Null},
		{"halt",                    0,   2,     0,    assemble_3R,         Null},
		{"multiply",                4,   2,     3,    assemble_3R,         Null},
		{"or",                      6,   2,     3,    assemble_3R,         Null},
		{"subtract",                5,   2,     3,    assemble_3R,         Null},
		{"branchifless",            7,   4,     3,    assemble_br1,        0}, 
		{"branchiflessorequal",     7,   4,     3,    assemble_br1,        1},
		{"branchifequal",           7,   4,     3,    assemble_br1,        2},
		{"branchifnotequal",        7,   4,     3,    assemble_br1,        3},
		{"branchifgreater",         7,   4,     3,    assemble_br1,        4},
		{"branchifgreaterorequal",  7,   4,     3,    assemble_br1,        5},
		{"call",                    7,   4,     1,    assemble_br2,        6},
		{"jump",                    7,   4,     1,    assemble_br2,        7},
		{"load",                    8,   2,     3,    assemble_ls,         Null},
		{"store",                   9,   2,     3,    assemble_ls,         Null},
		{"pop",                     10,  2,     1,    assemble_stack,      0b10},
		{"push",                    10,  2,     1,    assemble_stack,      0b01},
		{"return",                  10,  2,     0,    assemble_stack,      0b00},
		{"move",                    11,  2,     2,    assemble_move,       Null},
		{"interrupt",               12,  2,     1,    assemble_move,       Null}
	};
	if(first){	//sort the lookup table for bsearch on the first call of this function
		qsort((void*)instructionTable, 22, sizeof(instruction), instrCmp);
		first = false;
	}
	instruction* instrPtr = (instruction*)bsearch((void*)name, (void*)instructionTable, 22, sizeof(instruction), strInstrCmp);
	return instrPtr;
}

#undef Null

void lowercase(char* string){
	for(uint32_t i = 0; string[i]; i++)
		if(string[i] >= 'A' && string[i] <= 'Z')
		string[i] = string[i] + 32;
}

void replaceTabsAndNewlines(char* string){
	for(uint32_t i = 0; string[i]; i++)
		if(string[i] == '\t' || string[i] == '\n')
			string[i] = ' ';
}

void fixWhitespace(char* string){
	uint32_t si = 0, csi = 0;
	for(; string[si]; si++)		
		if(string[si] != ' ' || (si > 0 && string[si-1] != ' '))
			string[csi++] = string[si];
	for(; string[csi-1] == ' '; csi--);
	string[csi] = '\0';
}

uint32_t countOccurrences(char* string, char find){
	uint32_t occurrences = 0;
	for(uint32_t i = 0; string[i]; i++)
		if(string[i] == find)
			occurrences++;
	return occurrences;
}

void getWords(char* line, char** wordsList){
	wordsList[0] = strtok(line, " ");
	for(uint32_t i = 0; wordsList[i] != NULL;)
		wordsList[++i] = strtok(NULL, " ");
}

void assembleLine(char* line, uint8_t* outputBytes, uint32_t* numberOutputBytes, uint32_t lineNumber, char* errorMessage, char* warningMessage){
	replaceTabsAndNewlines(line);
	fixWhitespace(line);
	lowercase(line);
	uint32_t numberOfWords = countOccurrences(line, ' ') + 1;
	uint32_t wordsSize = (numberOfWords > 4 ? numberOfWords : 4) + 1; //must have min 4 to protect against out of bounds errs when assembling, +1 for extra null space in getWords
	char** words = (char**)calloc(wordsSize, sizeof(char*)); 
	getWords(line, words);
	instruction* currentInstruction = instrLookup(words[0]);
	uint32_t assembledInstruction = 0;
	*numberOutputBytes = 2;
	if(currentInstruction == NULL){
		snprintf(errorMessage, MAX_ERR_SIZE, "ERROR Line %d: \"%s\" Is not a valid instruction!\n", lineNumber, words[0]);
		currentInstruction = instrLookup("halt"); //if the instruction does not exist, replace it with 0x0000 (halt) and produce an error
	}else{
		if(currentInstruction->numberOfOperands != numberOfWords - 1)
			snprintf(errorMessage, MAX_ERR_SIZE, "ERROR Line %d: \"%s\" expects %d operand(s), but received %d\n", lineNumber, currentInstruction->name, currentInstruction->numberOfOperands, numberOfWords - 1);
		assembledInstruction = currentInstruction->assembler(currentInstruction->opcode, currentInstruction->info, words + 1, lineNumber, errorMessage, warningMessage); //calls the assembler function
		*numberOutputBytes = currentInstruction->size;
	}
	for(uint32_t i = 0; i < currentInstruction->size; i++){
		outputBytes[i] = (assembledInstruction>>(8*(currentInstruction->size - i - 1))) & 0xff;
	}
	free(words);
}

/*int main(int argc, char **argv){
	if (argc != 3){
		printf("assemble inputFile outputFile\n");
		return 1;
	}
	FILE* in = fopen(argv[1],"r");
	if(in == NULL){
		printf("unable to open input file\n");
		return 1;
	}
	FILE* out = fopen(argv[2],"wb");
	if (out == NULL){
		printf("unable to open output file\n");
		return 1; 
	}
	char line[MAX_LINE_SIZE], errorMessage[MAX_ERR_SIZE], warningMessage[MAX_ERR_SIZE];
	uint8_t outputBytes[4];
	uint32_t numberOutputBytes;
	for(uint32_t lineNumber = 0; !feof(in); lineNumber++){
		if(fgets(line, MAX_LINE_SIZE, in) != NULL && line[0] != '\n'){
			errorMessage[0] = '\0';
			warningMessage[0] = '\0';
			assembleLine(line, outputBytes, &numberOutputBytes, lineNumber + 1, errorMessage, warningMessage);
			if(errorMessage[0] != '\0')
				printf("%s", errorMessage);
			if(warningMessage[0] != '\0')
				printf("%s", warningMessage);
			fwrite(outputBytes, 1, numberOutputBytes, out);
		}
	}
	fclose(in);
	fclose(out);
	return 0;
}*/

#endif