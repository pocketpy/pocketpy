#pragma once

#include "obj.h"
#include "error.h"

namespace pkpy{

enum NameScope { NAME_LOCAL, NAME_GLOBAL };

enum Opcode {
    #define OPCODE(name) OP_##name,
    #include "opcodes.h"
    #undef OPCODE
};

inline const char* OP_NAMES[] = {
    #define OPCODE(name) #name,
    #include "opcodes.h"
    #undef OPCODE
};

struct Bytecode{
    uint16_t op;
    uint16_t block;
    int arg;
    int line;
};

enum CodeBlockType {
    NO_BLOCK,
    FOR_LOOP,
    WHILE_LOOP,
    CONTEXT_MANAGER,
    TRY_EXCEPT,
};

#define BC_NOARG       -1
#define BC_KEEPLINE     -1

struct CodeBlock {
    CodeBlockType type;
    int parent;         // parent index in blocks
    int start;          // start index of this block in codes, inclusive
    int end;            // end index of this block in codes, exclusive
};

struct CodeObject {
    shared_ptr<SourceData> src;
    Str name;
    bool is_generator = false;

    CodeObject(shared_ptr<SourceData> src, Str name) {
        this->src = src;
        this->name = name;
    }

    std::vector<Bytecode> codes;
    List consts;
    std::vector<StrName> names;
    std::set<Str> global_names;
    std::vector<CodeBlock> blocks = { CodeBlock{NO_BLOCK, -1} };
    std::map<StrName, int> labels;
    std::vector<FuncDecl_> func_decls;

    // may be.. just use a large NameDict?
    uint32_t perfect_locals_capacity = 2;
    uint32_t perfect_hash_seed = 0;

    void optimize(VM* vm);

    void _gc_mark() const {
        for(PyObject* v : consts) OBJ_MARK(v);
        for(auto& decl: func_decls) decl->_gc_mark();
    }
};

} // namespace pkpy