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
    uint16_t block;     // the block id of this bytecode
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
    std::vector<int> id;
    int parent;        // parent index in co_blocks

    int start;          // start index of this block in co_code, inclusive
    int end;            // end index of this block in co_code, exclusive

    std::string to_string() const {
        if(parent == -1) return "";
        std::string s = "[";
        for(int i = 0; i < id.size(); i++){
            s += std::to_string(id[i]);
            if(i != id.size()-1) s += "-";
        }
        s += ": type=";
        s += std::to_string(type);
        s += "]";
        return s;
    }

    bool operator==(const std::vector<int>& other) const{ return id == other; }
    bool operator!=(const std::vector<int>& other) const{ return id != other; }
    int depth() const{ return id.size(); }
};

struct CodeObject {
    _Source src;
    _Str name;

    CodeObject(_Source src, _Str name) {
        this->src = src;
        this->name = name;
    }

    std::vector<Bytecode> co_code;
    PyVarList co_consts;
    std::vector<std::pair<_Str, NameScope>> co_names;
    std::vector<_Str> co_global_names;

    std::vector<CodeBlock> co_blocks = { CodeBlock{NO_BLOCK, {}, -1} };

    // tmp variables
    int _currBlockIndex = 0;
    bool __isCurrBlockLoop() const {
        return co_blocks[_currBlockIndex].type == FOR_LOOP || co_blocks[_currBlockIndex].type == WHILE_LOOP;
    }

    void __enter_block(CodeBlockType type){
        const CodeBlock& currBlock = co_blocks[_currBlockIndex];
        std::vector<int> copy(currBlock.id);
        copy.push_back(-1);
        int t = 0;
        while(true){
            copy[copy.size()-1] = t;
            auto it = std::find(co_blocks.begin(), co_blocks.end(), copy);
            if(it == co_blocks.end()) break;
            t++;
        }
        co_blocks.push_back(CodeBlock{type, copy, _currBlockIndex, (int)co_code.size()});
        _currBlockIndex = co_blocks.size()-1;
    }

    void __exit_block(){
        co_blocks[_currBlockIndex].end = co_code.size();
        _currBlockIndex = co_blocks[_currBlockIndex].parent;
        if(_currBlockIndex < 0) UNREACHABLE();
    }

    // for goto use
    // goto/label should be put at toplevel statements
    emhash8::HashMap<_Str, int> co_labels;

    void add_label(const _Str& label){
        if(co_labels.find(label) != co_labels.end()){
            _Str msg = "label '" + label + "' already exists";
            throw std::runtime_error(msg.c_str());
        }
        co_labels[label] = co_code.size();
    }

    int add_name(_Str name, NameScope scope){
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

    int add_const(PyVar v){
        co_consts.push_back(v);
        return co_consts.size() - 1;
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

class Frame {
private:
    std::vector<PyVar> s_data;
    int ip = -1;
    int next_ip = 0;
    int m_id;
public:
    const _Code code;
    PyVar _module;
    pkpy::shared_ptr<PyVarDict> _locals;

    inline PyVarDict& f_locals() noexcept { return *_locals; }
    inline PyVarDict& f_globals() noexcept { return _module->attribs; }

    inline i64 id() const noexcept { return m_id; }

    Frame(const _Code code, PyVar _module, pkpy::shared_ptr<PyVarDict> _locals)
        : code(code), _module(_module), _locals(_locals) {
        static thread_local i64 _id = 0;
        m_id = _id++;
    }

    inline const Bytecode& next_bytecode() {
        ip = next_ip;
        next_ip = ip + 1;
        return code->co_code[ip];
    }

    _Str curr_snapshot(){
        int line = code->co_code[ip].line;
        return code->src->snapshot(line);
    }

    inline int stack_size() const{ return s_data.size(); }
    _Str stack_info(){
        _StrStream ss;
        ss << "[";
        for(int i=0; i<s_data.size(); i++){
            ss << UNION_TP_NAME(s_data[i]);
            if(i != s_data.size()-1) ss << ", ";
        }
        ss << "]";
        return ss.str();
    }

    inline bool has_next_bytecode() const{ return next_ip < code->co_code.size(); }

    inline PyVar pop(){
        if(s_data.empty()) throw std::runtime_error("s_data.empty() is true");
        PyVar v = std::move(s_data.back());
        s_data.pop_back();
        return v;
    }

    inline void __pop(){
        if(s_data.empty()) throw std::runtime_error("s_data.empty() is true");
        s_data.pop_back();
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
        if(s_data.empty()) throw std::runtime_error("s_data.empty() is true");
        return s_data.back();
    }

    inline PyVar top_value_offset(VM* vm, int n){
        PyVar value = s_data[s_data.size() + n];
        try_deref(vm, value);
        return value;
    }

    template<typename T>
    inline void push(T&& obj){ s_data.push_back(std::forward<T>(obj)); }

    inline void jump_abs(int i){ next_ip = i; }
    inline void jump_rel(int i){ next_ip += i; }

    std::stack<std::pair<int, std::vector<PyVar>>> s_try_block;

    inline void on_try_block_enter(){
        s_try_block.push(std::make_pair(code->co_code[ip].block, s_data));
    }

    inline void on_try_block_exit(){
        s_try_block.pop();
    }

    inline int get_ip() const{ return ip; }

    bool jump_to_exception_handler(){
        if(s_try_block.empty()) return false;
        PyVar obj = pop();
        auto& p = s_try_block.top();
        s_data = std::move(p.second);
        s_data.push_back(obj);
        next_ip = code->co_blocks[p.first].end;
        on_try_block_exit();
        return true;
    }

    void jump_abs_safe(int target){
        const Bytecode& prev = code->co_code[ip];
        int i = prev.block;
        next_ip = target;
        if(next_ip >= code->co_code.size()){
            while(i>=0){
                if(code->co_blocks[i].type == FOR_LOOP) pop();
                i = code->co_blocks[i].parent;
            }
        }else{
            const Bytecode& next = code->co_code[target];
            while(i>=0 && i!=next.block){
                if(code->co_blocks[i].type == FOR_LOOP) pop();
                i = code->co_blocks[i].parent;
            }
            if(i!=next.block) throw std::runtime_error("invalid jump");
        }
    }

    pkpy::Args pop_n_values_reversed(VM* vm, int n){
        int new_size = s_data.size() - n;
        if(new_size < 0) throw std::runtime_error("stack_size() < n");
        pkpy::Args v(n);
        for(int i=n-1; i>=0; i--){
            v[i] = std::move(s_data[new_size + i]);
            try_deref(vm, v[i]);
        }
        s_data.resize(new_size);
        return v;
    }

    pkpy::Args pop_n_reversed(int n){
        pkpy::Args v(n);
        for(int i=n-1; i>=0; i--) v[i] = pop();
        return v;
    }
};