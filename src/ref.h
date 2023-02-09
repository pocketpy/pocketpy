#pragma once

#include "obj.h"

struct BaseRef {
    virtual PyVar get(VM*, Frame*) const = 0;
    virtual void set(VM*, Frame*, PyVar) const = 0;
    virtual void del(VM*, Frame*) const = 0;
    virtual ~BaseRef() = default;
};

enum NameScope {
    NAME_LOCAL = 0,
    NAME_GLOBAL,
    NAME_ATTR,
    NAME_SPECIAL,
};

struct NameRef : BaseRef {
    std::pair<_Str, NameScope>* _pair;
    inline const _Str& name() const { return _pair->first; }
    inline NameScope scope() const { return _pair->second; }
    NameRef(std::pair<_Str, NameScope>& pair) : _pair(&pair) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};

struct AttrRef : BaseRef {
    mutable PyVar obj;
    NameRef attr;
    AttrRef(PyVar obj, NameRef attr) : obj(obj), attr(attr) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};

struct IndexRef : BaseRef {
    mutable PyVar obj;
    PyVar index;
    IndexRef(PyVar obj, PyVar index) : obj(obj), index(index) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};

struct TupleRef : BaseRef {
    _Tuple objs;
    TupleRef(_Tuple&& objs) : objs(std::move(objs)) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
    void del(VM* vm, Frame* frame) const;
};