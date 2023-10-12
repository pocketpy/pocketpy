#include "pocketpy/obj.h"

namespace pkpy{
    PyObject::~PyObject() {
        if(_attr == nullptr) return;
        _attr->~NameDict();
        pool128_dealloc(_attr);
    }
}   // namespace pkpy