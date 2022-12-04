#include <fstream>

#include "pocketpy.h"

#define PK_DEBUG_TIME
//#define PK_DEBUG_THREADED

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

#if defined(__EMSCRIPTEN__) || defined(__wasm__) || defined(__wasm32__) || defined(__wasm64__)

// these code is for demo use, feel free to modify it
REPL* _repl;

extern "C" {
    __EXPORT
    void repl_start(){
        _repl = pkpy_new_repl(pkpy_new_vm(true));
    }

    __EXPORT
    bool repl_input(const char* line){
        return pkpy_repl_input(_repl, line) == NEED_MORE_LINES;
    }
}

#else


void _tvm_dispatch(ThreadedVM* vm){
    while(pkpy_tvm_get_state(vm) != THREAD_FINISHED){
        if(pkpy_tvm_get_state(vm) == THREAD_SUSPENDED){
            char* obj = pkpy_tvm_read_jsonrpc_request(vm);
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


int main(int argc, char** argv){
    if(argc == 1){
#ifndef PK_DEBUG_THREADED
        VM* vm = pkpy_new_vm(true);
#else
        ThreadedVM* vm = pkpy_new_tvm(true);
#endif
        REPL repl(vm);
        while(true){
            (*vm->_stdout) << (repl.is_need_more_lines() ? "... " : ">>> ");
            std::string line;
            std::getline(std::cin, line);
            int result = pkpy_repl_input(&repl, line.c_str());
#ifdef PK_DEBUG_THREADED
            if(result == (int)EXEC_DONE){
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
        _Code code = nullptr;
        Timer("Compile time").run([&]{
            code = compile(vm, src.c_str(), filename);
        });
        if(code == nullptr) return 1;

        //std::cout << code->toString() << std::endl;

        // for(auto& kv : _strIntern)
        //     std::cout << kv.first << ", ";
        
#ifdef PK_DEBUG_THREADED
        Timer("Running time").run([=]{
            vm->execAsync(code);
            _tvm_dispatch(vm);
        });
#else
        Timer("Running time").run([=]{
            vm->exec(code);
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