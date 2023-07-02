#pragma once

#include "obj.h"
#include "error.h"

namespace pkpy{

enum NameScope { NAME_LOCAL, NAME_GLOBAL, NAME_GLOBAL_UNKNOWN };

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

inline const int BC_NOARG = -1;
inline const int BC_KEEPLINE = -1;

struct CodeBlock {
    CodeBlockType type;
    int parent;         // parent index in blocks
    int for_loop_depth; // this is used for exception handling
    int start;          // start index of this block in codes, inclusive
    int end;            // end index of this block in codes, exclusive

    CodeBlock(CodeBlockType type, int parent, int for_loop_depth, int start):
        type(type), parent(parent), for_loop_depth(for_loop_depth), start(start), end(-1) {}
};


struct CodeObjectSerializer{
    std::string buffer;
    int depth = 0;

    std::set<StrName> names;

    static const char END = '\n';

    CodeObjectSerializer();

    void write_int(i64 v);
    void write_float(f64 v);
    void write_str(const Str& v);
    void write_none();
    void write_ellipsis();
    void write_bool(bool v);
    void write_begin_mark();
    void write_name(StrName name);
    void write_end_mark();

    template<typename T>
    void write_bytes(T v){
        static_assert(std::is_trivially_copyable<T>::value);
        buffer += 'x';
        char* p = (char*)&v;
        for(int i=0; i<sizeof(T); i++){
            char c = p[i];
            buffer += "0123456789abcdef"[(c >> 4) & 0xf];
            buffer += "0123456789abcdef"[c & 0xf];
        }
        buffer += END;
    }

    void write_object(VM* vm, PyObject* obj);
    void write_code(VM* vm, const CodeObject* co);
    std::string str();
};


struct CodeObject {
    shared_ptr<SourceData> src;
    Str name;
    bool is_generator = false;

    std::vector<Bytecode> codes;
    std::vector<int> lines; // line number for each bytecode
    List consts;
    std::vector<StrName> varnames;      // local variables
    NameDictInt varnames_inv;
    std::vector<CodeBlock> blocks = { CodeBlock(NO_BLOCK, -1, 0, 0) };
    NameDictInt labels;
    std::vector<FuncDecl_> func_decls;

    CodeObject(shared_ptr<SourceData> src, const Str& name);
    void _gc_mark() const;
    void write(VM* vm, CodeObjectSerializer& ss) const;
    Str serialize(VM* vm) const;
};

} // namespace pkpy