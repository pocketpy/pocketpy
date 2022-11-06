#include <iostream>
#include <fstream>

#include <chrono>
#include "pocketpy.h"

//#define PK_DEBUG
//#define PK_DEBUG_TIME

class Timer{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::string title;
public:
    Timer(const std::string& title){
#ifdef PK_DEBUG_TIME
        start = std::chrono::high_resolution_clock::now();
        this->title = title;
#endif
    }

    void stop(){
#ifdef PK_DEBUG_TIME
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0;
        std::cout << title << ": " << elapsed << " s" << std::endl;
#endif
    }
};

VM* newVM(){
    VM* vm = createVM([](const char* str) { 
        std::cout << str;
        std::cout.flush();
    });
    registerModule(vm, "math", "pi = 3.141593");
    return vm;
}

void REPL(){
    std::cout << "pocketpy 0.1.0" << std::endl;

    bool need_more_lines = false;
    std::string buffer;
    VM* vm = newVM();

    while(true){
        vm->printFn(need_more_lines ? "... " : ">>> ");
        std::string line;
        std::getline(std::cin, line);

        if(need_more_lines){
            buffer += line;
            buffer += '\n';
            int n = buffer.size();
            if(n>=2 && buffer[n-1]=='\n' && buffer[n-2]=='\n'){
                need_more_lines = false;
                line = buffer;
                buffer.clear();
            }else{
                continue;
            }
        }else{
            if(line == "exit()") break;
            if(line.empty()) continue;
        }
        try{
            _Code code = compile(vm, line.c_str(), "<stdin>", true);
            vm->exec(code);
#ifdef PK_DEBUG
        }catch(NeedMoreLines& e){
#else
        }catch(std::exception& e){
#endif
            if(need_more_lines = dynamic_cast<NeedMoreLines*>(&e)){
                buffer += line;
                buffer += '\n';
            }else{
                vm->printFn(e.what());
                vm->printFn("\n");
                vm->cleanError();
            }
        }
    }
}

int main(int argc, char** argv){
    if(argc == 1){
        REPL();
        return 0;

        // argc = 2;
        // argv = new char*[2]{argv[0], (char*)"../tests/singletype/basic.py"};
    }
    
    if(argc == 2){
        std::string filename = argv[1];
        if(filename == "-h" || filename == "--help"){
            std::cout << "Usage: pocketpy [filename]" << std::endl;
            return 0;
        }
#ifndef PK_DEBUG
        try{
#endif
            std::ifstream file(filename);
            std::string src((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            VM* vm = newVM();
            Timer timer("编译时间");
            _Code code = compile(vm, src.c_str(), filename, false);
            timer.stop();
            //std::cout << code->toString() << std::endl;
            Timer timer2("运行时间");
            vm->exec(code);
            timer2.stop();
#ifndef PK_DEBUG
        }catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
#endif
        return 0;
    }
}