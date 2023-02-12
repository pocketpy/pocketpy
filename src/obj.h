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
    Type type;
    pkpy::NameDict* _attr;
    // void* _tid;
    const int _size;

    inline bool is_attr_valid() const noexcept { return _attr != nullptr; }
    inline pkpy::NameDict& attr() noexcept { return *_attr; }
    inline PyVar& attr(const Str& name) noexcept { return (*_attr)[name]; }

    inline bool is_type(Type type) const noexcept{ return this->type == type; }
    virtual void* value() = 0;

    PyObject(Type type, const int size) : type(type), _size(size) {}
    virtual ~PyObject() { delete _attr; }
};

template <typename T>
struct Py_ : PyObject {
    T _value;

    Py_(Type type, T val) : PyObject(type, sizeof(Py_<T>)), _value(val) {
        if constexpr (std::is_same_v<T, Dummy> || std::is_same_v<T, Type>
        || std::is_same_v<T, pkpy::Function_> || std::is_same_v<T, pkpy::NativeFunc>) {
            _attr = new pkpy::NameDict();
        }else{
            _attr = nullptr;
        }
    }
    void* value() override { return &_value; }
};

#define OBJ_GET(T, obj) (((Py_<T>*)((obj).get()))->_value)
#define OBJ_NAME(obj) OBJ_GET(Str, (obj)->attr(__name__))

#define PY_CLASS(mod, name) \
    inline static Type _type(VM* vm) { return OBJ_GET(Type, vm->_modules[#mod]->attr(#name)); } \
    inline static const char* _mod() { return #mod; } \
    inline static const char* _name() { return #name; }


namespace pkpy {
    template<int N>
    struct MemBlock {
        std::vector<void*> a;
        int block_size;

        MemBlock(int block_size) : block_size(block_size) {
            new_block();
        }

        void new_block(){
            int8_t* total = (int8_t*)malloc(N * block_size);
            for(int i = 0; i < block_size; ++i){
                a.push_back((void*)(total + i * N));
            }
        }

        inline void* alloc(){
            if(a.empty()) new_block();
            void* p = a.back();
            a.pop_back();
            return p;
        }

        inline void dealloc(void* p) noexcept{
            a.push_back(p);
        }

        ~MemBlock(){
            free(a[0]);
        }
    };

    constexpr int kMemObjSize = sizeof(int) + sizeof(Py_<i64>);
    static thread_local MemBlock<kMemObjSize> _mem_pool(512);

    template<>
    struct SpAllocator<PyObject> {
        template<typename U>
        inline static int* alloc(){
            if constexpr (sizeof(int) + sizeof(U) == kMemObjSize) {
                return (int*)_mem_pool.alloc();
            }
            return (int*)malloc(sizeof(int) + sizeof(U));
        }

        inline static void dealloc(int* counter){
            PyObject* obj = (PyObject*)(counter + 1);
            obj->~PyObject();
            if(obj->_size == kMemObjSize - sizeof(int)){
                _mem_pool.dealloc(counter);
            }else{
                free(counter);
            }
        }
    };
}