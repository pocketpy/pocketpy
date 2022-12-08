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
        name.intern();
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

    void __moveToEnd(int start, int end){
        auto _start = co_code.begin() + start;
        auto _end = co_code.begin() + end;
        co_code.insert(co_code.end(), _start, _end);
        for(int i=start; i<end; i++) co_code[i].op = OP_DELETED_OP;
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
            consts << UNION_TP_NAME(co_consts[i]);
            if(i != co_consts.size() - 1) consts << ", ";
        }

        _StrStream names;
        names << "co_names: ";
        for(int i=0; i<co_names.size(); i++){
            names << co_names[i].first;
            if(i != co_names.size() - 1) names << ", ";
        }
        ss << '\n' << consts.str() << '\n' << names.str() << '\n';
        // for(int i=0; i<co_consts.size(); i++){
        //     auto fn = std::get_if<_Func>(&co_consts[i]->_native);
        //     if(fn) ss << '\n' << (*fn)->code->name << ":\n" << (*fn)->code->toString();
        // }
        return _Str(ss.str());
    }
};

class Frame {
private:
    std::vector<PyVar> s_data;
    int ip = 0;
    std::stack<int> forLoops;       // record the FOR_ITER bytecode index
public:
    const CodeObject* code;
    PyVar _module;
    PyVarDict f_locals;

    uint64_t id;

    inline PyVarDict copy_f_locals(){
        return f_locals;
    }

    inline PyVarDict& f_globals(){
        return _module->attribs;
    }

    Frame(const CodeObject* code, PyVar _module, PyVarDict&& locals)
        : code(code), _module(_module), f_locals(locals) {
        
        static uint64_t frame_id = 1;
        id = frame_id++;
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

    inline void push(const PyVar& v){
        s_data.push_back(v);
    }

    inline void push(PyVar&& v){
        s_data.emplace_back(std::move(v));
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

    inline void jump(int i){
        this->ip = i;
    }

    void safeJump(int i){
        this->ip = i;
        while(!forLoops.empty()){
            int start = forLoops.top();
            int end = code->co_code[start].arg;
            if(i < start || i >= end){
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

    pkpy::ArgList __popNReversed(int n){
        pkpy::ArgList v(n);
        for(int i=n-1; i>=0; i--) v._index(i) = __pop();
        return v;
    }
};