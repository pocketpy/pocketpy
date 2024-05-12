#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"
#include "vector.h"

namespace pkpy {

using List = pod_vector<PyVar, 4>;

struct Tuple {
    PyVar* _args;
    PyVar _inlined[3];
    int _size;

    Tuple(int n);
    Tuple(const Tuple& other);
    Tuple(Tuple&& other) noexcept;
    Tuple(List&& other) noexcept;
    ~Tuple();

    Tuple(PyVar, PyVar);
    Tuple(PyVar, PyVar, PyVar);
    Tuple(PyVar, PyVar, PyVar, PyVar);

    bool is_inlined() const { return _args == _inlined; }
    PyVar& operator[](int i){ return _args[i]; }
    PyVar operator[](int i) const { return _args[i]; }

    int size() const { return _size; }

    PyVar* begin() const { return _args; }
    PyVar* end() const { return _args + _size; }
    PyVar* data() const { return _args; }
};

// a lightweight view for function args, it does not own the memory
struct ArgsView{
    PyVar* _begin;
    PyVar* _end;

    ArgsView(PyVar* begin, PyVar* end) : _begin(begin), _end(end) {}
    ArgsView(const Tuple& t) : _begin(t.begin()), _end(t.end()) {}

    PyVar* begin() const { return _begin; }
    PyVar* end() const { return _end; }
    int size() const { return _end - _begin; }
    bool empty() const { return _begin == _end; }
    PyVar operator[](int i) const { return _begin[i]; }

    List to_list() const;
    Tuple to_tuple() const;
};

}   // namespace pkpy