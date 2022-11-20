#pragma once

#include "compiler.h"
#include "vm.h"

class REPL: public PkExportedResource {
    int need_more_lines = 0;
    std::string buffer;
    CompileMode mode;
    VM* vm;

    bool use_prompt;        // whether to print >>> or ...

    bool exited = false;

    void _exit(){
        exited = true;
        exit(0);
    }

    void _loop_start(){
        mode = SINGLE_MODE;
        if(use_prompt){
            (*vm->_stdout) << (need_more_lines ? "... " : ">>> ");
        }
    }

public:
    REPL(VM* vm, bool use_prompt=true) : vm(vm), use_prompt(use_prompt) {
        (*vm->_stdout) << ("pocketpy " PK_VERSION " (" __DATE__ ", " __TIME__ ")\n");
        (*vm->_stdout) << ("https://github.com/blueloveTH/pocketpy" "\n");
        (*vm->_stdout) << ("Type \"exit()\" to exit." "\n");
        _loop_start();
    }

    bool input(const char* line){
        return input(std::string(line));
    }

    bool input(std::string line){
        if(exited) return false;
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
                goto __LOOP_CONTINUE;
            }
        }else{
            if(line == "exit()") _exit();
            if(line.empty()) goto __LOOP_CONTINUE;
        }

        try{
            _Code code = compile(vm, line.c_str(), "<stdin>", mode);
            if(code != nullptr) vm->exec(code, nullptr, true);
        }catch(NeedMoreLines& ne){
            buffer += line;
            buffer += '\n';
            need_more_lines = ne.isClassDef ? 3 : 2;
        }
__LOOP_CONTINUE:
        _loop_start();
        return need_more_lines > 0;
    }
};