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

struct CodeObject;
struct FuncDecl;
using CodeObject_ = std::shared_ptr<CodeObject>;
using FuncDecl_ = std::shared_ptr<FuncDecl>;

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
    std::shared_ptr<SourceData> src;
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

    CodeObject(std::shared_ptr<SourceData> src, const Str& name);
    void _gc_mark() const;
    void write(VM* vm, CodeObjectSerializer& ss) const;
    Str serialize(VM* vm) const;
};

struct FuncDecl {
    struct KwArg {
        int key;                // index in co->varnames
        PyObject* value;        // default value
    };
    CodeObject_ code;           // code object of this function
    pod_vector<int> args;       // indices in co->varnames
    pod_vector<KwArg> kwargs;   // indices in co->varnames
    int starred_arg = -1;       // index in co->varnames, -1 if no *arg
    int starred_kwarg = -1;     // index in co->varnames, -1 if no **kwarg
    bool nested = false;        // whether this function is nested

    Str signature;              // signature of this function
    Str docstring;              // docstring of this function
    void _gc_mark() const;
};

struct UserData{
    char data[15];
    bool empty;

    UserData(): empty(true) {}
    template<typename T>
    UserData(T t): empty(false){
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(sizeof(T) <= sizeof(data));
        memcpy(data, &t, sizeof(T));
    }

    template <typename T>
    T get() const{
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(sizeof(T) <= sizeof(data));
#if PK_DEBUG_EXTRA_CHECK
        PK_ASSERT(!empty);
#endif
        return reinterpret_cast<const T&>(data);
    }
};

struct NativeFunc {
    NativeFuncC f;

    // old style argc-based call
    int argc;

    // new style decl-based call
    FuncDecl_ decl;

    UserData _userdata;

    void set_userdata(UserData data) {
        if(!_userdata.empty && !data.empty){
            // override is not supported
            throw std::runtime_error("userdata already set");
        }
        _userdata = data;
    }

    NativeFunc(NativeFuncC f, int argc, bool method);
    NativeFunc(NativeFuncC f, FuncDecl_ decl);

    void check_size(VM* vm, ArgsView args) const;
    PyObject* call(VM* vm, ArgsView args) const;
};

struct Function{
    FuncDecl_ decl;
    PyObject* _module;
    NameDict_ _closure;
};

template<>
struct Py_<Function> final: PyObject {
    Function _value;
    template<typename... Args>
    Py_(Type type, Args&&... args): PyObject(type), _value(std::forward<Args>(args)...) {
        enable_instance_dict();
    }
    void _obj_gc_mark() override {
        _value.decl->_gc_mark();
        if(_value._module != nullptr) PK_OBJ_MARK(_value._module);
        if(_value._closure != nullptr) gc_mark_namedict(*_value._closure);
    }
};

template<>
struct Py_<NativeFunc> final: PyObject {
    NativeFunc _value;
    template<typename... Args>
    Py_(Type type, Args&&... args): PyObject(type), _value(std::forward<Args>(args)...) {
        enable_instance_dict();
    }
    void _obj_gc_mark() override {
        if(_value.decl != nullptr){
            _value.decl->_gc_mark();
        }
    }
};

template<typename T>
T lambda_get_userdata(PyObject** p){
    if(p[-1] != PY_NULL) return PK_OBJ_GET(NativeFunc, p[-1])._userdata.get<T>();
    else return PK_OBJ_GET(NativeFunc, p[-2])._userdata.get<T>();
}

} // namespace pkpy