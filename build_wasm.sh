rm -rf build/wasm/
mkdir -p build/wasm/
emcc src/main.cpp -fexceptions -sEXIT_RUNTIME -sEXPORTED_FUNCTIONS=_repl_input,_repl_start -sEXPORTED_RUNTIME_METHODS=ccall -o build/wasm/pocketpy.js