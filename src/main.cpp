#include <fstream>
#include <filesystem>

#include "pocketpy.h"

#ifndef __EMSCRIPTEN__

int main(int argc, char** argv){
    pkpy::VM* vm = pkpy_new_vm();
    vm->bind_builtin_func<0>("input", [](pkpy::VM* vm, pkpy::ArgsView args){
        return VAR(pkpy::getline());
    });
    if(argc == 1){
        pkpy::REPL* repl = pkpy_new_repl(vm);
        bool need_more_lines = false;
        while(true){
            vm->_stdout(vm, need_more_lines ? "... " : ">>> ");
            bool eof = false;
            std::string line = pkpy::getline(&eof);
            if(eof) break;
            need_more_lines = pkpy_repl_input(repl, line.c_str());
        }
        pkpy_delete(vm);
        return 0;
    }
    
    if(argc == 2){
        std::string argv_1 = argv[1];
        if(argv_1 == "-h" || argv_1 == "--help") goto __HELP;

        std::filesystem::path filepath(argv[1]);
        filepath = std::filesystem::absolute(filepath);
        if(!std::filesystem::exists(filepath)){
            std::cerr << "File not found: " << argv_1 << std::endl;
            return 2;
        }        
        std::ifstream file(filepath);
        if(!file.is_open()){
            std::cerr << "Failed to open file: " << argv_1 << std::endl;
            return 3;
        }
        std::string src((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // set parent path as cwd
        std::filesystem::current_path(filepath.parent_path());

        pkpy::PyObject* ret = nullptr;
        ret = vm->exec(src.c_str(), filepath.filename().string(), pkpy::EXEC_MODE);
        pkpy_delete(vm);
        return ret != nullptr ? 0 : 1;
    }

__HELP:
    std::cout << "Usage: pocketpy [filename]" << std::endl;
    return 0;
}

#endif