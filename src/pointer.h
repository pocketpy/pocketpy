#pragma once

#include "obj.h"

class Frame;

struct BasePointer {
    virtual PyVar get(VM*, Frame*) const = 0;
    virtual void set(VM*, Frame*, PyVar) const = 0;
    virtual void del(VM*, Frame*) const = 0;
    virtual ~BasePointer() = default;
};

enum NameScope {
    NAME_LOCAL = 0,
    NAME_GLOBAL = 1,
    NAME_ATTR = 2,
};

struct NamePointer : BasePointer {
    const std::pair<_Str, NameScope>* pair;
    NamePointer(const std::pair<_Str, NameScope>* pair) : pair(pair) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};

struct AttrPointer : BasePointer {
    mutable PyVar obj;
    const NamePointer attr;
    AttrPointer(PyVar obj, const NamePointer attr) : obj(obj), attr(attr) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};

struct IndexPointer : BasePointer {
    mutable PyVar obj;
    PyVar index;
    IndexPointer(PyVar obj, PyVar index) : obj(obj), index(index) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};

struct CompoundPointer : BasePointer {
    PyVarList varRefs;
    CompoundPointer(const PyVarList& varRefs) : varRefs(varRefs) {}
    CompoundPointer(PyVarList&& varRefs) : varRefs(std::move(varRefs)) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};

struct UserPointer : BasePointer {
    VarRef p;
    uint64_t f_id;
    UserPointer(VarRef p, uint64_t f_id) : p(p), f_id(f_id) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};