#pragma once

#include "__stl__.h"
#include "str.h"

class PyObject;
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


class PyVarDict: public std::unordered_map<_Str, PyVar> {
    PyVar& at(const _Str&) = delete;

public:
    PyVar& operator[](const _Str& key) {
        return std::unordered_map<_Str, PyVar>::operator[](key);
    }

    const PyVar& operator[](const _Str& key) const {
        auto it = find(key);
        if (it == end()){
            auto msg = "std::unordered_map key not found, '" + key.str() + "'";
            throw std::out_of_range(msg);
        }
        return it->second;
    }

    // define constructors the same as std::unordered_map
    using std::unordered_map<_Str, PyVar>::unordered_map;
};