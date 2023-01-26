#include <fstream>
#include <functional>

#include "pocketpy.h"

//#define PK_DEBUG_TIME

struct Timer{
    const char* title;
    Timer(const char* title) : title(title) {}
    void run(std::function<void()> f){
#ifdef PK_DEBUG_TIME
        auto start = std::chrono::high_resolution_clock::now();
        f();
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0;
        std::cout << title << ": " << elapsed << " s" << std::endl;
#else
        f();
#endif
    }
};



#ifndef __NO_MAIN

int main(int argc, char** argv){
    if(argc == 1){
        VM* vm = pkpy_new_vm(true);
        REPL repl(vm);
        int result = -1;
        while(true){
            (*vm->_stdout) << (result==0 ? "... " : ">>> ");
            std::string line;
            std::getline(std::cin, line);
            result = pkpy_repl_input(&repl, line.c_str());
        }
        return 0;
    }
    
    if(argc == 2){
        std::string filename = argv[1];
        if(filename == "-h" || filename == "--help") goto __HELP;

        std::ifstream file(filename);
        if(!file.is_open()){
            std::cerr << "File not found: " << filename << std::endl;
            return 1;
        }
        std::string src((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        VM* vm = pkpy_new_vm(true);
        Timer("Running time").run([=]{
            vm->exec(src.c_str(), filename, EXEC_MODE);
        });
        pkpy_delete(vm);
        return 0;
    }

__HELP:
    std::cout << "Usage: pocketpy [filename]" << std::endl;
    return 0;
}

#endif