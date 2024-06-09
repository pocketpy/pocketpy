#include "pocketpy/common/any.hpp"
#include "pocketpy/common/utils.hpp"

#include <cstdio>

namespace pkpy {

void any::__bad_any_cast(const std::type_index expected, const std::type_index actual) {
    PK_FATAL_ERROR("bad_any_cast: expected %s, got %s\n", expected.name(), actual.name())
}

any::any(any&& other) noexcept : data(other.data), _vt(other._vt) {
    other.data = nullptr;
    other._vt = nullptr;
}

any& any::operator= (any&& other) noexcept {
    if(data) _vt->deleter(data);
    data = other.data;
    _vt = other._vt;
    other.data = nullptr;
    other._vt = nullptr;
    return *this;
}

}  // namespace pkpy
