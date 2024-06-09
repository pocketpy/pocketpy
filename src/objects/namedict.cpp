#include "pocketpy/objects/namedict.hpp"

namespace pkpy {

#define HASH_PROBE_1(key, ok, i)                                                                                       \
    ok = false;                                                                                                        \
    i = key.index & _mask;                                                                                             \
    while(!_items[i].first.empty()) {                                                                                  \
        if(_items[i].first == (key)) {                                                                                 \
            ok = true;                                                                                                 \
            break;                                                                                                     \
        }                                                                                                              \
        i = (i + 1) & _mask;                                                                                           \
    }

#define HASH_PROBE_0 HASH_PROBE_1

void NameDict::_set_capacity_and_alloc_items(uint16_t val) {
    _capacity = val;
    _critical_size = val * _load_factor;
    _mask = val - 1;

    _items = (Item*)std::malloc(_capacity * sizeof(Item));
    std::memset(_items, 0, _capacity * sizeof(Item));
}

void NameDict::set(StrName key, PyVar val) {
    bool ok;
    uint16_t i;
    HASH_PROBE_1(key, ok, i);
    if(!ok) {
        _size++;
        if(_size > _critical_size) {
            _rehash_2x();
            HASH_PROBE_1(key, ok, i);
        }
        _items[i].first = key;
    }
    _items[i].second = val;
}

void NameDict::_rehash_2x() {
    Item* old_items = _items;
    uint16_t old_capacity = _capacity;
    _set_capacity_and_alloc_items(_capacity * 2);
    for(uint16_t i = 0; i < old_capacity; i++) {
        if(old_items[i].first.empty()) continue;
        bool ok;
        uint16_t j;
        HASH_PROBE_1(old_items[i].first, ok, j);
        assert(!ok);
        _items[j] = old_items[i];
    }
    std::free(old_items);
}

PyVar NameDict::try_get(StrName key) const {
    bool ok;
    uint16_t i;
    HASH_PROBE_0(key, ok, i);
    if(!ok) return nullptr;
    return _items[i].second;
}

PyVar* NameDict::try_get_2(StrName key) const {
    bool ok;
    uint16_t i;
    HASH_PROBE_0(key, ok, i);
    if(!ok) return nullptr;
    return &_items[i].second;
}

PyVar NameDict::try_get_likely_found(StrName key) const {
    uint16_t i = key.index & _mask;
    if(_items[i].first == key) return _items[i].second;
    i = (i + 1) & _mask;
    if(_items[i].first == key) return _items[i].second;
    return try_get(key);
}

PyVar* NameDict::try_get_2_likely_found(StrName key) const {
    uint16_t i = key.index & _mask;
    if(_items[i].first == key) return &_items[i].second;
    i = (i + 1) & _mask;
    if(_items[i].first == key) return &_items[i].second;
    return try_get_2(key);
}

bool NameDict::del(StrName key) {
    bool ok;
    uint16_t i;
    HASH_PROBE_0(key, ok, i);
    if(!ok) return false;
    _items[i].first = StrName();
    _items[i].second = nullptr;
    _size--;
    // tidy
    uint16_t pre_z = i;
    uint16_t z = (i + 1) & _mask;
    while(!_items[z].first.empty()) {
        uint16_t h = _items[z].first.index & _mask;
        if(h != i) break;
        std::swap(_items[pre_z], _items[z]);
        pre_z = z;
        z = (z + 1) & _mask;
    }
    return true;
}

bool NameDict::contains(StrName key) const {
    bool ok;
    uint16_t i;
    HASH_PROBE_0(key, ok, i);
    return ok;
}

PyVar NameDict::operator[] (StrName key) const {
    PyVar* val = try_get_2_likely_found(key);
    if(val == nullptr) PK_FATAL_ERROR("NameDict key not found: %s", key.escape().c_str())
    return *val;
}

array<StrName> NameDict::keys() const {
    array<StrName> v(_size);
    int j = 0;
    for(uint16_t i = 0; i < _capacity; i++) {
        if(_items[i].first.empty()) continue;
        new (&v[j++]) StrName(_items[i].first);
    }
    return v;
}

array<NameDict::Item> NameDict::items() const {
    array<Item> v(_size);
    int j = 0;
    for(uint16_t i = 0; i < _capacity; i++) {
        if(_items[i].first.empty()) continue;
        new (&v[j++]) Item(_items[i].first, _items[i].second);
    }
    return v;
}

void NameDict::clear() {
    std::memset(_items, 0, _capacity * sizeof(Item));
    _size = 0;
}

void NameDict::apply (void (*f)(StrName, PyVar, void*), void* data) {
    for(uint16_t i = 0; i < _capacity; i++) {
        if(_items[i].first.empty()) continue;
        f(_items[i].first, _items[i].second, data);
    }
}

#undef HASH_PROBE_0
#undef HASH_PROBE_1

PyVar PyObject::attr(StrName name) const {
    assert(is_attr_valid());
    return (*_attr)[name];
}

NameDict* PyObject::__init_namedict(float lf) {
    return new NameDict(lf);
}

}  // namespace pkpy
