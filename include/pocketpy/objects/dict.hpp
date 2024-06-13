#pragma once

#include "pocketpy/objects/base.hpp"
#include "pocketpy/objects/tuplelist.hpp"
#include "pocketpy/objects/dict.h"

namespace pkpy {

struct Dict : private pkpy_Dict {
    Dict() {
        pkpy_Dict__ctor(this);
    }

    Dict(Dict&& other) {
        std::memcpy(this, &other, sizeof(Dict));
        pkpy_Dict__ctor(&other);
    }

    Dict(const Dict& other) {
        // OPTIMIZEME: reduce copy
        auto clone = pkpy_Dict__copy(&other);
        std::memcpy(this, &clone, sizeof(Dict));
    }
    
    Dict& operator= (const Dict&) = delete;
    Dict& operator= (Dict&&) = delete;

    int size() const { return count; }

    void set(VM* vm, PyVar key, PyVar val) {
        pkpy_Dict__set(this, vm, *reinterpret_cast<::pkpy_Var*>(&key), *reinterpret_cast<::pkpy_Var*>(&val));
    }

    PyVar try_get(VM* vm, PyVar key) const {
        auto res = pkpy_Dict__try_get(this, vm, *reinterpret_cast<::pkpy_Var*>(&key));
        if (!res) return nullptr;
        return *reinterpret_cast<PyVar*>(&res);
    }

    bool contains(VM* vm, PyVar key) const {
        return pkpy_Dict__contains(this, vm, *reinterpret_cast<::pkpy_Var*>(&key));
    }

    bool del(VM* vm, PyVar key) {
        return pkpy_Dict__del(this, vm, *reinterpret_cast<::pkpy_Var*>(&key));
    }

    void update(VM* vm, const Dict& other) {
        pkpy_Dict__update(this, vm, &other);
    }

    template <typename __Func>
    void apply(__Func f) const {
        pkpy_DictIter it = iter();
        PyVar key, val;
        while(pkpy_DictIter__next(&it, reinterpret_cast<::pkpy_Var*>(&key), reinterpret_cast<::pkpy_Var*>(&val))) {
            f(key, val);
        }
    }

    Tuple keys() const {
        Tuple res(count);
        pkpy_DictIter it = iter();
        PyVar key, val;
        int i = 0;
        while(pkpy_DictIter__next(&it, reinterpret_cast<::pkpy_Var*>(&key), reinterpret_cast<::pkpy_Var*>(&val))) {
            res[i++] = key;
        }
        return res;
    }

    Tuple values() const {
        Tuple res(count);
        pkpy_DictIter it = iter();
        PyVar key, val;
        int i = 0;
        while(pkpy_DictIter__next(&it, reinterpret_cast<::pkpy_Var*>(&key), reinterpret_cast<::pkpy_Var*>(&val))) {
            res[i++] = val;
        }
        return res;
    }

    pkpy_DictIter iter() const {
        return pkpy_Dict__iter(this);
    }

    void clear() {
        pkpy_Dict__clear(this);
    }

    ~Dict() {
        pkpy_Dict__dtor(this);
    }

    void _gc_mark(VM*) const;
};

}  // namespace pkpy
