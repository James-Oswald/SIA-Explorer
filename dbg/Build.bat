rmdir /s /q build
mkdir build
xcopy .\src\website .\build
emcc -O3 ./src/native/main.c -o ./build/SIA.js