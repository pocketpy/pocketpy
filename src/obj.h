#pragma once

#include "safestl.h"

typedef int64_t _Int;
typedef double _Float;

const _Int _Int_MAX_POS = 9223372036854775807LL;
const _Int _Int_MAX_NEG = -9223372036854775807LL;
const _Float _FLOAT_INF_POS = INFINITY;
const _Float _FLOAT_INF_NEG = -INFINITY;

#define PK_VERSION "0.2.5"

class CodeObject;
class BasePointer;
class VM;
class PkExportedResource {};

typedef std::shared_ptr<const BasePointer> _Pointer;
typedef PyVar (*_CppFunc)(VM*, PyVarList);
typedef std::shared_ptr<CodeObject> _Code;

struct Function {
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
    int stop = 2147483647;  // container types always use int32 as index, no support for int64

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

typedef std::shared_ptr<Function> _Func;
typedef std::variant<_Int,_Float,bool,_Str,PyVarList,_CppFunc,_Func,std::shared_ptr<_Iterator>,_BoundedMethod,_Range,_Slice,_Pointer> _Value;

const int _SIZEOF_VALUE = sizeof(_Value);

#ifdef POCKETPY_H
#define UNREACHABLE() throw std::runtime_error( "L" + std::to_string(__LINE__) + " UNREACHABLE()! This should be a bug, please report it");
#else
#define UNREACHABLE() throw std::runtime_error( __FILE__ + std::string(":") + std::to_string(__LINE__) + " UNREACHABLE()!");
#endif

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
