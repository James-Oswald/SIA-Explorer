
"use strict"

let initalized = false;
let assembleLine = null;
let errorSize = null;

function start(){
    try{
        lwtahSetUpdateOnInput(false);
        assembleLine = Module.cwrap("assembleLineW", "null", ["string", "number", "number", "number", "number", "number"]);
        errorSize = Module.cwrap("getMaxErrorSize", "number", []);
        initalized = true;
        console.log("WASM init sucessfull");
    }catch(err){
        console.error(err);
    }
}

function assembleCallback(warning, error){

}

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
    let processedInput = "";
    
    for(let i = 0; i < lines.length; i++){
        let line = lines[i];
        for(let j = 0; j < maxErrSize; j++){    //0s out the entire error and warning buffer? 
            Module.HEAPU8.set([0], errorPtr + j)
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
        for(let j = 0; j < numOutputBytes; j++)
            output.value += ("0" + outputBytes[j].toString(16)).slice(-2);
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
    lwtahClear("SIAInput");
    lwtahClear("SIAOutput");
    lwtahClear("SIADebug");
    for(let i = 0; i < tags.length; i++){
        lwtahAddRange("SIAInput", tags[i][0], tags[i][3]);
        lwtahAddRange("SIAOutput", tags[i][1], tags[i][3]);
        lwtahAddRange("SIADebug", tags[i][2], tags[i][3]);
    }   
}