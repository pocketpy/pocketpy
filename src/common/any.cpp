#include "pocketpy/common/any.hpp"

#include <stdexcept>
#include <cstdio>

namespace pkpy {

void any::__bad_any_cast(const std::type_index expected, const std::type_index actual) {
    char error[256];
    snprintf(error, sizeof(error), "bad_any_cast: expected %s, got %s", expected.name(), actual.name());
    throw std::runtime_error(error);
}

any::any(any&& other) noexcept : data(other.data), _vt(other._vt) {
    other.data = nullptr;
    other._vt = nullptr;
}

any& any::operator= (any&& other) noexcept {
    if(data) {
        _vt->deleter(data);
    }
    data = other.data;
    _vt = other._vt;
    other.data = nullptr;
    other._vt = nullptr;
    return *this;
}

}  // namespace pkpy
