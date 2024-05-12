#pragma once

#include "obj.h"
#include "error.h"
#include "any.h"

namespace pkpy{

enum NameScope { NAME_LOCAL, NAME_GLOBAL, NAME_GLOBAL_UNKNOWN };

enum Opcode: uint8_t {
    #define OPCODE(name) OP_##name,
    #include "opcodes.h"
    #undef OPCODE
};

struct Bytecode{
    uint8_t op;
    uint16_t arg;
};

enum class CodeBlockType {
    NO_BLOCK,
    FOR_LOOP,
    WHILE_LOOP,
    CONTEXT_MANAGER,
    TRY_EXCEPT,
};

inline const uint8_t BC_NOARG = 0;
inline const int BC_KEEPLINE = -1;

struct CodeBlock {
    CodeBlockType type;
    int parent;         // parent index in blocks
    int base_stack_size; // this is used for exception handling
    int start;          // start index of this block in codes, inclusive
    int end;            // end index of this block in codes, exclusive
    int end2;           // ...

    CodeBlock(CodeBlockType type, int parent, int base_stack_size, int start):
        type(type), parent(parent), base_stack_size(base_stack_size), start(start), end(-1), end2(-1) {}

    int get_break_end() const{
        if(end2 != -1) return end2;
        return end;
    }
};

struct CodeObject;
struct FuncDecl;
using CodeObject_ = std::shared_ptr<CodeObject>;
using FuncDecl_ = std::shared_ptr<FuncDecl>;

struct CodeObject {
    struct LineInfo{
        int lineno;             // line number for each bytecode
        bool is_virtual;        // whether this bytecode is virtual (not in source code)
    };

    std::shared_ptr<SourceData> src;
    Str name;

    std::vector<Bytecode> codes;
    std::vector<int> iblocks;       // block index for each bytecode
    std::vector<LineInfo> lines;
    
    small_vector_2<PyVar, 8> consts;         // constants
    small_vector_2<StrName, 8> varnames;         // local variables

    NameDictInt varnames_inv;
    std::vector<CodeBlock> blocks;
    NameDictInt labels;
    std::vector<FuncDecl_> func_decls;

    int start_line;
    int end_line;

    const CodeBlock& _get_block_codei(int codei) const{
        return blocks[iblocks[codei]];
    }

    CodeObject(std::shared_ptr<SourceData> src, const Str& name);
    void _gc_mark() const;
};

enum class FuncType{
    UNSET,
    NORMAL,
    SIMPLE,
    EMPTY,
    GENERATOR,
};

struct FuncDecl {
    struct KwArg {
        int index;              // index in co->varnames
        StrName key;            // name of this argument
        PyVar value;        // default value
    };
    CodeObject_ code;           // code object of this function

    small_vector_2<int, 6> args;      // indices in co->varnames
    small_vector_2<KwArg, 6> kwargs;  // indices in co->varnames

    int starred_arg = -1;       // index in co->varnames, -1 if no *arg
    int starred_kwarg = -1;     // index in co->varnames, -1 if no **kwarg
    bool nested = false;        // whether this function is nested

    const char* docstring;      // docstring of this function (weak ref)

    FuncType type = FuncType::UNSET;

    NameDictInt kw_to_index;

    void add_kwarg(int index, StrName key, PyVar value){
        kw_to_index.set(key, index);
        kwargs.push_back(KwArg{index, key, value});
    }
    
    void _gc_mark() const;
};

struct NativeFunc {
    NativeFuncC f;
    int argc;           // old style argc-based call
    FuncDecl_ decl;     // new style decl-based call
    any _userdata;

    NativeFunc(NativeFuncC f, int argc, any userdata={}): f(f), argc(argc), decl(nullptr), _userdata(std::move(userdata)) {}
    NativeFunc(NativeFuncC f, FuncDecl_ decl, any userdata={}): f(f), argc(-1), decl(decl), _userdata(std::move(userdata)) {}

    void check_size(VM* vm, ArgsView args) const;
    PyVar call(VM* vm, ArgsView args) const { return f(vm, args); }
};

struct Function{
    FuncDecl_ decl;
    PyVar _module;  // weak ref
    PyVar _class;   // weak ref
    NameDict_ _closure;

    explicit Function(FuncDecl_ decl, PyVar _module, PyVar _class, NameDict_ _closure):
        decl(decl), _module(_module), _class(_class), _closure(_closure) {}
};

template<>
struct Py_<Function> final: PyObject {
    Function _value;
    template<typename... Args>
    Py_(Type type, Args&&... args): PyObject(type), _value(std::forward<Args>(args)...) {
        // _enable_instance_dict();
    }
    void _obj_gc_mark() override {
        _value.decl->_gc_mark();
        if(_value._closure != nullptr) _gc_mark_namedict(_value._closure.get());
    }
};

template<>
struct Py_<NativeFunc> final: PyObject {
    NativeFunc _value;
    template<typename... Args>
    Py_(Type type, Args&&... args): PyObject(type), _value(std::forward<Args>(args)...) {
        // _enable_instance_dict();
    }
    void _obj_gc_mark() override {
        if(_value.decl != nullptr){
            _value.decl->_gc_mark();
        }
    }
};

template<typename T>
T& lambda_get_userdata(PyVar* p){
    static_assert(std::is_same_v<T, std::decay_t<T>>);
    int offset = p[-1] != PY_NULL ? -1 : -2;
    return PK_OBJ_GET(NativeFunc, p[offset])._userdata.cast<T>();
}

} // namespace pkpy