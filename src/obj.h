#pragma once

#include "safestl.h"

struct CodeObject;
struct Frame;
struct BaseRef;
class VM;

typedef std::function<PyVar(VM*, const pkpy::Args&)> NativeFuncRaw;
typedef pkpy::shared_ptr<CodeObject> CodeObject_;

namespace pkpy{
struct NativeFunc {
    NativeFuncRaw f;
    int argc;       // DONOT include self
    bool method;
    
    NativeFunc(NativeFuncRaw f, int argc, bool method) : f(f), argc(argc), method(method) {}
    inline PyVar operator()(VM* vm, const pkpy::Args& args) const;
};

struct Function {
    Str name;
    CodeObject_ code;
    std::vector<Str> args;
    Str starredArg;                // empty if no *arg
    pkpy::NameDict kwArgs;          // empty if no k=v
    std::vector<Str> kwArgsOrder;

    bool hasName(const Str& val) const {
        bool _0 = std::find(args.begin(), args.end(), val) != args.end();
        bool _1 = starredArg == val;
        bool _2 = kwArgs.find(val) != kwArgs.end();
        return _0 || _1 || _2;
    }
};

struct BoundMethod {
    PyVar obj;
    PyVar method;
};

struct Range {
    i64 start = 0;
    i64 stop = -1;
    i64 step = 1;
};

struct Slice {
    int start = 0;
    int stop = 0x7fffffff; 

    void normalize(int len){
        if(start < 0) start += len;
        if(stop < 0) stop += len;
        if(start < 0) start = 0;
        if(stop > len) stop = len;
    }
};

typedef shared_ptr<Function> Function_;
}

class BaseIter {
protected:
    VM* vm;
    PyVar _ref;     // keep a reference to the object so it will not be deleted while iterating
public:
    virtual PyVar next() = 0;
    virtual bool has_next() = 0;
    PyVarRef var;
    BaseIter(VM* vm, PyVar _ref) : vm(vm), _ref(_ref) {}
    virtual ~BaseIter() = default;
};

struct PyObject {
    PyVar type;
    pkpy::NameDict attribs;
    void* _tid;

    inline bool is_type(const PyVar& type) const noexcept{ return this->type == type; }
    virtual void* value() = 0;

    PyObject(const PyVar& type, void* _tid) : type(type), _tid(_tid) {}
    virtual ~PyObject() = default;
};

template <typename T>
struct Py_ : PyObject {
    T _value;

    Py_(const PyVar& type, T val) : PyObject(type, tid<T>()), _value(val) {}
    void* value() override { return &_value; }
};

#define OBJ_GET(T, obj) (((Py_<T>*)((obj).get()))->_value)
#define OBJ_NAME(obj) OBJ_GET(Str, (obj)->attribs[__name__])
#define OBJ_TP_NAME(obj) OBJ_GET(Str, (obj)->type->attribs[__name__])

#define PY_CLASS(mod, name) \
    inline static PyVar _type(VM* vm) { return vm->_modules[#mod]->attribs[#name]; } \
    inline static const char* _mod() { return #mod; } \
    inline static const char* _name() { return #name; }

#define PY_BUILTIN_CLASS(name) inline static PyVar _type(VM* vm) { return vm->tp_##name; }

static thread_local emhash8::HashMap<void*, std::vector<int*>> _obj_pool;

namespace pkpy {
    template<>
    struct sp_deleter<PyObject> {
        inline static void call(int* counter) {
            PyObject* obj = (PyObject*)(counter + 1);
            std::vector<int*>& pool = _obj_pool[obj->_tid];
            if(obj->_tid==tid<Dummy>() || pool.size() > 32){
                obj->~PyObject();
                free(counter);
            }else{
                pool.push_back(counter);
            }
        }
    };
}