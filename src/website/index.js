
"use strict"

//default values for various program states
let initalized = false;        //wasm initilization 
//let displayOnAll = true;
let machineCodeLen = 0;

//wasm functions, will be set if initalized == true
let assembleLine = null;
let errorSize = null;
let memorySize = null;
let vmStep = null;
let loadPgrm = null;
let getMem = null;
let getReg = null;
let getPC = null;
let getHalt = null;

//init wasm functions
function start(){
    try{
        lwtahSetUpdateOnInput(false);
        assembleLine = Module.cwrap("assembleLineW", "null", ["string", "number", "number", "number", "number", "number"]);
        errorSize = Module.cwrap("getMaxErrorSize", "number", []);
        memorySize = Module.cwrap("getMemorySize", "number", []);
        vmStep = Module.cwrap("vmStep", "null", []);
        loadPgrm = Module.cwrap("loadProgram", "null", ["number", "number"]);   //u8* memory, int length 
        getMem = Module.cwrap("getMem", "number", []);  //ret u8* to mem
        getReg = Module.cwrap("getReg", "number", []);  //ret u32* to registers
        getPC = Module.cwrap("getPC", "number", []);    //ret u32 PC value
        getHalt = Module.cwrap("getHalt", "number", []);
        initalized = true;
        console.log("WASM init sucessfull");
        if(document.readyState === "complete")
            setUpProgram();
        else
            window.addEventListener("load", setUpProgram);
    }catch(err){
        console.error(err);
    }
}

function setUpProgram(){
    document.getElementById("SIAInput").value =
`move 0 r4
move 20 r5
move 1 r6
move 0 r0
move 0 r1
push r1
move 1 r2
push r2
branchifequal r0 r5 9
add r1 r2 r3 
add r2 r4 r1
add r3 r4 r2
push r2
add r0 r6 r0
jump 8
halt`;
    assemble();
}
//=====================================================
//Swap between assembler and VM modes

window.addEventListener("load", hideVM);

function hideVM(){
    if(lwtahInit){  //black magic to make sure we have our box sizes
        let vm = document.getElementById("vmCont");
        vm.style.display = "none";
    }else
        setTimeout(hideVM, 100);
}

function setMode(mode){
    let asm = document.getElementById("asmCont");
    let vm = document.getElementById("vmCont");
    if(mode == "asm"){
        asm.style.display = "block";
        vm.style.display = "none";
    }else{
        asm.style.display = "none";
        vm.style.display = "block";
    }
}

//=====================================================
//application calls

