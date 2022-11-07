#pragma once

#include "obj.h"

class Frame;

struct BasePointer {
    virtual PyVar get(VM*, Frame*) const = 0;
    virtual void set(VM*, Frame*, PyVar) const = 0;
};

enum NameScope {
    NAME_LOCAL = 0,
    NAME_GLOBAL = 1,
    NAME_ATTR = 2,
};

struct NamePointer : BasePointer {
    const _Str name;
    const NameScope scope;
    NamePointer(const _Str& name, NameScope scope) : name(name), scope(scope) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;

    bool operator==(const NamePointer& other) const {
        return name == other.name && scope == other.scope;
    }
};

struct AttrPointer : BasePointer {
    mutable PyVar obj;
    const NamePointer* attr;
    AttrPointer(PyVar obj, const NamePointer* attr) : obj(obj), attr(attr) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
};

struct IndexPointer : BasePointer {
    mutable PyVar obj;
    const PyVar index;
    IndexPointer(PyVar obj, PyVar index) : obj(obj), index(index) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
};
