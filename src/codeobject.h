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

#define BC_NOARG        -1
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


struct CodeObjectSerializer{
    std::string buffer;
    int depth = 0;

    std::set<StrName> names;

    static const char END = '\n';

    CodeObjectSerializer(){
        write_str(PK_VERSION);
    }

    void write_int(i64 v){
        buffer += 'i';
        buffer += std::to_string(v);
        buffer += END;
    }

    void write_float(f64 v){
        buffer += 'f';
        buffer += std::to_string(v);
        buffer += END;
    }

    void write_str(const Str& v){
        buffer += 's';
        buffer += v.escape(false).str();
        buffer += END;
    }

    void write_none(){
        buffer += 'N';
        buffer += END;
    }

    void write_ellipsis(){
        buffer += 'E';
        buffer += END;
    }

    void write_bool(bool v){
        buffer += 'b';
        buffer += v ? '1' : '0';
        buffer += END;
    }

    void write_begin_mark(){
        buffer += '[';
        buffer += END;
        depth++;
    }

    void write_name(StrName name){
        PK_ASSERT(StrName::is_valid(name.index));
        buffer += 'n';
        buffer += std::to_string(name.index);
        buffer += END;
        names.insert(name);
    }

    void write_end_mark(){
        buffer += ']';
        buffer += END;
        depth--;
        PK_ASSERT(depth >= 0);
    }

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

    std::string str(){
        PK_ASSERT(depth == 0);
        for(auto name: names){
            PK_ASSERT(StrName::is_valid(name.index));
            write_name(name);
            write_str(name.sv());
        }
        return std::move(buffer);
    }
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
    std::vector<StrName> varnames;      // local variables
    NameDictInt varnames_inv;
    std::vector<CodeBlock> blocks = { CodeBlock(NO_BLOCK, -1, 0, 0) };
    NameDictInt labels;
    std::vector<FuncDecl_> func_decls;

    void _gc_mark() const {
        for(PyObject* v : consts) OBJ_MARK(v);
        for(auto& decl: func_decls) decl->_gc_mark();
    }

    void write(VM* vm, CodeObjectSerializer& ss) const{
        ss.write_begin_mark();          // [
        ss.write_str(src->filename);    // src->filename
        ss.write_int(src->mode);        // src->mode
        ss.write_end_mark();            // ]
        ss.write_str(name);             // name
        ss.write_bool(is_generator);    // is_generator
        ss.write_begin_mark();          // [
            for(Bytecode bc: codes){
                if(StrName::is_valid(bc.arg)) ss.names.insert(StrName(bc.arg));
                ss.write_bytes(bc);
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(int line: lines){
                ss.write_int(line);         // line
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(PyObject* o: consts){
                ss.write_object(vm, o);
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(StrName vn: varnames){
                ss.write_name(vn);        // name
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(CodeBlock block: blocks){
                ss.write_bytes(block);      // block
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(auto& label: labels.items()){
                ss.write_name(label.first);     // label.first
                ss.write_int(label.second);     // label.second
            }
        ss.write_end_mark();            // ]
        ss.write_begin_mark();          // [
            for(auto& decl: func_decls){
                ss.write_code(vm, decl->code.get()); // decl->code
                ss.write_begin_mark();      // [
                    for(int arg: decl->args) ss.write_int(arg);
                ss.write_end_mark();        // ]
                
                ss.write_begin_mark();      // [
                    for(auto kw: decl->kwargs){
                        ss.write_int(kw.key);           // kw.key
                        ss.write_object(vm, kw.value);  // kw.value
                    }
                ss.write_end_mark();        // ]

                ss.write_int(decl->starred_arg);
                ss.write_int(decl->starred_kwarg);
                ss.write_bool(decl->nested);
            }
        ss.write_end_mark();            // ]
    }

    Str serialize(VM* vm) const{
        CodeObjectSerializer ss;
        ss.write_code(vm, this);
        return ss.str();
    }
};

inline void CodeObjectSerializer::write_code(VM* vm, const CodeObject* co){
    buffer += '(';
    buffer += END;
    co->write(vm, *this);
    buffer += ')';
    buffer += END;
}

} // namespace pkpy