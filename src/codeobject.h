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
    uint16_t block;     // the block id of this bytecode
};

_Str pad(const _Str& s, const int n){
    return s + std::string(n - s.size(), ' ');
}

enum CodeBlockType {
    NO_BLOCK,
    FOR_LOOP,
    CONTEXT_MANAGER,
    TRY_EXCEPT,
};

struct CodeBlock {
    CodeBlockType type;
    std::vector<int> id;
    int parent;        // parent index in co_blocks

    std::string toString() const {
        if(parent == -1) return "";
        std::string s = "[";
        for(int i = 0; i < id.size(); i++){
            s += std::to_string(id[i]);
            if(i != id.size()-1) s += "-";
        }
        s += ": ";
        s += std::to_string(type);
        s += "]";
        return s;
    }

    bool operator==(const std::vector<int>& other) const {
        return id == other;
    }

    bool operator!=(const std::vector<int>& other) const {
        return id != other;
    }

    int depth() const {
        return id.size();
    }
};

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

    std::vector<CodeBlock> co_blocks = { CodeBlock{NO_BLOCK, {}, -1} };

    // tmp variables
    int _currBlockIndex = 0;

    void __enterBlock(CodeBlockType type){
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
        co_blocks.push_back(CodeBlock{type, copy, _currBlockIndex});
        _currBlockIndex = co_blocks.size()-1;
    }

    void __exitBlock(){
        _currBlockIndex = co_blocks[_currBlockIndex].parent;
        if(_currBlockIndex < 0) UNREACHABLE();
    }

    // for goto use
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
public:
    const _Code code;
    PyVar _module;
    PyVarDict f_locals;

    inline PyVarDict copy_f_locals() const {
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

    inline void jumpAbsolute(int i){
        this->ip = i;
    }

    inline void jumpRelative(int i){
        this->ip += i;
    }

    void jumpAbsoluteSafe(int target){
        const ByteCode& prev = code->co_code[this->ip];
        int i = prev.block;
        this->ip = target;
        if(isCodeEnd()){
            while(i>=0){
                if(code->co_blocks[i].type == FOR_LOOP) __pop();
                i = code->co_blocks[i].parent;
            }
        }else{
            const ByteCode& next = code->co_code[target];
            while(i>=0 && i!=next.block){
                if(code->co_blocks[i].type == FOR_LOOP) __pop();
                i = code->co_blocks[i].parent;
            }
            if(i!=next.block) throw std::runtime_error(
                "invalid jump from " + code->co_blocks[prev.block].toString() + " to " + code->co_blocks[next.block].toString()
            );
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