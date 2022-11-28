#pragma once

#include "compiler.h"
#include "vm.h"

class REPL: public PkExportedResource {
protected:
    int need_more_lines = 0;
    std::string buffer;
    CompileMode mode;
    VM* vm;
    bool exited = false;

    void _exit(){
        exited = true;
        exit(0);
    }
public:
    REPL(VM* vm) : vm(vm){
        (*vm->_stdout) << ("pocketpy " PK_VERSION " (" __DATE__ ", " __TIME__ ")\n");
        (*vm->_stdout) << ("https://github.com/blueloveTH/pocketpy" "\n");
        (*vm->_stdout) << ("Type \"exit()\" to exit." "\n");
    }

    VM* getVM() { return vm; }

    bool is_need_more_lines() const {
        return need_more_lines;
    }

    bool input(const char* line){
        return input(std::string(line));
    }

    bool input(std::string line){
        if(exited) return false;
        mode = SINGLE_MODE;
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
            this->onCompiled(code);
        }catch(NeedMoreLines& ne){
            buffer += line;
            buffer += '\n';
            need_more_lines = ne.isClassDef ? 3 : 2;
        }
__LOOP_CONTINUE:
        return is_need_more_lines();
    }

    _Code readBufferCode(){
        auto copy = std::move(bufferCode);
        bufferCode = nullptr;
        return copy;
    }

protected:
    _Code bufferCode = nullptr;

    void onCompiled(_Code code){
        if(code == nullptr){
            bufferCode = nullptr;
        }else{
            bufferCode = std::move(code);
        }
    }
};