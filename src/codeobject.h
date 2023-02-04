#pragma once

#include "obj.h"
#include "ref.h"
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

struct Bytecode{
    uint8_t op;
    int arg;
    int line;
    uint16_t block;
};

_Str pad(const _Str& s, const int n){
    if(s.size() >= n) return s.substr(0, n);
    return s + std::string(n - s.size(), ' ');
}

enum CodeBlockType {
    NO_BLOCK,
    FOR_LOOP,
    WHILE_LOOP,
    CONTEXT_MANAGER,
    TRY_EXCEPT,
};

struct CodeBlock {
    CodeBlockType type;
    int parent;         // parent index in blocks
    int start;          // start index of this block in co_code, inclusive
    int end;            // end index of this block in co_code, exclusive

    std::string to_string() const {
        if(parent == -1) return "";
        return "[B:" + std::to_string(type) + "]";
    }
};

struct CodeObject {
    pkpy::shared_ptr<SourceData> src;
    _Str name;

    CodeObject(pkpy::shared_ptr<SourceData> src, _Str name) {
        this->src = src;
        this->name = name;
    }

    std::vector<Bytecode> co_code;
    PyVarList consts;
    std::vector<std::pair<_Str, NameScope>> names;
    emhash8::HashMap<_Str, int> global_names;
    std::vector<CodeBlock> blocks = { CodeBlock{NO_BLOCK, -1} };
    emhash8::HashMap<_Str, int> labels;

    // tmp variables
    int _curr_block_i = 0;
    bool __is_curr_block_loop() const {
        return blocks[_curr_block_i].type == FOR_LOOP || blocks[_curr_block_i].type == WHILE_LOOP;
    }

    void __enter_block(CodeBlockType type){
        const CodeBlock& currBlock = blocks[_curr_block_i];
        blocks.push_back(CodeBlock{type, _curr_block_i, (int)co_code.size()});
        _curr_block_i = blocks.size()-1;
    }

    void __exit_block(){
        blocks[_curr_block_i].end = co_code.size();
        _curr_block_i = blocks[_curr_block_i].parent;
        if(_curr_block_i < 0) UNREACHABLE();
    }

    bool add_label(const _Str& label){
        if(labels.contains(label)) return false;
        labels[label] = co_code.size();
        return true;
    }

    int add_name(_Str name, NameScope scope){
        if(scope == NAME_LOCAL && global_names.contains(name)) scope = NAME_GLOBAL;
        auto p = std::make_pair(name, scope);
        for(int i=0; i<names.size(); i++){
            if(names[i] == p) return i;
        }
        names.push_back(p);
        return names.size() - 1;
    }

    int add_const(PyVar v){
        consts.push_back(v);
        return consts.size() - 1;
    }

    void optimize_level_1(){
        for(int i=0; i<co_code.size(); i++){
            if(co_code[i].op >= OP_BINARY_OP && co_code[i].op <= OP_CONTAINS_OP){
                for(int j=0; j<2; j++){
                    Bytecode& bc = co_code[i-j-1];
                    if(bc.op >= OP_LOAD_CONST && bc.op <= OP_LOAD_NAME_REF){
                        if(bc.op == OP_LOAD_NAME_REF){
                            bc.op = OP_LOAD_NAME;
                        }
                    }else{
                        break;
                    }
                }
            }else if(co_code[i].op == OP_CALL){
                int ARGC = co_code[i].arg & 0xFFFF;
                int KWARGC = (co_code[i].arg >> 16) & 0xFFFF;
                if(KWARGC != 0) continue;
                for(int j=0; j<ARGC+1; j++){
                    Bytecode& bc = co_code[i-j-1];
                    if(bc.op >= OP_LOAD_CONST && bc.op <= OP_LOAD_NAME_REF){
                        if(bc.op == OP_LOAD_NAME_REF){
                            bc.op = OP_LOAD_NAME;
                        }
                    }else{
                        break;
                    }
                }
            }
        }
    }

