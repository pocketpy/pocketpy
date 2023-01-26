#pragma once

#include "compiler.h"
#include "vm.h"

enum InputResult {
    NEED_MORE_LINES = 0,
    EXEC_STARTED = 1,
    EXEC_SKIPPED = 2,
};

class REPL {
protected:
    int need_more_lines = 0;
    std::string buffer;
    VM* vm;
public:
    REPL(VM* vm) : vm(vm){
        (*vm->_stdout) << ("pocketpy " PK_VERSION " (" __DATE__ ", " __TIME__ ")\n");
        (*vm->_stdout) << ("https://github.com/blueloveTH/pocketpy" "\n");
        (*vm->_stdout) << ("Type \"exit()\" to exit." "\n");
    }

    InputResult input(std::string line){
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
                buffer.clear();
            }else{
__NOT_ENOUGH_LINES:
                return NEED_MORE_LINES;
            }
        }else{
            if(line.empty()) return EXEC_SKIPPED;
        }

        try{
            vm->compile(line, "<stdin>", SINGLE_MODE);
        }catch(NeedMoreLines& ne){
            buffer += line;
            buffer += '\n';
            need_more_lines = ne.isClassDef ? 3 : 2;
            if (need_more_lines) return NEED_MORE_LINES;
        }catch(...){
            // do nothing
        }
        vm->exec(line, "<stdin>", SINGLE_MODE);
        return EXEC_STARTED;
    }
};