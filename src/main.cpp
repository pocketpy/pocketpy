#include <iostream>
#include <fstream>

#include <chrono>
#include "pocketpy.h"

//#define PK_DEBUG_TIME

struct Timer{
    const char* title;
    Timer(const char* title) : title(title) {}
    void run(std::function<void()> f){
        auto start = std::chrono::high_resolution_clock::now();
        f();
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0;
#ifdef PK_DEBUG_TIME
        std::cout << title << ": " << elapsed << " s" << std::endl;
#endif
    }
};

VM* newVM(){
    VM* vm = createVM([](const char* str) { 
        std::cout << str;
        std::cout.flush();
    }, [](const char* str) { 
        std::cerr << str;
        std::cerr.flush();
    });
    return vm;
}

void REPL(){
    std::cout << "pocketpy 0.1.0" << std::endl;
    std::cout << "https://github.com/blueloveTH/pocketpy" << std::endl;

    int need_more_lines = 0;
    std::string buffer;
    VM* vm = newVM();

    while(true){
        CompileMode mode = SINGLE_MODE;
        vm->_stdout(need_more_lines ? "... " : ">>> ");
        std::string line;
        std::getline(std::cin, line);

        if(need_more_lines){
            buffer += line;
            buffer += '\n';
            int n = buffer.size();
            if(n>=need_more_lines){
                for(int i=buffer.size()-need_more_lines; i<buffer.size(); i++){
                    if(buffer[i] != '\n') goto __NOT_ENOUGH_LINES;
                }
                need_more_lines = 0;
                line = buffer;
                mode = EXEC_MODE;       // tmp set to EXEC_MODE
                buffer.clear();
            }else{
__NOT_ENOUGH_LINES:
                continue;
            }
        }else{
            if(line == "exit()") break;
            if(line.empty()) continue;
        }

        try{
            _Code code = compile(vm, line.c_str(), "<stdin>", mode);
            if(code != nullptr) vm->exec(code);
        }catch(NeedMoreLines& ne){
            buffer += line;
            buffer += '\n';
            need_more_lines = ne.isClassDef ? 3 : 2;
        }
    }
}

int main(int argc, char** argv){
    if(argc == 1){
        REPL();
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

        VM* vm = newVM();
        _Code code;
        Timer("Compile time").run([&]{
            code = compile(vm, src.c_str(), filename);
        });
        if(code == nullptr) return 1;
        //std::cout << code->toString() << std::endl;
        Timer("Running time").run([=]{
            vm->exec(code);
        });
        return 0;
    }

__HELP:
    std::cout << "Usage: pocketpy [filename]" << std::endl;
    return 0;
}