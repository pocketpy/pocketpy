#pragma once

#include "pocketpy/common/any.h"
#include "pocketpy/common/traits.hpp"
#include "pocketpy/objects/tuplelist.hpp"
#include "pocketpy/objects/namedict.hpp"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/common/smallmap.h"
#include "pocketpy/objects/codeobject.h"

namespace pkpy {

typedef PyVar (*NativeFuncC)(VM*, ArgsView);

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

    ~NativeFunc() {
        if(decl) PK_DECREF(decl);
    }
};

struct Function {
    PK_ALWAYS_PASS_BY_POINTER(Function)

    FuncDecl_ decl;
    PyObject* _module;  // weak ref
    PyObject* _class;   // weak ref
    NameDict* _closure; // strong ref

    Function(FuncDecl_ decl, PyObject* _module, PyObject* _class, NameDict* _closure) :
        decl(decl), _module(_module), _class(_class), _closure(_closure) {
            PK_INCREF(decl);
        }

    void _gc_mark(VM*) const;

    ~Function() {
        PK_DECREF(decl);
        delete _closure;
    }
};

template <typename T>
T lambda_get_userdata(PyVar* p) {
    static_assert(std::is_same_v<T, std::decay_t<T>>);
    static_assert(is_pod_v<T>);
    int offset = p[-1] != nullptr ? -1 : -2;
    NativeFunc& nf = p[offset].obj_get<NativeFunc>();
    return nf._userdata.as<T>();
}

}  // namespace pkpy
