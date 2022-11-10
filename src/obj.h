#pragma once

#include "__stl__.h"
#include "str.h"

typedef int64_t _Int;
typedef double _Float;

const _Int _Int_MAX_POS = 9223372036854775807LL;
const _Int _Int_MAX_NEG = -9223372036854775807LL;
const _Float _FLOAT_INF_POS = INFINITY;
const _Float _FLOAT_INF_NEG = -INFINITY;

class PyObject;
class CodeObject;
class BasePointer;
class VM;

typedef std::shared_ptr<PyObject> PyVar;
typedef PyVar PyVarOrNull;

class PyVarList: public std::vector<PyVar> {
    PyVar& at(size_t) = delete;

    inline void __checkIndex(size_t i) const {
        if (i >= size()){
            auto msg = "std::vector index out of range, " + std::to_string(i) + " not in [0, " + std::to_string(size()) + ")";
            throw std::out_of_range(msg);
        }
    }
public:
    PyVar& operator[](size_t i) {
        __checkIndex(i);
        return std::vector<PyVar>::operator[](i);
    }

    const PyVar& operator[](size_t i) const {
        __checkIndex(i);
        return std::vector<PyVar>::operator[](i);
    }

    // define constructors the same as std::vector
    using std::vector<PyVar>::vector;
};

typedef std::unordered_map<_Str, PyVar> PyVarDict;
typedef std::shared_ptr<const BasePointer> _Pointer;

typedef PyVar (*_CppFunc)(VM*, PyVarList);
typedef std::shared_ptr<CodeObject> _Code;

struct _Func {
    _Str name;
    _Code code;
    std::vector<_Str> args;
    _Str starredArg;        // empty if no *arg
    PyVarDict kwArgs;       // empty if no k=v

    bool hasName(const _Str& val) const {
        bool _0 = std::find(args.begin(), args.end(), val) != args.end();
        bool _1 = starredArg == val;
        bool _2 = kwArgs.find(val) != kwArgs.end();
        return _0 || _1 || _2;
    }
};

struct BoundedMethod {
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
    int stop = 2147483647;  // contain types always use int32 as index, no support for int64

    void normalize(int len){
        if(start < 0) start += len;
        if(stop < 0) stop += len;
        if(start < 0) start = 0;
        if(stop > len) stop = len;
    }
};

class _Iterator {
protected:
    PyVar _ref;     // keep a reference to the object so it will not be deleted while iterating
    VM* vm;
public:
    virtual PyVar next() = 0;
    virtual bool hasNext() = 0;
    _Pointer var;
    _Iterator(VM* vm, PyVar _ref) : vm(vm), _ref(_ref) {}
};

typedef std::variant<_Int,_Float,bool,_Str,PyVarList,_CppFunc,_Func,std::shared_ptr<_Iterator>,BoundedMethod,_Range,_Slice,_Pointer> _Value;

#define UNREACHABLE() throw std::runtime_error("unreachable code! (this should be a bug, please report it)");

struct PyObject {
    PyVarDict attribs;
    _Value _native;

    inline bool isType(const PyVar& type){
        return attribs[__class__] == type;
    }

    // currently __name__ is only used for 'type'
    _Str getName(){
        _Value val = attribs["__name__"]->_native;
        return std::get<_Str>(val);
    }

    _Str getTypeName(){
        return attribs[__class__]->getName();
    }

    PyObject(_Value val): _native(val) {}
};