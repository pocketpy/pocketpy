#include "pocketpy/any.h"

namespace pkpy{

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