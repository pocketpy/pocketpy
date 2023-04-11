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
    int for_loop_depth; // this is used for exception handling
    int start;          // start index of this block in codes, inclusive
    int end;            // end index of this block in codes, exclusive

    CodeBlock(CodeBlockType type, int parent, int for_loop_depth, int start):
        type(type), parent(parent), for_loop_depth(for_loop_depth), start(start), end(-1) {}
};

struct CodeObject {
    shared_ptr<SourceData> src;
    Str name;
    bool is_generator = false;

    CodeObject(shared_ptr<SourceData> src, const Str& name):
        src(src), name(name) {}

    std::vector<Bytecode> codes;
    std::vector<int> lines; // line number for each bytecode
    List consts;
    std::vector<StrName> names;         // other names
    std::vector<StrName> varnames;      // local variables
    std::map<StrName, int> varnames_inv;
    std::set<Str> global_names;
    std::vector<CodeBlock> blocks = { CodeBlock(NO_BLOCK, -1, 0, 0) };
    std::map<StrName, int> labels;
    std::vector<FuncDecl_> func_decls;

    void optimize(VM* vm);

    void _gc_mark() const {
        for(PyObject* v : consts) OBJ_MARK(v);
        for(auto& decl: func_decls) decl->_gc_mark();
    }
};

} // namespace pkpy