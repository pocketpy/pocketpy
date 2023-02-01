#include <fstream>
#include <functional>

#include "pocketpy.h"

struct Timer{
    const char* title;
    Timer(const char* title) : title(title) {}
    void run(std::function<void()> f){
        auto start = std::chrono::high_resolution_clock::now();
        f();
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0;
        std::cout << title << ": " << elapsed << " s" << std::endl;
    }
};

#ifndef __NO_MAIN

int main(int argc, char** argv){
    VM* vm = pkpy_new_vm(true);
    pkpy_vm_bind_str__(vm, "builtins", "input", [](){
        static std::string line;
        std::getline(std::cin, line);
        return line.c_str();
    });
    if(argc == 1){
        REPL* repl = pkpy_new_repl(vm);
        int result = -1;
        while(true){
            (*vm->_stdout) << (result==0 ? "... " : ">>> ");
            std::string line;
            std::getline(std::cin, line);
            result = pkpy_repl_input(repl, line.c_str());
        }
        pkpy_delete(vm);
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
        PyVarOrNull ret = nullptr;

        if(filename.find("1.py") != std::string::npos || filename.find("2.py") != std::string::npos){
            Timer("Running time").run([&]{
                ret = vm->exec(src.c_str(), filename, EXEC_MODE);
            });
        }else{
            ret = vm->exec(src.c_str(), filename, EXEC_MODE);
        }
        pkpy_delete(vm);
        return ret != nullptr ? 0 : 1;
    }

__HELP:
    std::cout << "Usage: pocketpy [filename]" << std::endl;
    return 0;
}

#endif