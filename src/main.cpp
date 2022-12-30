#include <fstream>
#include <functional>

#include "pocketpy.h"

#define PK_DEBUG_TIME
//#define PK_DEBUG_THREADED

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

void _tvm_dispatch(ThreadedVM* vm){
    while(pkpy_tvm_get_state(vm) != THREAD_FINISHED){
        if(pkpy_tvm_get_state(vm) == THREAD_SUSPENDED){
            char* obj = pkpy_tvm_read_jsonrpc_request(vm);
            // this is not safe, but it's ok for demo
            bool is_input_call = std::string_view(obj).find("\"input\"") != std::string::npos;
            if(is_input_call){
                std::string line;
                std::getline(std::cin, line);
                _StrStream ss;
                ss << '{';
                ss << "\"result\": " << _Str(line).__escape(false);
                ss << '}';
                pkpy_tvm_write_jsonrpc_response(vm, ss.str().c_str());
            }else{
                std::cout << "unknown jsonrpc call" << std::endl;
                std::cout << obj << std::endl;
                exit(3);
            }
            pkpy_delete(obj);
        }
    }
}

#ifndef __NO_MAIN

int main(int argc, char** argv){
    if(argc == 1){
#ifndef PK_DEBUG_THREADED
        VM* vm = pkpy_new_vm(true);
#else
        ThreadedVM* vm = pkpy_new_tvm(true);
#endif
        REPL repl(vm);
        int result = -1;
        while(true){
            (*vm->_stdout) << (result==0 ? "... " : ">>> ");
            std::string line;
            std::getline(std::cin, line);
            pkpy_repl_input(&repl, line.c_str());
            result = pkpy_repl_last_input_result(&repl);
#ifdef PK_DEBUG_THREADED
            if(result == (int)EXEC_STARTED){
                _tvm_dispatch(vm);
                pkpy_tvm_reset_state(vm);
            }
#endif
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

        ThreadedVM* vm = pkpy_new_tvm(true);
        //std::cout << code->toString() << std::endl;
#ifdef PK_DEBUG_THREADED
        Timer("Running time").run([=]{
            vm->execAsync(src.c_str(), filename, EXEC_MODE);
            _tvm_dispatch(vm);
        });
#else
        Timer("Running time").run([=]{
            vm->exec(src.c_str(), filename, EXEC_MODE);
        });
#endif

        pkpy_delete(vm);
        return 0;
    }

__HELP:
    std::cout << "Usage: pocketpy [filename]" << std::endl;
    return 0;
}

#endif