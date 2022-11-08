#pragma once

#include <unordered_map>
#include <memory>
#include <variant>
#include <functional>
#include <stack>
#include <cmath>
#include <stdexcept>

#include "str.h"

class PyObject;
class CodeObject;
class BasePointer;
class VM;

typedef std::shared_ptr<PyObject> PyVar;
typedef PyVar PyVarOrNull;
typedef std::vector<PyVar> PyVarList;
typedef std::unordered_map<_Str, PyVar> StlDict;
typedef std::shared_ptr<const BasePointer> _Pointer;

typedef PyVar (*_CppFunc)(VM*, PyVarList);
typedef std::shared_ptr<CodeObject> _Code;

struct _Func {
    _Str name;
    _Code code;
    std::vector<_Str> args;
    _Str starredArg;        // empty if no *arg
    StlDict kwArgs;         // empty if no k=v
    _Str doubleStarredArg;  // empty if no **kwargs

    bool hasName(const _Str& val) const {
        bool _0 = std::find(args.begin(), args.end(), val) != args.end();
        bool _1 = starredArg == val;
        bool _2 = kwArgs.find(val) != kwArgs.end();
        bool _3 = doubleStarredArg == val;
        return _0 || _1 || _2 || _3;
    }
};

struct BoundedMethod {
    PyVar obj;
    PyVar method;
};

struct _Range {
    int start = 0;
    int stop = -1;
    int step = 1;
};

struct _Slice {
    int start = 0;
    int stop = 2147483647;

    void normalize(int len){
        if(start < 0) start += len;
        if(stop < 0) stop += len;
        if(start < 0) start = 0;
        if(stop > len) stop = len;
    }
};

class _Iterator {
private:
    PyVar _ref;     // keep a reference to the object so it will not be deleted while iterating
public:
    virtual PyVar next() = 0;
    virtual bool hasNext() = 0;

    _Pointer var;

    _Iterator(PyVar _ref) : _ref(_ref) {}
};

typedef std::variant<int,float,bool,_Str,PyVarList,_CppFunc,_Func,std::shared_ptr<_Iterator>,BoundedMethod,_Range,_Slice,_Pointer> _Value;

#define UNREACHABLE() throw std::runtime_error("Unreachable code")

struct PyObject {
    StlDict attribs;
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