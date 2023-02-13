#include <fstream>
#include "pocketpy.h"

#ifndef __EMSCRIPTEN__

int main(int argc, char** argv){
    VM* vm = pkpy_new_vm(true);
    vm->bind_builtin_func<0>("input", [](VM* vm, pkpy::Args& args){
        static std::string line;
        std::getline(std::cin, line);
        return vm->PyStr(line);
    });
    if(argc == 1){
        REPL* repl = pkpy_new_repl(vm);
        bool need_more_lines = false;
        while(true){
            (*vm->_stdout) << (need_more_lines ? "... " : ">>> ");
            std::string line;
            if (!std::getline(std::cin, line)) break;
            need_more_lines = pkpy_repl_input(repl, line.c_str());
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
        ret = vm->exec(src.c_str(), filename, EXEC_MODE);
        pkpy_delete(vm);
        return ret != nullptr ? 0 : 1;
    }

__HELP:
    std::cout << "Usage: pocketpy [filename]" << std::endl;
    return 0;
}

#endif