function assemble(){
    if(!initalized)
        throw new Error("Can't assemble untill WASM is initilized");
    let input = document.getElementById("SIAInput");
    let output = document.getElementById("SIAOutput");
    let debug = document.getElementById("SIADebug");
    output.value = "";
    debug.value = "";
    let lines = input.value.split("\n");

    //return values
    let maxErrSize = errorSize();                   //The max length of an error or warning message
    let outputBytesPtr = Module._malloc(4);         //The machine code of the instruction
    let numOutputBytesPtr = Module._malloc(4);      //The number length of the machine code
    let errorPtr = Module._malloc(maxErrSize);      //The pointer to an error message
    let warningPtr = Module._malloc(maxErrSize);    //The pointer to a warning message

    let tags = [];
    let machineCode = [];
    let processedInput = "";
    
    
    for(let i = 0; i < lines.length; i++){
        let line = lines[i];
        for(let j = 0; j < maxErrSize; j++){    //0s out the entire error and warning buffer 
            Module.HEAPU8.set([0], errorPtr + j);
            Module.HEAPU8.set([0], warningPtr + j);
        }
        assembleLine(line, outputBytesPtr, numOutputBytesPtr, i, errorPtr, warningPtr);
        let numOutputBytes = Module.getValue(numOutputBytesPtr, "i32");
        console.log(numOutputBytes);
        //fix this with numOutputBytes after testing <--- OG comment? I think i already did this last year.
        let outputBytes = new Uint8Array(Module.HEAPU8.buffer, outputBytesPtr, numOutputBytes); 
        let error = String.fromCharCode.apply(null, new Uint8Array(Module.HEAPU8.buffer, errorPtr, maxErrSize));
        let warning = String.fromCharCode.apply(null, new Uint8Array(Module.HEAPU8.buffer, warningPtr, maxErrSize));
        error = error.replace(/\0[\s\S]*$/g,'').trim();
        warning = warning.replace(/\0[\s\S]*$/g,'').trim();

        let inputTagStart = processedInput.length;
        processedInput += line + "\n";
        let inputTagEnd = processedInput.length;

        let outputTagStart = output.value.length;
        for(let j = 0; j < numOutputBytes; j++){
            output.value += ("0" + outputBytes[j].toString(16)).slice(-2);
            machineCode.push(outputBytes[j]);
        }
        let outputTagEnd = output.value.length;
        output.value += "\n";

        let debugTagStart = debug.value.length;
        if(error != "")
            debug.value += error + "\n";
        else if(warning != "")
            debug.value += warning + "\n";
        let debugTagEnd = debug.value.length;
        
        if(error != "")
            tags.push([[inputTagStart, inputTagEnd],[outputTagStart, outputTagEnd],[debugTagStart,debugTagEnd], "#f4cccc"]);
        else if(warning != "")
            tags.push([[inputTagStart, inputTagEnd],[outputTagStart, outputTagEnd],[debugTagStart,debugTagEnd], "#fff2cc"]);
    }

    //Send the assembled machine code to the VM
    let machineCodeBuffer = Module._malloc(machineCode.length);
    Module.HEAPU8.set(Uint8Array.from(machineCode), machineCodeBuffer);
    loadPgrm(machineCodeBuffer, machineCode.length);
    machineCodeLen = machineCode.length
    displayVM();

    //Error and warning highlighting code 
    lwtahClear("SIAInput");
    lwtahClear("SIAOutput");
    lwtahClear("SIADebug");
    for(let i = 0; i < tags.length; i++){
        lwtahAddRange("SIAInput", tags[i][0], tags[i][3]);
        lwtahAddRange("SIAOutput", tags[i][1], tags[i][3]);
        lwtahAddRange("SIADebug", tags[i][2], tags[i][3]);
    }   
}

function step(){
    if(!initalized)
        throw new Error("Can't run VM untill WASM is initilized");
    vmStep();
    displayVM();
}

function displayVM(){
    if(!initalized)
        throw new Error("Can't Display VM untill WASM is initilized");
    let regArea = document.getElementById("registers");
    let memArea = document.getElementById("memory");
    regArea.value = "";
    memArea.value = "";
    lwtahClear("registers");
    lwtahClear("memory");
    let memSize = memorySize();
    let memory = new Uint8Array(Module.HEAPU8.buffer, getMem(), memSize);
    let registers = new Uint32Array(Module.HEAPU32.buffer, getReg(), 16);
    let pc = getPC();
    for(let i = 0; i < memory.length; i++){
        let tagStart = memArea.value.length;
        memArea.value += ("0" + memory[i].toString(16)).slice(-2) + " ";
        let tagEnd = memArea.value.length;

        //coloring rules for memory
        if(i == pc)
            lwtahAddRange("memory", [tagStart, tagEnd], "#f4cccc");
        else if(i >= registers[15])
            lwtahAddRange("memory" , [tagStart, tagEnd], "#d9d2e9");
        else if(i < machineCodeLen)
            lwtahAddRange("memory" , [tagStart, tagEnd], "#fff2cc");    
    }
    regArea.value += "Halt: " + getHalt() + "\n";
    let toHex = i =>"0x" + ("0000000" + i.toString(16)).slice(-8) //32-bit hex converter
    regArea.value += "PC:   " + toHex(pc) + "\n";
    for(let i = 0; i < registers.length; i++){
        regArea.value += ("r" + i + ":   ").slice(0, 6) + toHex(registers[i]) + "\n";
    }
}