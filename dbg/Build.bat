
call emcc -O3 ./src/native/main.c -o ./src/website/SIA.js -s EXTRA_EXPORTED_RUNTIME_METHODS=['ccall','cwrap','getValue'] -s EXPORTED_FUNCTIONS=['_malloc','_main']
rmdir /s /q build
mkdir build
xcopy .\src\website .\build
