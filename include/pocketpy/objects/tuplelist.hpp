#pragma once

#include "pocketpy/common/vector.hpp"
#include "pocketpy/objects/base.hpp"

namespace pkpy {

struct Tuple {
    const static int INLINED_SIZE = 3;

    PyVar* _args;
    PyVar _inlined[INLINED_SIZE];
    int _size;

    Tuple(int n);
    Tuple(Tuple&& other) noexcept;
    Tuple(const Tuple& other) = delete;
    Tuple& operator= (const Tuple& other) = delete;
    Tuple& operator= (Tuple&& other) = delete;
    ~Tuple();

    Tuple(PyVar, PyVar);
    Tuple(PyVar, PyVar, PyVar);

    bool is_inlined() const { return _args == _inlined; }

    PyVar& operator[] (int i) { return _args[i]; }

    PyVar operator[] (int i) const { return _args[i]; }

    int size() const { return _size; }

    PyVar* begin() const { return _args; }

    PyVar* end() const { return _args + _size; }

    PyVar* data() const { return _args; }

    void _gc_mark(VM*) const;
};

struct List : public vector<PyVar> {
    using vector<PyVar>::vector;
    void _gc_mark(VM*) const;

    Tuple to_tuple() const {
        Tuple ret(size());
        for(int i = 0; i < size(); i++)
            ret[i] = (*this)[i];
        return ret;
    }
};

// a lightweight view for function args, it does not own the memory
struct ArgsView {
    PyVar* _begin;
    PyVar* _end;

    ArgsView(PyVar* begin, PyVar* end) : _begin(begin), _end(end) {}

    ArgsView(const Tuple& t) : _begin(t.begin()), _end(t.end()) {}

    PyVar* begin() const { return _begin; }

    PyVar* end() const { return _end; }

    int size() const { return _end - _begin; }

    bool empty() const { return _begin == _end; }

    PyVar operator[] (int i) const { return _begin[i]; }

    List to_list() const;
    Tuple to_tuple() const;
};

}  // namespace pkpy
