#pragma once

#include "obj.h"
#include "pointer.h"

enum Opcode {
    #define OPCODE(name) OP_##name,
    #include "opcodes.h"
    #undef OPCODE
};

static const char* OP_NAMES[] = {
    #define OPCODE(name) #name,
    #include "opcodes.h"
    #undef OPCODE
};

struct ByteCode{
    uint8_t op;
    int arg;
    uint16_t line;
};

_Str pad(const _Str& s, const int n){
    return s + _Str(n - s.size(), ' ');
}

enum CompileMode {
    EXEC_MODE,
    EVAL_MODE,
    SINGLE_MODE
};

struct CodeObject {
    CompileMode mode = EXEC_MODE;

    std::vector<ByteCode> co_code;
    _Str co_filename;
    _Str co_name;

    PyVarList co_consts;
    std::vector<std::shared_ptr<NamePointer>> co_names;

    int addName(const _Str& name, NameScope scope){
        auto p = std::make_shared<NamePointer>(name, scope);
        for(int i=0; i<co_names.size(); i++){
            if(*co_names[i] == *p) return i;
        }
        co_names.push_back(p);
        return co_names.size() - 1;
    }

    int addConst(PyVar v){
        co_consts.push_back(v);
        return co_consts.size() - 1;
    }

    void __moveToEnd(int start, int end){
        auto _start = co_code.begin() + start;
        auto _end = co_code.begin() + end;
        co_code.insert(co_code.end(), _start, _end);
        for(int i=start; i<end; i++) co_code[i].op = OP_NO_OP;
    }

    _Str toString(){
        _StrStream ss;
        int prev_line = -1;
        for(int i=0; i<co_code.size(); i++){
            const ByteCode& byte = co_code[i];
            if(byte.op == OP_NO_OP) continue;
            _Str line = std::to_string(byte.line);
            if(byte.line == prev_line) line = "";
            else{
                if(prev_line != -1) ss << "\n";
                prev_line = byte.line;
            }
            ss << pad(line, 12) << " " << pad(std::to_string(i), 3);
            ss << " " << pad(OP_NAMES[byte.op], 20) << " ";
            ss << (byte.arg == -1 ? "" : std::to_string(byte.arg));
            if(i != co_code.size() - 1) ss << '\n';
        }
        _StrStream consts;
        consts << "co_consts: ";
        for(int i=0; i<co_consts.size(); i++){
            consts << co_consts[i]->getTypeName();
            if(i != co_consts.size() - 1) consts << ", ";
        }

        _StrStream names;
        names << "co_names: ";
        for(int i=0; i<co_names.size(); i++){
            names << co_names[i]->name;
            if(i != co_names.size() - 1) names << ", ";
        }
        ss << '\n' << consts.str() << '\n' << names.str() << '\n';
        for(int i=0; i<co_consts.size(); i++){
            auto fn = std::get_if<_Func>(&co_consts[i]->_native);
            if(fn) ss << '\n' << fn->code->co_name << ":\n" << fn->code->toString();
        }
        return _Str(ss);
    }
};

class Frame {
private:
    std::vector<PyVar> s_data;
    int ip = 0;
public:
    PyVarDict* f_globals;
    PyVarDict f_locals;

    const CodeObject* code;

    Frame(const CodeObject* code, PyVarDict locals, PyVarDict* globals)
        : code(code), f_locals(locals), f_globals(globals) {}

    inline const ByteCode& readCode() {
        return code->co_code[ip++];
    }

    int currentLine(){
        if(isEnd()) return -1;
        return code->co_code[ip].line;
    }

    int stackSize() const {
        return s_data.size();
    }

    inline bool isEnd() const {
        return ip >= code->co_code.size();
    }

    inline PyVar __pop(){
        PyVar v = s_data.back();
        s_data.pop_back();
        return v;
    }

    inline PyVar __deref_pointer(VM*, PyVar);

    inline PyVar popValue(VM* vm){
        return __deref_pointer(vm, __pop());
    }

    inline PyVar topValue(VM* vm){
        return __deref_pointer(vm, s_data.back());
    }

    inline PyVar topNValue(VM* vm, int n=-1){
        return __deref_pointer(vm, s_data[s_data.size() + n]);
    }

    inline void push(PyVar v){
        s_data.push_back(v);
    }

    inline void jumpTo(int i){
        this->ip = i;
    }

    PyVarList popNValuesReversed(VM* vm, int n){
        PyVarList v(n);
        for(int i=n-1; i>=0; i--) v[i] = popValue(vm);
        return v;
    }

    PyVarList __popNReversed(int n){
        PyVarList v(n);
        for(int i=n-1; i>=0; i--) v[i] = __pop();
        return v;
    }
};