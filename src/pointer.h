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
    const _Pointer root;
    const NamePointer* attr;
    AttrPointer(const _Pointer& root, const NamePointer* attr) : root(root), attr(attr) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
};

struct IndexPointer : BasePointer {
    const _Pointer root;
    const PyVar index;
    IndexPointer(_Pointer root, PyVar index) : root(root), index(index) {}

    PyVar get(VM* vm, Frame* frame) const;
    void set(VM* vm, Frame* frame, PyVar val) const;
};
