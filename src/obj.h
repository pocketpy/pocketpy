#pragma once

#include "safestl.h"

typedef int64_t _Int;
typedef double _Float;

struct CodeObject;
struct BasePointer;
class VM;
class Frame;

typedef pkpy::shared_ptr<const BasePointer> _Pointer;
typedef PyVar (*_CppFunc)(VM*, const pkpy::ArgList&);
typedef pkpy::shared_ptr<CodeObject> _Code;

struct Function {
    _Str name;
    _Code code;
    std::vector<_Str> args;
    _Str starredArg;        // empty if no *arg
    PyVarDict kwArgs;       // empty if no k=v
    std::vector<_Str> kwArgsOrder;

    bool hasName(const _Str& val) const {
        bool _0 = std::find(args.begin(), args.end(), val) != args.end();
        bool _1 = starredArg == val;
        bool _2 = kwArgs.find(val) != kwArgs.end();
        return _0 || _1 || _2;
    }
};

struct _BoundedMethod {
    PyVar obj;
    PyVar method;
};

struct _Range {
    _Int start = 0;
    _Int stop = -1;
    _Int step = 1;
};

struct _Slice {
    int start = 0;
    int stop = 0x7fffffff; 

    void normalize(int len){
        if(start < 0) start += len;
        if(stop < 0) stop += len;
        if(start < 0) start = 0;
        if(stop > len) stop = len;
    }
};

class BaseIterator {
protected:
    VM* vm;
    PyVar _ref;     // keep a reference to the object so it will not be deleted while iterating
public:
    virtual PyVar next() = 0;
    virtual bool hasNext() = 0;
    _Pointer var;
    BaseIterator(VM* vm, PyVar _ref) : vm(vm), _ref(_ref) {}
    virtual ~BaseIterator() = default;
};

typedef pkpy::shared_ptr<Function> _Func;
typedef pkpy::shared_ptr<BaseIterator> _Iterator;

struct PyObject {
    PyVarDict attribs;
    PyVar _type;

    inline bool isType(const PyVar& type){ return this->_type == type; }

    // currently __name__ is only used for 'type'
    PyVar _typeName(){ return _type->attribs[__name__]; }
};

template <typename T>
struct Py_ : PyObject {
    T _value;

    Py_(const T& val, const PyVar& type) {
        _value = val;
        _type = type;
    }
    Py_(T&& val, const PyVar& type) {
        _value = std::move(val);
        _type = type;
    }
};

#define UNION_GET(T, obj) (((Py_<T>*)((obj).get()))->_value)
#define UNION_TP_NAME(obj) UNION_GET(_Str, (obj)->_typeName())