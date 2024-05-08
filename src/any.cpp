#include "pocketpy/any.h"

namespace pkpy{

void any::__bad_any_cast(const std::type_index expected, const std::type_index actual){
    Str error = _S("bad_any_cast: expected ", expected.name(), ", got ", actual.name());
    throw std::runtime_error(error.c_str());
}

any::any(any&& other) noexcept: data(other.data), _vt(other._vt){
    other.data = nullptr;
    other._vt = nullptr;
}

any& any::operator=(any&& other) noexcept{
    if(data) _vt->deleter(data);
    data = other.data;
    _vt = other._vt;
    other.data = nullptr;
    other._vt = nullptr;
    return *this;
}

}   // namespace pkpy