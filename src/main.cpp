#include <iostream>
#include <fstream>

#include "pocketpy.h"

//#define PK_DEBUG_TIME
//#define PK_DEBUG_STACK

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


#if defined(__EMSCRIPTEN__) || defined(__wasm__) || defined(__wasm32__) || defined(__wasm64__)

REPL* _repl;

extern "C" {
    __EXPORT
    void repl_start(){
        _repl = new REPL(newVM(), false);
    }

    __EXPORT
    bool repl_input(const char* line){
        return _repl->input(line);
    }
}

#else


#ifdef PK_DEBUG_STACK
#include <sys/resource.h>

void setStackSize(_Float mb){
    const rlim_t kStackSize = (_Int)(mb * 1024 * 1024);
    struct rlimit rl;
    int result;
    result = getrlimit(RLIMIT_STACK, &rl);
    rl.rlim_cur = kStackSize;
    result = setrlimit(RLIMIT_STACK, &rl);
    if (result != 0){
        std::cerr << "setrlimit returned result = " << result << std::endl;
    }
}
#endif

int main(int argc, char** argv){
#ifdef PK_DEBUG_STACK
    setStackSize(0.5);
#endif

    if(argc == 1){
        REPL repl(newVM());
        while(true){
            std::string line;
            std::getline(std::cin, line);
            repl.input(line);
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

#endif