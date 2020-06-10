/*#ifndef ASM_HPP
#define ASM_HPP

#include<cstdint>
#include<vector>
#include<string>

using namespace std;

namespace Asm{
    extern "C"{
#include "assembler.h"
    }
}

void assembleLine(const string& line, vector<uint8_t>& output,  uint32_t lineNumber, string& err, string& warn){
    char linep[Asm::maxLineSize];
    strcpy(linep, line.c_str());
    uint8_t outputBytes[4];
    uint32_t numOutputBytes;
    char errorMsg[Asm::maxErrorSize], warnMsg[Asm::maxErrorSize];
    errorMsg[0] = warnMsg[0] = '/0';
    Asm::assembleLine(linep, outputBytes, &numOutputBytes, lineNumber, errorMsg, warnMsg);
    if(errorMsg[0] != '\0')
        err = string(errorMsg);
    if(warnMsg[0] != '\0')
        warn = string(warnMsg);
    output = vector<uint8_t>(outputBytes, outputBytes + 4);
}

#endif
*/