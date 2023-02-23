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

Str pad(const Str& s, const int n){
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
    int start;          // start index of this block in codes, inclusive
    int end;            // end index of this block in codes, exclusive

    std::string to_string() const {
        if(parent == -1) return "";
        return "[B:" + std::to_string(type) + "]";
    }
};

struct CodeObject {
    pkpy::shared_ptr<SourceData> src;
    Str name;
    bool is_generator = false;

    CodeObject(pkpy::shared_ptr<SourceData> src, Str name) {
        this->src = src;
        this->name = name;
    }

    std::vector<Bytecode> codes;
    pkpy::List consts;
    std::vector<std::pair<StrName, NameScope>> names;
    std::map<StrName, int> global_names;
    std::vector<CodeBlock> blocks = { CodeBlock{NO_BLOCK, -1} };
    std::map<StrName, int> labels;

    uint32_t perfect_locals_capacity = 2;
    uint32_t perfect_hash_seed = 0;

    void optimize(VM* vm);

    bool add_label(StrName label){
        if(labels.count(label)) return false;
        labels[label] = codes.size();
        return true;
    }

    int add_name(StrName name, NameScope scope){
        if(scope == NAME_LOCAL && global_names.count(name)) scope = NAME_GLOBAL;
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

    /************************************************/
    int _curr_block_i = 0;
    int _rvalue = 0;
    bool _is_curr_block_loop() const {
        return blocks[_curr_block_i].type == FOR_LOOP || blocks[_curr_block_i].type == WHILE_LOOP;
    }

    void _enter_block(CodeBlockType type){
        blocks.push_back(CodeBlock{type, _curr_block_i, (int)codes.size()});
        _curr_block_i = blocks.size()-1;
    }

    void _exit_block(){
        blocks[_curr_block_i].end = codes.size();
        _curr_block_i = blocks[_curr_block_i].parent;
        if(_curr_block_i < 0) UNREACHABLE();
    }
    /************************************************/
};
