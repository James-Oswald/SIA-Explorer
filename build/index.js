
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
    output.innerHTML = "";
    debug.innerHTML = "";
    let lines = document.getElementById("SIAInput").value.split("\n");
    let maxErrSize = errorSize();
    let outputBytesPtr = Module._malloc(4);
    let numOutputBytesPtr = Module._malloc(4);
    let errorPtr = Module._malloc(maxErrSize);
    let warningPtr = Module._malloc(maxErrSize);
    let inputIndex = 0;
    let outputIndex = 0;
    let debugIndex = 0;
    for(let i = 0; i < lines.length; i++){
        let line = lines[i];
        for(let j = 0; j < maxErrSize; j++){
            Module.HEAPU8.set([0], errorPtr + j)
            Module.HEAPU8.set([0], warningPtr + j);
        }
        assembleLine(line, outputBytesPtr, numOutputBytesPtr, i, errorPtr, warningPtr);
        let numOutputBytes = Module.getValue(numOutputBytesPtr, "i32");
        console.log(numOutputBytes);
        let outputBytes = new Uint8Array(Module.HEAPU8.buffer, outputBytesPtr, numOutputBytes); //fix this with numOutputBytes after testing
        let error = String.fromCharCode.apply(null, new Uint8Array(Module.HEAPU8.buffer, errorPtr, maxErrSize));
        let warning = String.fromCharCode.apply(null, new Uint8Array(Module.HEAPU8.buffer, warningPtr, maxErrSize));
        error = error.replace(/\0[\s\S]*$/g,'').trim();
        warning = warning.replace(/\0[\s\S]*$/g,'').trim();
        if(error != ""){
            errorStrings[2].push(error);
            debug.innerHTML += error + "&#13;&#10;"
            console.warn(error);
        }
        if(warning != ""){
            warningStrings[2].push(warning);
            debug.innerHTML += warning + "&#13;&#10;"
            console.warn(warning);
        }
        let outputString = "";
        for(let j = 0; j < numOutputBytes; j++)
            outputString += ("0" + outputBytes[j].toString(16)).slice(-2);
        output.innerHTML += outputString + "&#13;&#10;";
        if(error != ""){
            errorStrings[1].push(outputString);
            errorStrings[0].push(line);
        }
        else if(warning != ""){
            warningStrings[1].push(outputString);
            warningStrings[0].push(line);
        }
    }    
    console.log(errorStrings);
    console.log(warningStrings);
}