    void optimize(int level=1){
        optimize_level_1();
    }
};

struct Frame {
    std::vector<PyVar> _data;
    int _ip = -1;
    int _next_ip = 0;

    const _Code co;
    PyVar _module;
    pkpy::shared_ptr<PyVarDict> _locals;
    i64 _id;

    inline PyVarDict& f_locals() noexcept { return *_locals; }
    inline PyVarDict& f_globals() noexcept { return _module->attribs; }

    Frame(const _Code co, PyVar _module, pkpy::shared_ptr<PyVarDict> _locals)
        : co(co), _module(_module), _locals(_locals) {
        static thread_local i64 kGlobalId = 0;
        _id = kGlobalId++;
    }

    inline const Bytecode& next_bytecode() {
        _ip = _next_ip;
        _next_ip = _ip + 1;
        return co->co_code[_ip];
    }

    _Str curr_snapshot(){
        int line = co->co_code[_ip].line;
        return co->src->snapshot(line);
    }

    _Str stack_info(){
        _StrStream ss;
        ss << "[";
        for(int i=0; i<_data.size(); i++){
            ss << UNION_TP_NAME(_data[i]);
            if(i != _data.size()-1) ss << ", ";
        }
        ss << "]";
        return ss.str();
    }

    inline bool has_next_bytecode() const {
        return _next_ip < co->co_code.size();
    }

    inline PyVar pop(){
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
        PyVar v = std::move(_data.back());
        _data.pop_back();
        return v;
    }

    inline void __pop(){
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
        _data.pop_back();
    }

    inline void try_deref(VM*, PyVar&);

    inline PyVar pop_value(VM* vm){
        PyVar value = pop();
        try_deref(vm, value);
        return value;
    }

    inline PyVar top_value(VM* vm){
        PyVar value = top();
        try_deref(vm, value);
        return value;
    }

    inline PyVar& top(){
        if(_data.empty()) throw std::runtime_error("_data.empty() is true");
        return _data.back();
    }

    inline PyVar top_value_offset(VM* vm, int n){
        PyVar value = _data[_data.size() + n];
        try_deref(vm, value);
        return value;
    }

    template<typename T>
    inline void push(T&& obj){ _data.push_back(std::forward<T>(obj)); }

    inline void jump_abs(int i){ _next_ip = i; }
    inline void jump_rel(int i){ _next_ip += i; }

    std::stack<std::pair<int, std::vector<PyVar>>> s_try_block;

    inline void on_try_block_enter(){
        s_try_block.push(std::make_pair(co->co_code[_ip].block, _data));
    }

    inline void on_try_block_exit(){
        s_try_block.pop();
    }

    bool jump_to_exception_handler(){
        if(s_try_block.empty()) return false;
        PyVar obj = pop();
        auto& p = s_try_block.top();
        _data = std::move(p.second);
        _data.push_back(obj);
        _next_ip = co->blocks[p.first].end;
        on_try_block_exit();
        return true;
    }

    void jump_abs_safe(int target){
        const Bytecode& prev = co->co_code[_ip];
        int i = prev.block;
        _next_ip = target;
        if(_next_ip >= co->co_code.size()){
            while(i>=0){
                if(co->blocks[i].type == FOR_LOOP) pop();
                i = co->blocks[i].parent;
            }
        }else{
            const Bytecode& next = co->co_code[target];
            while(i>=0 && i!=next.block){
                if(co->blocks[i].type == FOR_LOOP) pop();
                i = co->blocks[i].parent;
            }
            if(i!=next.block) throw std::runtime_error("invalid jump");
        }
    }

    pkpy::Args pop_n_values_reversed(VM* vm, int n){
        pkpy::Args v(n);
        for(int i=n-1; i>=0; i--){
            v[i] = pop();
            try_deref(vm, v[i]);
        }
        return v;
    }

    pkpy::Args pop_n_reversed(int n){
        pkpy::Args v(n);
        for(int i=n-1; i>=0; i--) v[i] = pop();
        return v;
    }
};