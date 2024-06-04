#include "pocketpy/objects/tuplelist.hpp"

namespace pkpy {

Tuple::Tuple(int n) {
    if(n <= INLINED_SIZE) {
        this->_args = _inlined;
    } else {
        this->_args = (PyVar*)std::malloc(n * sizeof(PyVar));
    }
    this->_size = n;
}

Tuple::Tuple(Tuple&& other) noexcept {
    _size = other._size;
    if(other.is_inlined()) {
        _args = _inlined;
        for(int i = 0; i < _size; i++)
            _args[i] = other._args[i];
    } else {
        _args = other._args;
        other._args = other._inlined;
        other._size = 0;
    }
}

Tuple::Tuple(PyVar _0, PyVar _1) : Tuple(2) {
    _args[0] = _0;
    _args[1] = _1;
}

Tuple::Tuple(PyVar _0, PyVar _1, PyVar _2) : Tuple(3) {
    _args[0] = _0;
    _args[1] = _1;
    _args[2] = _2;
}

Tuple::~Tuple() {
    if(!is_inlined()) std::free(_args);
}

List ArgsView::to_list() const {
    List ret(size());
    for(int i = 0; i < size(); i++)
        ret[i] = _begin[i];
    return ret;
}

Tuple ArgsView::to_tuple() const {
    Tuple ret(size());
    for(int i = 0; i < size(); i++)
        ret[i] = _begin[i];
    return ret;
}

}  // namespace pkpy
