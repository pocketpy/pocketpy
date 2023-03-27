#include <fstream>
#include <filesystem>

#include "pocketpy.h"

#ifdef _WIN32

#include <Windows.h>

std::string getline(bool* eof=nullptr) {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    std::wstringstream wss;
    WCHAR buf;
    DWORD read;
    while (ReadConsoleW(hStdin, &buf, 1, &read, NULL) && buf != L'\n') {
        if(eof && buf == L'\x1A') *eof = true;  // Ctrl+Z
        wss << buf;
    }
    std::wstring wideInput = wss.str();
    int length = WideCharToMultiByte(CP_UTF8, 0, wideInput.c_str(), (int)wideInput.length(), NULL, 0, NULL, NULL);
    std::string output;
    output.resize(length);
    WideCharToMultiByte(CP_UTF8, 0, wideInput.c_str(), (int)wideInput.length(), &output[0], length, NULL, NULL);
    if(!output.empty() && output.back() == '\r') output.pop_back();
    return output;
}

#else

std::string getline(bool* eof=nullptr){
    std::string line;
    if(!std::getline(std::cin, line)){
        if(eof) *eof = true;
    }
    return line;
}

#endif

#ifndef __EMSCRIPTEN__

int main(int argc, char** argv){
    pkpy::VM* vm = pkpy_new_vm(true);
    vm->bind_builtin_func<0>("input", [](pkpy::VM* vm, pkpy::Args& args){
        return VAR(getline());
    });
    if(argc == 1){
        pkpy::REPL* repl = pkpy_new_repl(vm);
        bool need_more_lines = false;
        while(true){
            (*vm->_stdout) << (need_more_lines ? "... " : ">>> ");
            bool eof = false;
            std::string line = getline(&eof);
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
            return 1;
        }        
        std::ifstream file(filepath);
        if(!file.is_open()) return 1;
        std::string src((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        // set parent path as cwd
        std::filesystem::current_path(filepath.parent_path());

        pkpy::PyVarOrNull ret = nullptr;
        ret = vm->exec(src.c_str(), argv_1, pkpy::EXEC_MODE);
        pkpy_delete(vm);
        return ret != nullptr ? 0 : 1;
    }

__HELP:
    std::cout << "Usage: pocketpy [filename]" << std::endl;
    return 0;
}

#endif