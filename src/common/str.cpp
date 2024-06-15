#include "pocketpy/common/str.hpp"

#include <cassert>
#include <ostream>
#include <algorithm>
#include <cmath>
#include <map>

namespace pkpy {

Str::Str(pair<char*, int> detached) {
    this->size = detached.second;
    this->is_ascii = true;
    this->is_sso = false;
    this->_ptr = detached.first;
    for(int i = 0; i < size; i++) {
        if(!isascii(_ptr[i])) {
            is_ascii = false;
            break;
        }
    }
    assert(_ptr[size] == '\0');
}

static std::map<std::string_view, uint16_t>& _interned() {
    static std::map<std::string_view, uint16_t> interned;
    return interned;
}

static std::map<uint16_t, std::string>& _r_interned() {
    static std::map<uint16_t, std::string> r_interned;
    return r_interned;
}

std::string_view StrName::sv() const { return _r_interned()[index]; }
const char* StrName::c_str() const { return _r_interned()[index].c_str(); }

uint32_t StrName::_pesudo_random_index = 0;

StrName StrName::get(std::string_view s) {
    // TODO: PK_GLOBAL_SCOPE_LOCK()
    auto it = _interned().find(s);
    if(it != _interned().end()) return StrName(it->second);
    // generate new index
    // https://github.com/python/cpython/blob/3.12/Objects/dictobject.c#L175
    uint16_t index = ((_pesudo_random_index * 5) + 1) & 65535;
    if(index == 0) PK_FATAL_ERROR("StrName index overflow\n")
    auto res = _r_interned().emplace(index, s);
    assert(res.second);
    s = std::string_view(res.first->second);
    _interned()[s] = index;
    _pesudo_random_index = index;
    return StrName(index);
}

// unary operators
const StrName __repr__ = StrName::get("__repr__");
const StrName __str__ = StrName::get("__str__");
const StrName __hash__ = StrName::get("__hash__");
const StrName __len__ = StrName::get("__len__");
const StrName __iter__ = StrName::get("__iter__");
const StrName __next__ = StrName::get("__next__");
const StrName __neg__ = StrName::get("__neg__");
// logical operators
const StrName __eq__ = StrName::get("__eq__");
const StrName __lt__ = StrName::get("__lt__");
const StrName __le__ = StrName::get("__le__");
const StrName __gt__ = StrName::get("__gt__");
const StrName __ge__ = StrName::get("__ge__");
const StrName __contains__ = StrName::get("__contains__");
// binary operators
const StrName __add__ = StrName::get("__add__");
const StrName __radd__ = StrName::get("__radd__");
const StrName __sub__ = StrName::get("__sub__");
const StrName __rsub__ = StrName::get("__rsub__");
const StrName __mul__ = StrName::get("__mul__");
const StrName __rmul__ = StrName::get("__rmul__");
const StrName __truediv__ = StrName::get("__truediv__");
const StrName __floordiv__ = StrName::get("__floordiv__");
const StrName __mod__ = StrName::get("__mod__");
const StrName __pow__ = StrName::get("__pow__");
const StrName __matmul__ = StrName::get("__matmul__");
const StrName __lshift__ = StrName::get("__lshift__");
const StrName __rshift__ = StrName::get("__rshift__");
const StrName __and__ = StrName::get("__and__");
const StrName __or__ = StrName::get("__or__");
const StrName __xor__ = StrName::get("__xor__");
const StrName __invert__ = StrName::get("__invert__");
// indexer
const StrName __getitem__ = StrName::get("__getitem__");
const StrName __setitem__ = StrName::get("__setitem__");
const StrName __delitem__ = StrName::get("__delitem__");

// specials
const StrName __new__ = StrName::get("__new__");
const StrName __init__ = StrName::get("__init__");
const StrName __call__ = StrName::get("__call__");
const StrName __divmod__ = StrName::get("__divmod__");
const StrName __enter__ = StrName::get("__enter__");
const StrName __exit__ = StrName::get("__exit__");
const StrName __name__ = StrName::get("__name__");
const StrName __all__ = StrName::get("__all__");
const StrName __package__ = StrName::get("__package__");
const StrName __path__ = StrName::get("__path__");
const StrName __class__ = StrName::get("__class__");
const StrName __missing__ = StrName::get("__missing__");

const StrName pk_id_add = StrName::get("add");
const StrName pk_id_set = StrName::get("set");
const StrName pk_id_long = StrName::get("long");
const StrName pk_id_complex = StrName::get("complex");

}  // namespace pkpy
