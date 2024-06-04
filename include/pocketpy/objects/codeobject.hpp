#pragma once

#include "pocketpy/common/any.hpp"
#include "pocketpy/objects/tuplelist.hpp"
#include "pocketpy/objects/object.hpp"
#include "pocketpy/objects/sourcedata.hpp"

namespace pkpy {

#if PK_ENABLE_STD_FUNCTION
using NativeFuncC = function<PyVar(VM*, ArgsView)>;
#else
typedef PyVar (*NativeFuncC)(VM*, ArgsView);
#endif

enum class BindType {
    DEFAULT,
    STATICMETHOD,
    CLASSMETHOD,
};

enum NameScope { NAME_LOCAL, NAME_GLOBAL, NAME_GLOBAL_UNKNOWN };

enum Opcode : uint8_t {

#define OPCODE(name) OP_##name,
#include "pocketpy/opcodes.h"
#undef OPCODE
};

struct Bytecode {
    uint8_t op;
    uint16_t arg;

    void set_signed_arg(int arg) {
        assert(arg >= INT16_MIN && arg <= INT16_MAX);
        this->arg = (int16_t)arg;
    }

    bool is_forward_jump() const { return op >= OP_JUMP_FORWARD && op <= OP_LOOP_BREAK; }
};

enum class CodeBlockType {
    NO_BLOCK,
    FOR_LOOP,
    WHILE_LOOP,
    CONTEXT_MANAGER,
    TRY_EXCEPT,
};

const inline uint8_t BC_NOARG = 0;
const inline int BC_KEEPLINE = -1;

struct CodeBlock {
    CodeBlockType type;
    int parent;  // parent index in blocks
    int start;   // start index of this block in codes, inclusive
    int end;     // end index of this block in codes, exclusive
    int end2;    // ...

    CodeBlock(CodeBlockType type, int parent, int start) :
        type(type), parent(parent), start(start), end(-1), end2(-1) {}

    int get_break_end() const {
        if(end2 != -1) return end2;
        return end;
    }
};

struct CodeObject;
struct FuncDecl;
using CodeObject_ = std::shared_ptr<CodeObject>;
using FuncDecl_ = std::shared_ptr<FuncDecl>;

struct CodeObject {
    struct LineInfo {
        int lineno;       // line number for each bytecode
        bool is_virtual;  // whether this bytecode is virtual (not in source code)
        int iblock;       // block index
    };

    std::shared_ptr<SourceData> src;
    Str name;

    vector<Bytecode> codes;
    vector<LineInfo> lines;

    small_vector_2<PyVar, 8> consts;      // constants
    small_vector_2<StrName, 8> varnames;  // local variables
    int nlocals;                          // varnames.size()

    NameDictInt varnames_inv;
    vector<CodeBlock> blocks;
    NameDictInt labels;
    vector<FuncDecl_> func_decls;

    int start_line;
    int end_line;

    const CodeBlock& _get_block_codei(int codei) const { return blocks[lines[codei].iblock]; }

    CodeObject(std::shared_ptr<SourceData> src, const Str& name);
    void _gc_mark(VM*) const;
};

enum class FuncType {
    UNSET,
    NORMAL,
    SIMPLE,
    EMPTY,
    GENERATOR,
};

struct FuncDecl {
    struct KwArg {
        int index;    // index in co->varnames
        StrName key;  // name of this argument
        PyVar value;  // default value
    };

    CodeObject_ code;  // code object of this function

    small_vector_2<int, 8> args;      // indices in co->varnames
    small_vector_2<KwArg, 6> kwargs;  // indices in co->varnames

    int starred_arg = -1;    // index in co->varnames, -1 if no *arg
    int starred_kwarg = -1;  // index in co->varnames, -1 if no **kwarg
    bool nested = false;     // whether this function is nested

    const char* docstring;  // docstring of this function (weak ref)

    FuncType type = FuncType::UNSET;

    NameDictInt kw_to_index;

    void add_kwarg(int index, StrName key, PyVar value) {
        kw_to_index.set(key, index);
        kwargs.push_back(KwArg{index, key, value});
    }

    void _gc_mark(VM*) const;
};

struct NativeFunc {
    NativeFuncC f;
    int argc;        // old style argc-based call
    FuncDecl_ decl;  // new style decl-based call
    any _userdata;

    NativeFunc(NativeFuncC f, int argc, any userdata = {}) :
        f(f), argc(argc), decl(nullptr), _userdata(std::move(userdata)) {}

    NativeFunc(NativeFuncC f, FuncDecl_ decl, any userdata = {}) :
        f(f), argc(-1), decl(decl), _userdata(std::move(userdata)) {}

    PyVar call(VM* vm, ArgsView args) const { return f(vm, args); }

    void _gc_mark(VM*) const;
};

struct Function {
    FuncDecl_ decl;
    PyObject* _module;  // weak ref
    PyObject* _class;   // weak ref
    NameDict_ _closure;

    explicit Function(FuncDecl_ decl, PyObject* _module, PyObject* _class, NameDict_ _closure) :
        decl(decl), _module(_module), _class(_class), _closure(_closure) {}

    void _gc_mark(VM*) const;
};

template <typename T>
T& lambda_get_userdata(PyVar* p) {
    static_assert(std::is_same_v<T, std::decay_t<T>>);
    int offset = p[-1] != nullptr ? -1 : -2;
    return p[offset].obj_get<NativeFunc>()._userdata.cast<T>();
}

}  // namespace pkpy
