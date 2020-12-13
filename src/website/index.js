
"use strict"

let initalized = false;
let assembleLine = null;
let errorSize = null;

function start(){
    try{
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
    let output = document.getElementById("SIAOutput");
    let debug = document.getElementById("SIADebug");
    output.value = "";
    debug.value = "";
    let lines = document.getElementById("SIAInput").value.split("\n");

    //return values
    let maxErrSize = errorSize();                   //The max length of an error or warning message
    let outputBytesPtr = Module._malloc(4);         //The machine code of the instruction
    let numOutputBytesPtr = Module._malloc(4);      //The number length of the machine code
    let errorPtr = Module._malloc(maxErrSize);      //The pointer to an error message
    let warningPtr = Module._malloc(maxErrSize);    //The pointer to a warning message

    let outputErrorTags = [];
    let outputWarningTags = [];
    let debugErrorTags = [];
    let debugWarningTags = [];
    let inputErrorTags = [];
    let inputWarningTags = [];
    
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
        let outputTagStart = output.value.length;
        for(let j = 0; j < numOutputBytes; j++)
            output.value += ("0" + outputBytes[j].toString(16)).slice(-2);
        let outputTagEnd = output.value.length;
        output.value += "/n";
        
        let debugTagStart = output.value.length;
        if(error != "")
            debug.value += error + "/n"
        if(warning != "")
            debug.value += warning + "/n"
        let debugTagEnd = output.value.length;

    }   
    console.log(errorStrings);
    console.log(warningStrings);
}