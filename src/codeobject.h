#pragma once

#include "obj.h"
#include "pointer.h"
#include "error.h"

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
    return s + std::string(n - s.size(), ' ');
}

struct CodeObject {
    _Source src;
    _Str name;

    CodeObject(_Source src, _Str name) {
        this->src = src;
        this->name = name;
    }

    CompileMode mode() const {
        return src->mode;
    }

    std::vector<ByteCode> co_code;
    PyVarList co_consts;
    std::vector<std::pair<_Str, NameScope>> co_names;
    std::vector<_Str> co_global_names;

    // for goto use
    // note: some opcodes moves the bytecode, such as listcomp
    // goto/label should be put at toplevel statements
    emhash8::HashMap<_Str, int> co_labels;

    void addLabel(const _Str& label){
        if(co_labels.find(label) != co_labels.end()){
            _Str msg = "label '" + label + "' already exists";
            throw std::runtime_error(msg.c_str());
        }
        co_labels[label] = co_code.size();
    }

    int addName(_Str name, NameScope scope){
        if(scope == NAME_LOCAL && std::find(co_global_names.begin(), co_global_names.end(), name) != co_global_names.end()){
            scope = NAME_GLOBAL;
        }
        auto p = std::make_pair(name, scope);
        for(int i=0; i<co_names.size(); i++){
            if(co_names[i] == p) return i;
        }
        co_names.push_back(p);
        return co_names.size() - 1;
    }

    int addConst(PyVar v){
        co_consts.push_back(v);
        return co_consts.size() - 1;
    }
};

class Frame {
private:
    std::vector<PyVar> s_data;
    int ip = 0;
    std::stack<int> forLoops;       // record the FOR_ITER bytecode index
public:
    const _Code code;
    PyVar _module;
    PyVarDict f_locals;

    inline PyVarDict copy_f_locals(){
        return f_locals;
    }

    inline PyVarDict& f_globals(){
        return _module->attribs;
    }

    Frame(const _Code code, PyVar _module, PyVarDict&& locals)
        : code(code), _module(_module), f_locals(std::move(locals)) {
    }

    inline const ByteCode& readCode() {
        return code->co_code[ip++];
    }

    _Str errorSnapshot(){
        int line = code->co_code[ip-1].line;
        return code->src->snapshot(line);
    }

    inline int stackSize() const {
        return s_data.size();
    }

    inline bool isCodeEnd() const {
        return ip >= code->co_code.size();
    }

    inline PyVar __pop(){
        if(s_data.empty()) throw std::runtime_error("s_data.empty() is true");
        PyVar v = std::move(s_data.back());
        s_data.pop_back();
        return v;
    }

    inline PyVar __deref_pointer(VM*, PyVar);

    inline PyVar popValue(VM* vm){
        return __deref_pointer(vm, __pop());
    }

    inline PyVar topValue(VM* vm){
        if(s_data.empty()) throw std::runtime_error("s_data.empty() is true");
        return __deref_pointer(vm, s_data.back());
    }

    inline PyVar& __top(){
        if(s_data.empty()) throw std::runtime_error("s_data.empty() is true");
        return s_data.back();
    }

    inline PyVar __topValueN(VM* vm, int n=-1){
        return __deref_pointer(vm, s_data[s_data.size() + n]);
    }

    template<typename T>
    inline void push(T&& obj){
        s_data.push_back(std::forward<T>(obj));
    }


    void __reportForIter(){
        int lastIp = ip - 1;
        if(forLoops.empty()) forLoops.push(lastIp);
        else{
            if(forLoops.top() == lastIp) return;
            if(forLoops.top() < lastIp) forLoops.push(lastIp);
            else UNREACHABLE();
        }
    }

    inline void jumpAbsolute(int i){
        this->ip = i;
    }

    inline void jumpRelative(int i){
        this->ip += i;
    }

    void __safeJumpClean(){
        while(!forLoops.empty()){
            int start = forLoops.top();
            int end = code->co_code[start].arg;
            if(ip < start || ip >= end){
                //printf("%d <- [%d, %d)\n", i, start, end);
                __pop();    // pop the iterator
                forLoops.pop();
            }else{
                break;
            }
        }
    }

    pkpy::ArgList popNValuesReversed(VM* vm, int n){
        pkpy::ArgList v(n);
        for(int i=n-1; i>=0; i--) v._index(i) = popValue(vm);
        return v;
    }

    PyVarList popNValuesReversedUnlimited(VM* vm, int n){
        PyVarList v(n);
        for(int i=n-1; i>=0; i--) v[i] = popValue(vm);
        return v;
    }

    pkpy::ArgList __popNReversed(int n){
        pkpy::ArgList v(n);
        for(int i=n-1; i>=0; i--) v._index(i) = __pop();
        return v;
    }
};