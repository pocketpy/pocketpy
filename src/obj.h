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

typedef pkpy::shared_ptr<Function> _Func;
typedef std::variant<PyVar,_Int,_Float,bool,_Str,PyVarList,_CppFunc,_Func,pkpy::shared_ptr<_Iterator>,_BoundedMethod,_Range,_Slice,_Pointer> _Value;

const int VALUE_SIZE = sizeof(_Value);


struct PyObject {
    PyVarDict attribs;
    _Value _native;
    PyVar _type;

    inline bool isType(const PyVar& type){
        return this->_type == type;
    }

    inline void setType(const PyVar& type){
        this->_type = type;
        // this->attribs[__class__] = type;
    }

    // currently __name__ is only used for 'type'
    _Str getName(){
        _Value val = attribs[__name__]->_native;
        return std::get<_Str>(val);
    }

    _Str getTypeName(){
        return _type->getName();
    }

    PyObject(const _Value& val): _native(val) {}
    PyObject(_Value&& val): _native(std::move(val)) {}
};