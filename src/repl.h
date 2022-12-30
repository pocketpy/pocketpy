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
    InputResult lastResult = EXEC_SKIPPED;
public:
    REPL(VM* vm) : vm(vm){
        (*vm->_stdout) << ("pocketpy " PK_VERSION " (" __DATE__ ", " __TIME__ ")\n");
        (*vm->_stdout) << ("https://github.com/blueloveTH/pocketpy" "\n");
        (*vm->_stdout) << ("Type \"exit()\" to exit." "\n");
    }

    InputResult last_input_result() const {
        return lastResult;
    }

    void input(std::string line){
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
                lastResult = NEED_MORE_LINES;
                return;
            }
        }else{
            if(line == "exit()") exit(0);
            if(line.empty()) {
                lastResult = EXEC_SKIPPED;
                return;
            }
        }

        try{
            // duplicated compile to catch NeedMoreLines
            vm->compile(line, "<stdin>", SINGLE_MODE);
        }catch(NeedMoreLines& ne){
            buffer += line;
            buffer += '\n';
            need_more_lines = ne.isClassDef ? 3 : 2;
            if (need_more_lines) {
                lastResult = NEED_MORE_LINES;
            }
            return;
        }catch(...){
            // do nothing
        }

        lastResult = EXEC_STARTED;
        vm->execAsync(line, "<stdin>", SINGLE_MODE);
    }
};