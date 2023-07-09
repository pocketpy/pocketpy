#include <fstream>
#include <filesystem>
#include <iostream>

#include "pocketpy_c.h"


#ifdef _WIN32

void pkpy_platform_getline(char* buffer, int size, bool* eof){
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

    size = std::min<int>(size-1, output.size());
    for(int i=0; i<size; i++) buffer[i] = output[i];
    buffer[size] = '\0';
}

#else

void pkpy_platform_getline(char* buffer, int size, bool* eof){
    std::string output;
    if(!std::getline(std::cin, output)){
        if(eof) *eof = true;
    }
    size = std::min<int>(size-1, output.size());
    for(int i=0; i<size; i++) buffer[i] = output[i];
    buffer[size] = '\0';
}

#endif

// std::string f_input(){
//     return pkpy::platform_getline();
// }

int main(int argc, char** argv){
    char buffer[1024];
#if _WIN32
    // implicitly load pocketpy.dll in current directory
#elif __linux__
    dlopen("libpocketpy.so", RTLD_NOW | RTLD_GLOBAL);
#elif __APPLE__
    dlopen("libpocketpy.dylib", RTLD_NOW | RTLD_GLOBAL);
#endif
    void* vm = pkpy_new_vm();
    // pkpy::_bind(vm, vm->builtins, "input() -> str", &f_input);

    if(argc == 1){
        void* repl = pkpy_new_repl(vm);
        bool need_more_lines = false;
        while(true){
            std::cout << (need_more_lines ? "... " : ">>> ");
            bool eof = false;
            pkpy_platform_getline(buffer, 1024, &eof);
            if(eof) break;
            need_more_lines = pkpy_repl_input(repl, buffer);
        }
        pkpy_delete_vm(vm);
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

        pkpy_vm_exec_2(vm, src.c_str(), filepath.filename().string().c_str(), 0, NULL);
        pkpy_delete_vm(vm);
        // return ret != nullptr ? 0 : 1;
        return 0;
    }

__HELP:
    std::cout << "Usage: pocketpy [filename]" << std::endl;
    return 0;
}
